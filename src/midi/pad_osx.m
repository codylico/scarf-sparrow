
#include "internal/pad_struct.h"
#include <allegro5/allegro.h>
#include <CoreMIDI/CoreMIDI.h>
#include <time.h>
#include <stdio.h>
#include <stdatomic.h>
#include <stddef.h>



struct ssmidi_pad_osx
{
  struct SSMIDI_PAD base;
  ALLEGRO_MUTEX* osx_mx;
  ALLEGRO_COND* osx_cond;
  MIDIClientRef client;
  MIDIPortRef in_port;
  MIDIEventPacket packets[16];
  _Atomic(unsigned) packet_rpos;
  _Atomic(unsigned) packet_wpos;
};

static void ssmidi_pad_notify_osx(struct MIDINotification const* notif, void *pad_ptr)
{
  struct SSMIDI_PAD* const pad = (struct SSMIDI_PAD*)pad_ptr;
  if (notif->messageID == kMIDIMsgObjectRemoved)
  {
    fprintf(stderr, "uh oh.");
  }
  else if (notif->messageID == kMIDIMsgObjectAdded)
  {
    fprintf(stderr, "hey.");
  }
}

static void ssmidi_pad_add_packet(struct ssmidi_pad_osx* pad, MIDIEventPacket const* pack)
{
  size_t const max_words = sizeof(pack->words)/sizeof(UInt32);
  unsigned sz = (pack->wordCount < max_words
    ? offsetof(MIDIEventPacket, words) + sizeof(UInt32)*pack->wordCount : sizeof(MIDIEventPacket));
  unsigned wpos = atomic_load_explicit(&pad->packet_wpos, memory_order_acquire);
  unsigned rpos = atomic_load_explicit(&pad->packet_rpos, memory_order_acquire);
  if ((wpos - rpos) < 16)
  {
    memset(&pad->packets[wpos&15], 0, sizeof(MIDIEventPacket));
    memcpy(&pad->packets[wpos&15], pack, sz);
    atomic_fetch_add_explicit(&pad->packet_wpos, 1, memory_order_release);
  }
}

static int ssmidi_pad_get_packet(struct ssmidi_pad_osx* pad, MIDIEventPacket* pack)
{
  unsigned wpos = atomic_load_explicit(&pad->packet_wpos, memory_order_acquire);
  unsigned rpos = atomic_load_explicit(&pad->packet_rpos, memory_order_acquire);
  if ((wpos - rpos) > 0)
  {
    unsigned const read_anchor = atomic_fetch_add_explicit(&pad->packet_rpos, 1, memory_order_acquire);
    memcpy(pack, &pad->packets[read_anchor&15], sizeof(MIDIEventPacket));
    return 1;
  }
  return 0;
}

static void decode_note(UInt32 word)
{
  switch ((word >> 16)&240u)
  {
  case 0x90:
    fprintf(stderr, "on [%2u] %2u (%3u)\n", (word>>16)&15u, (word >> 8)&255u, word&255u);
    break;
  case 0x80:
    fprintf(stderr, "off[%2u] %2u (%3u)\n", (word>>16)&15u, (word >> 8)&255u, word&255u);
    break;
  }
}

