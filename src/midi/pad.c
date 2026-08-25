
#include "internal/pad_struct.h"
#include <stdlib.h>

struct SSMIDI_PAD* ssmidi_create_pad(char const* addr)
{
  struct SSMIDI_PAD* const pad = ssmidi_pad_os_alloc();
  if (!pad)
    return NULL;
  pad->worker = NULL;
  al_init_user_event_source(&pad->event_source);
  if (!ssmidi_init_pad(pad, addr))
  {
    ssmidi_fast_pad_cleanup(pad);
    return NULL;
  }
  return pad;
}

void ssmidi_destroy_pad(struct SSMIDI_PAD* pad)
{
  if (pad)
  {
    ssmidi_shutdown_pad(pad);
    al_destroy_user_event_source(&pad->event_source);
  }
  free(pad);
}

void ssmidi_pad_add_note(struct SSMIDI_PAD* pad, struct SSMIDI_PAD_NOTE const* pack)
{
  unsigned wpos = atomic_load_explicit(&pad->wpos, memory_order_acquire);
  unsigned rpos = atomic_load_explicit(&pad->rpos, memory_order_acquire);
  if ((wpos - rpos) < 64)
  {
    memcpy(&pad->q[wpos&63], pack, sizeof(struct SSMIDI_PAD_NOTE));
    atomic_fetch_add_explicit(&pad->wpos, 1, memory_order_release);
  }
}

int ssmidi_pad_get_note(struct SSMIDI_PAD* pad, struct SSMIDI_PAD_NOTE* pack)
{
  unsigned wpos = atomic_load_explicit(&pad->wpos, memory_order_acquire);
  unsigned rpos = atomic_load_explicit(&pad->rpos, memory_order_acquire);
  if ((wpos - rpos) > 0)
  {
    unsigned const read_anchor = atomic_fetch_add_explicit(&pad->rpos, 1, memory_order_acquire);
    memcpy(pack, &pad->q[read_anchor&63], sizeof(struct SSMIDI_PAD_NOTE));
    return 1;
  }
  return 0;
}

struct ALLEGRO_EVENT_SOURCE* ssmidi_pad_get_event_source(struct SSMIDI_PAD* pad)
{
  return &pad->event_source;
}
