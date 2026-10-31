
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include "internal/pad_struct.h"
#include "pad.h"
#include <allegro5/allegro.h>
#include <time.h>
#include <stdio.h>
#include <stdint.h>
#include <intrin.h>
#include <Windows.h>
#include <mmeapi.h>


enum ssmidi_pad_const
{
  SSMIDI_PAD_RING_BITS = 6,
  SSMIDI_PAD_RING_SIZE = (1 << SSMIDI_PAD_RING_BITS),
  SSMIDI_PAD_RING_MASK = (SSMIDI_PAD_RING_SIZE - 1),
};

struct ssmidi_note_w32
{
  DWORD_PTR timestamp;
  DWORD_PTR data;
};

struct ssmidi_pad_w32
{
  struct SSMIDI_PAD base;
  ALLEGRO_MUTEX* alsa_mx;
  ALLEGRO_COND* alsa_cond;
  HMIDIIN dev_handle;
  int lost;
  // snd_seq_t* client;
  int in_port;
  int in_queue;
  // snd_seq_addr_t src_addr;
  struct ssmidi_note_w32 packets[SSMIDI_PAD_RING_SIZE];
  LONG volatile packet_rpos;
  LONG volatile packet_wpos;
  struct pollfd* poll_table;
  unsigned poll_count;
  struct timespec tick_info;
  uint64_t first_stamp;
};



static void ssmidi_pad_add_packet(struct ssmidi_pad_w32* pad, struct ssmidi_note_w32 const* pack)
{
  LONG const wpos = pad->packet_wpos;
  LONG const rpos = pad->packet_rpos;
  if ((wpos - rpos) < SSMIDI_PAD_RING_SIZE)
  {
	memcpy(&pad->packets[wpos & SSMIDI_PAD_RING_MASK], pack, sizeof(struct ssmidi_note_w32));
	_InterlockedIncrement(&pad->packet_wpos);
  }
}

static int ssmidi_pad_get_packet(struct ssmidi_pad_w32* pad, struct ssmidi_note_w32* pack)
{
  LONG const wpos = pad->packet_wpos;
  LONG const rpos = pad->packet_rpos;
  if ((wpos - rpos) > 0)
  {
	memcpy(pack, &pad->packets[rpos & SSMIDI_PAD_RING_MASK], sizeof(struct ssmidi_note_w32));
	_InterlockedIncrement(&pad->packet_rpos);
	return 1;
  }
  return 0;
}


static unsigned ssmidi_decode_timestamp(struct ssmidi_pad_w32* pad, DWORD_PTR ts)
{
  return (unsigned)(((ts / 1000) << 16)
	| ((ts % 1000) * 65536L / 1000L));
}

static void ssmidi_decode_note(struct ssmidi_pad_w32* pad, struct ssmidi_note_w32 const* w32note)
{
  struct SSMIDI_PAD_NOTE note = { 0 };
  note.timestamp = ssmidi_decode_timestamp(pad, w32note->timestamp);
  note.cmd = (unsigned char)(w32note->data & 255u);
  note.args[0] = (unsigned char)((w32note->data >> 8) & 255u);
  note.args[1] = (unsigned char)((w32note->data >> 16) & 255u);
  ssmidi_pad_add_note(&pad->base, &note);
}

static void CALLBACK ssmidi_pad_callback(HMIDIIN hMidiIn, UINT wMsg,
  DWORD_PTR pad_ptr, DWORD_PTR dwParam1, DWORD_PTR dwParam2)
{
  struct ssmidi_pad_w32* const pad = (struct ssmidi_pad_w32*)pad_ptr;
  switch (wMsg)
  {
  case MIM_CLOSE:
	pad->lost = 1;
	break;
  case MIM_DATA:
	{
	  struct ssmidi_note_w32 next = {0};
	  next.timestamp = dwParam2;
	  next.data = dwParam1;
	  ssmidi_pad_add_packet(pad, &next);
	}
	break;
  default:
	return;
  }
  al_lock_mutex(pad->alsa_mx);
  al_signal_cond(pad->alsa_cond);
  al_unlock_mutex(pad->alsa_mx);
}