void* ssmidi_pad_worker_osx(ALLEGRO_THREAD *th, void *pad_ptr)
{
  struct ssmidi_pad_osx* const pad = (struct ssmidi_pad_osx*)pad_ptr;
  al_lock_mutex(pad->osx_mx);
  CFStringRef name_ref = CFStringCreateWithCString(NULL, "ScarfSparrow MIDI Client", kCFStringEncodingUTF8);
  CFStringRef sink_name_ref = NULL;
  CFStringRef addr_ref = NULL;
  OSStatus res = MIDIClientCreate(name_ref, ssmidi_pad_notify_osx, pad, &pad->client);
  CFRelease(name_ref);
  if (res != 0)
  {
    al_unlock_mutex(pad->osx_mx);
    return NULL;
  }
  sink_name_ref = CFStringCreateWithCString(NULL, "ScarfSparrow MIDI Sink", kCFStringEncodingUTF8);
  MIDIEventPacket pack;
  res = MIDIInputPortCreateWithProtocol(pad->client, sink_name_ref, kMIDIProtocol_1_0, &pad->in_port,
    ^(const MIDIEventList * _Nonnull evtlist, void * _Nullable srcConnRefCon) {
      if (evtlist->numPackets > 0) {
        MIDIEventPacket const* next = &evtlist->packet[0];
        UInt32 i;
        for (i = 0; i < evtlist->numPackets; ++i)
        {
          if (next->wordCount > 0)
            ssmidi_pad_add_packet(pad, next);
          next = MIDIEventPacketNext(next);
        }
        al_signal_cond(pad->osx_cond);
      }
    });
  if (res != 0)
  {
    al_unlock_mutex(pad->osx_mx);
    return NULL;
  }
  // Enumerate sources.
  ItemCount src_count = MIDIGetNumberOfSources();
  ItemCount src_i = 0;
  ItemCount src_found = (ItemCount)(-1);
  for (src_i = 0; src_i < src_count; ++src_i)
  {
    char name_buffer[64] = {0};
    MIDIEndpointRef src_ref = MIDIGetSource(src_i);
    MIDIEntityRef src_entity = NULL;
    MIDIEndpointGetEntity(src_ref, &src_entity);
    if (!src_entity)
      continue;
    CFStringRef src_name_value = NULL;
    if (MIDIObjectGetStringProperty(src_entity, kMIDIPropertyName, &src_name_value) != 0)
      continue;
    CFStringGetCString(src_name_value, name_buffer, sizeof(name_buffer), kCFStringEncodingUTF8);
    CFRelease(src_name_value);
    name_buffer[sizeof(name_buffer)-1] = 0;
    fprintf(stderr, "%lu: %s\n", src_i, name_buffer);
    if (strncmp(name_buffer, pad->base.addr, 64) == 0)
    {
      src_found = src_i;
      MIDIPortConnectSource(pad->in_port, src_ref, NULL);
      fprintf(stderr, "  connected: %s\n", name_buffer);
      break;
    }
  }
  if (src_found == (ItemCount)(-1))
  {
    al_unlock_mutex(pad->osx_mx);
    return NULL;
  }
  while (!al_get_thread_should_stop(th))
  {
    int count = 0;
    MIDIEventPacket next = {0};
    al_wait_cond(pad->osx_cond, pad->osx_mx);
    while (ssmidi_pad_get_packet(pad, &next))
    {
      fprintf(stderr, "ts: %#llx\n", next.timeStamp);
      decode_note(next.words[0]);
      count += 1;
    }
  }
  al_unlock_mutex(pad->osx_mx);
  return NULL;
}


int ssmidi_init_pad(struct SSMIDI_PAD* pad, char const* addr)
{
  struct ssmidi_pad_osx* const expad = (struct ssmidi_pad_osx*)pad;
  pad->addr = strdup(addr);
  if (!pad->addr)
  {
    ssmidi_fast_pad_cleanup(pad);
    return 0;
  }
  expad->osx_cond = al_create_cond();
  expad->osx_mx = al_create_mutex();
  if (!(expad->osx_cond && expad->osx_mx))
  {
    ssmidi_fast_pad_cleanup(pad);
    return 0;
  }
  pad->worker = al_create_thread(ssmidi_pad_worker_osx, pad);
  if (!pad->worker)
  {
    ssmidi_fast_pad_cleanup(pad);
    return 0;
  }
  al_start_thread(pad->worker);
  return 1;
}
void ssmidi_shutdown_pad(struct SSMIDI_PAD* pad)
{
  ssmidi_fast_pad_cleanup(pad);
  pad->worker = NULL;
  pad->addr = NULL;
}

void ssmidi_fast_pad_cleanup(struct SSMIDI_PAD* pad)
{
  struct ssmidi_pad_osx* const expad = (struct ssmidi_pad_osx*)pad;
  if (pad->worker)
  {
    al_lock_mutex(expad->osx_mx);
    al_set_thread_should_stop(pad->worker);
    al_signal_cond(expad->osx_cond);
    al_unlock_mutex(expad->osx_mx);
    al_destroy_thread(pad->worker);
  }
  free(pad->addr);
  if (expad->in_port)
  {
    MIDIPortDispose(expad->in_port);
  }
  if (expad->client)
  {
    /*
    "Don’t explicitly dispose of your client; the system automatically
      disposes all clients when an app terminates. However, if you call
      this method to dispose the last or only client owned by an app,
      the MIDI server may exit if there are no other clients remaining
      in the system."
    */
    // MIDIClientDispose(expad->client);
  }
  al_destroy_cond(expad->osx_cond);
  al_destroy_mutex(expad->osx_mx);
}

struct SSMIDI_PAD* ssmidi_pad_os_alloc(void)
{
  return (struct SSMIDI_PAD*)calloc(1, sizeof(struct ssmidi_pad_osx));
}
