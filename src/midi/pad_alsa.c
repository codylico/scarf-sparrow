
#include "internal/pad_struct.h"
#include "pad.h"
#include <allegro5/allegro.h>
#include <alsa/asoundlib.h>
#include <poll.h>
#include <time.h>
#include <stdio.h>
#include <stdatomic.h>
#include <stddef.h>


enum ssmidi_pad_const
{
  SSMIDI_PAD_RING_BITS = 6,
  SSMIDI_PAD_RING_SIZE = (1<<SSMIDI_PAD_RING_BITS),
  SSMIDI_PAD_RING_MASK = (SSMIDI_PAD_RING_SIZE-1),
};

struct ssmidi_pad_alsa
{
	struct SSMIDI_PAD base;
	ALLEGRO_MUTEX* alsa_mx;
	ALLEGRO_COND* alsa_cond;
	snd_seq_t* client;
	int in_port;
	int in_queue;
	snd_seq_addr_t src_addr;
	snd_seq_event_t packets[SSMIDI_PAD_RING_SIZE];
	_Atomic(unsigned) packet_rpos;
	_Atomic(unsigned) packet_wpos;
	struct pollfd* poll_table;
	unsigned poll_count;
	struct timespec tick_info;
	uint64_t first_stamp;
};

#if 0
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

static void ssmidi_pad_add_packet(struct ssmidi_pad_alsa* pad, MIDIEventPacket const* pack)
{
  size_t const max_words = sizeof(pack->words)/sizeof(UInt32);
  unsigned sz = (pack->wordCount < max_words
	? offsetof(MIDIEventPacket, words) + sizeof(UInt32)*pack->wordCount : sizeof(MIDIEventPacket));
  unsigned wpos = atomic_load_explicit(&pad->packet_wpos, memory_order_acquire);
  unsigned rpos = atomic_load_explicit(&pad->packet_rpos, memory_order_acquire);
  if ((wpos - rpos) < SSMIDI_PAD_RING_SIZE)
  {
	memset(&pad->packets[wpos&SSMIDI_PAD_RING_MASK], 0, sizeof(MIDIEventPacket));
	memcpy(&pad->packets[wpos&SSMIDI_PAD_RING_MASK], pack, sz);
	atomic_fetch_add_explicit(&pad->packet_wpos, 1, memory_order_release);
  }
}

static int ssmidi_pad_get_packet(struct ssmidi_pad_alsa* pad, MIDIEventPacket* pack)
{
  unsigned wpos = atomic_load_explicit(&pad->packet_wpos, memory_order_acquire);
  unsigned rpos = atomic_load_explicit(&pad->packet_rpos, memory_order_acquire);
  if ((wpos - rpos) > 0)
  {
	unsigned const read_anchor = atomic_fetch_add_explicit(&pad->packet_rpos, 1, memory_order_acquire);
	memcpy(pack, &pad->packets[read_anchor&SSMIDI_PAD_RING_MASK], sizeof(MIDIEventPacket));
	return 1;
  }
  return 0;
}
#endif //0

static unsigned ssmidi_decode_timestamp(struct ssmidi_pad_alsa* pad, snd_seq_timestamp_t const* ts)
{
	return (unsigned)(((ts->time.tv_sec & 0xFFFFu) << 16)
		| (ts->time.tv_nsec * 65536L / 1000000000L));
}

static int ssmidi_decode_note(struct ssmidi_pad_alsa* pad, snd_seq_event_t* ev)
{
	struct SSMIDI_PAD_NOTE note = {0};
	if (!ev)
		return 0;
	fprintf(stderr, "\t<%u,%u>\n", ev->time.time.tv_sec, ev->time.time.tv_nsec);
	if (!(ev->time.time.tv_sec || ev->time.time.tv_nsec))
	{
		// TODO: Get MIDI timestamps working properly.
		struct timespec ts = {0};
		clock_gettime(CLOCK_MONOTONIC, &ts);
		ev->time.time.tv_sec = (unsigned)ts.tv_sec;
		ev->time.time.tv_nsec = (unsigned)ts.tv_nsec;
	}
	note.timestamp = ssmidi_decode_timestamp(pad, &ev->time);
	switch (ev->type)
	{
	case SND_SEQ_EVENT_NOTEOFF:
	case SND_SEQ_EVENT_NOTEON:
		{
			note.cmd = (ev->type == SND_SEQ_EVENT_NOTEON ? 0x90 : 0x80)
				| (ev->data.note.channel & 15);
			note.args[0] = ev->data.note.note;
			note.args[1] = (ev->type == SND_SEQ_EVENT_NOTEON ? ev->data.note.velocity : 0);
		} break;
	default:
		note.cmd = (unsigned char)(ev->type & 0x7F);
		break;
	}
	ssmidi_pad_add_note(&pad->base, &note);
	return 1;
}