static void* ssmidi_pad_worker_w32(ALLEGRO_THREAD* th, void* pad_ptr)
{
  struct ssmidi_pad_w32* const pad = (struct ssmidi_pad_w32*)pad_ptr;
  MMRESULT res = 0;
  unsigned dev_count = midiInGetNumDevs();
  unsigned dev_index = ~0u;
  al_lock_mutex(pad->alsa_mx);
  for (dev_index = 0; dev_index < dev_count; ++dev_index)
  {
	MIDIINCAPSA caps = {0};
	MMRESULT mm_res = midiInGetDevCapsA(dev_index, &caps, sizeof(caps));
	if (mm_res == MMSYSERR_NOERROR)
	{
	  // Check name of device.
	  if (strncmp(pad->base.addr, caps.szPname, sizeof(caps.szPname)) == 0)
	  {
		fprintf(stderr, "found: %.32s\n", caps.szPname);
		break;
	  }
	}
  }
  if (dev_index >= dev_count)
  {
	al_unlock_mutex(pad->alsa_mx);
	return NULL;
  }
  // Open the device.
  res = midiInOpen(&pad->dev_handle, dev_index, (DWORD_PTR)ssmidi_pad_callback,
	(DWORD_PTR)pad_ptr, CALLBACK_FUNCTION);
  if (res != MMSYSERR_NOERROR)
  {
	al_unlock_mutex(pad->alsa_mx);
	return NULL;
  }
  res = midiInStart(pad->dev_handle);
  if (res != MMSYSERR_NOERROR)
  {
	al_unlock_mutex(pad->alsa_mx);
	return NULL;
  }
  fputs("running...\n", stderr);

  // Enable timestamps.
  {
  }

  while (!(al_get_thread_should_stop(th) || pad->lost))
  {
	int count = 0;
	struct ssmidi_note_w32 next = {0};
	al_wait_cond(pad->alsa_cond, pad->alsa_mx);
	while (ssmidi_pad_has_note_room(&pad->base) && ssmidi_pad_get_packet(pad, &next))
	{
	  ssmidi_decode_note(pad, &next);
	  count += 1;
	}
	if (count > 0)
	{
	  ALLEGRO_EVENT ev = { 0 };
	  ev.user.type = ALLEGRO_GET_EVENT_TYPE('S', 'S', 'M', '%');
	  ev.user.data1 = count;
	  ev.user.data2 = (next.data & 0xFFffFF);
	  al_emit_user_event(&pad->base.event_source, &ev, NULL);
	}
  }
  midiInStop(pad->dev_handle);
  al_unlock_mutex(pad->alsa_mx);
  return NULL;
}



int ssmidi_init_pad(struct SSMIDI_PAD* pad, char const* addr)
{
  struct ssmidi_pad_w32* const expad = (struct ssmidi_pad_w32*)pad;
  expad->in_port = -1;
  expad->in_queue = -1;
  pad->addr = strdup(addr);
  if (!pad->addr)
  {
	ssmidi_fast_pad_cleanup(pad);
	return 0;
  }
  expad->alsa_cond = al_create_cond();
  expad->alsa_mx = al_create_mutex();
  if (!(expad->alsa_cond && expad->alsa_mx))
  {
	ssmidi_fast_pad_cleanup(pad);
	return 0;
  }
  pad->worker = al_create_thread(ssmidi_pad_worker_w32, pad);
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
  struct ssmidi_pad_w32* const expad = (struct ssmidi_pad_w32*)pad;
  if (pad->worker)
  {
	al_lock_mutex(expad->alsa_mx);
	al_set_thread_should_stop(pad->worker);
	al_signal_cond(expad->alsa_cond);
	al_unlock_mutex(expad->alsa_mx);
	al_destroy_thread(pad->worker);
  }
  free(pad->addr);
  if (expad->poll_table)
  {
	free(expad->poll_table);
	expad->poll_table = NULL;
  }
  if (expad->dev_handle && expad->dev_handle != INVALID_HANDLE_VALUE)
  {
	midiInClose(expad->dev_handle);
	expad->dev_handle = INVALID_HANDLE_VALUE;
  }
  al_destroy_cond(expad->alsa_cond);
  al_destroy_mutex(expad->alsa_mx);
}

struct SSMIDI_PAD* ssmidi_pad_os_alloc(void)
{
  return (struct SSMIDI_PAD*)calloc(1, sizeof(struct ssmidi_pad_w32));
}