void* ssmidi_pad_worker_alsa(ALLEGRO_THREAD *th, void *pad_ptr)
{
	struct ssmidi_pad_alsa* const pad = (struct ssmidi_pad_alsa*)pad_ptr;
	int res = 0;
	snd_seq_addr_t sink_addr = {0};
	al_lock_mutex(pad->alsa_mx);
	res = snd_seq_open(&pad->client, "default", SND_SEQ_OPEN_INPUT, 0);
	if (res < 0)
	{
		al_unlock_mutex(pad->alsa_mx);
		return NULL;
	}
	sink_addr.client = (unsigned char)snd_seq_client_id(pad->client);
	fprintf(stderr, "client: %i\n", sink_addr.client);
	snd_seq_nonblock(pad->client, 1);
	snd_seq_set_client_name(pad->client, "ScarfSparrow MIDI Client");
	{
		snd_seq_port_info_t* info = NULL;
		snd_seq_port_info_alloca(&info);
		snd_seq_port_info_set_timestamp_queue(info, pad->in_queue);
		snd_seq_port_info_set_timestamping(info, 1);
		snd_seq_port_info_set_timestamp_real(info, 1);
		snd_seq_port_info_set_name(info, "ScarfSparrow MIDI Sink");
		snd_seq_port_info_set_capability(info, SND_SEQ_PORT_CAP_WRITE|SND_SEQ_PORT_CAP_SUBS_WRITE);
		snd_seq_port_info_set_type(info, SND_SEQ_PORT_TYPE_MIDI_GENERIC);
		pad->in_port = snd_seq_create_port(pad->client, info);
		if (pad->in_port < 0)
		{
			al_unlock_mutex(pad->alsa_mx);
			return NULL;
		}
	}
	sink_addr.port = (unsigned char)pad->in_port;
	pad->in_queue = snd_seq_alloc_named_queue(pad->client, "ScarfSparrow MIDI Queue");
	if (pad->in_queue < 0)
	{
		al_unlock_mutex(pad->alsa_mx);
		return NULL;
	}
	res = snd_seq_poll_descriptors_count(pad->client, POLLIN);
	fprintf(stderr, "poll count: %i\n", res);
	pad->poll_table = calloc(res, sizeof(struct pollfd));
	pad->poll_count = (unsigned)res;
	snd_seq_poll_descriptors(pad->client, pad->poll_table, pad->poll_count, POLLIN);

	// Parse the source address.
	if (snd_seq_parse_address(pad->client, &pad->src_addr, pad->base.addr) < 0)
	{
		al_unlock_mutex(pad->alsa_mx);
		return NULL;
	}
	fprintf(stderr, "addr: %u %u\n", pad->src_addr.client, pad->src_addr.port);

	// Connect.
	{
		snd_seq_port_subscribe_t *subs;
		snd_seq_port_subscribe_alloca(&subs);
		snd_seq_port_subscribe_set_sender(subs, &pad->src_addr);
		snd_seq_port_subscribe_set_dest(subs, &sink_addr);
		snd_seq_port_subscribe_set_time_real(subs, 1);
		snd_seq_port_subscribe_set_queue(subs, pad->in_queue);
		snd_seq_port_subscribe_set_time_update(subs, 1);
		if (snd_seq_subscribe_port(pad->client, subs) < 0)
		{
			al_unlock_mutex(pad->alsa_mx);
			return NULL;
		}
	}
	snd_seq_start_queue(pad->client, pad->in_queue, NULL);
	snd_seq_drain_output(pad->client);
	fputs("running...\n", stderr);

	// Enable timestamps.
	{
	}

	while (!al_get_thread_should_stop(th))
	{
		snd_seq_event_t* ev = NULL;
		int expect = 1;
		int count = 0;
		struct timespec ts = {0};
		unsigned short revents = 0;
		ts.tv_sec = 1;
		poll(pad->poll_table, pad->poll_count, 1000);
		if (snd_seq_poll_descriptors_revents(pad->client, pad->poll_table,
			pad->poll_count, &revents) != 0 || !(revents & POLLIN))
		{
			continue;
		}
		while (ssmidi_pad_has_note_room(&pad->base)
			&& expect > 0)
		{
			expect = snd_seq_event_input(pad->client, &ev);
			fprintf(stderr, "[%i]\n", expect);
			count += ssmidi_decode_note(pad, ev);
		}
		if (count > 0)
		{
			fprintf(stderr, "[%+i]\n", count);
			ALLEGRO_EVENT ev = {0};
			ev.user.type = ALLEGRO_GET_EVENT_TYPE('S','S','M','%');
			ev.user.data1 = count;
			ev.user.data2 = 0; //(next.words[0] & 0xFFffFF);
			al_emit_user_event(&pad->base.event_source, &ev, NULL);
		}
	}
	al_unlock_mutex(pad->alsa_mx);
	return NULL;
}


int ssmidi_init_pad(struct SSMIDI_PAD* pad, char const* addr)
{
	struct ssmidi_pad_alsa* const expad = (struct ssmidi_pad_alsa*)pad;
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
	pad->worker = al_create_thread(ssmidi_pad_worker_alsa, pad);
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
  struct ssmidi_pad_alsa* const expad = (struct ssmidi_pad_alsa*)pad;
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
  if (expad->in_queue >= 0)
  {
	snd_seq_free_queue(expad->client, expad->in_queue);
	expad->in_queue = -1;
  }
  if (expad->in_port >= 0)
  {
	snd_seq_delete_port(expad->client, expad->in_port);
	expad->in_port = -1;
  }
  if (expad->client)
  {
	snd_seq_close(expad->client);
	expad->client = NULL;
  }
  al_destroy_cond(expad->alsa_cond);
  al_destroy_mutex(expad->alsa_mx);
}

struct SSMIDI_PAD* ssmidi_pad_os_alloc(void)
{
  return (struct SSMIDI_PAD*)calloc(1, sizeof(struct ssmidi_pad_alsa));
}
