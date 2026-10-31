
#include "internal/pad_struct.h"
#include <stdlib.h>
#if (defined _WIN32)
#  include <intrin.h>
#endif //_WIN32

#if (defined _WIN32)
#  define ssmidi_atomic_load(x) (x)
#  define ssmidi_atomic_add_rel(x) _InterlockedIncrement(&x)
#else
#  define ssmidi_atomic_load(x) atomic_load_explicit(&(x), memory_order_acquire)
#  define ssmidi_atomic_add_rel(x) atomic_fetch_add_explicit(&(x), 1, memory_order_release)
#endif //_WIN32

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
  unsigned wpos = ssmidi_atomic_load(pad->wpos);
  unsigned rpos = ssmidi_atomic_load(pad->rpos);
  if ((wpos - rpos) < 64)
  {
    memcpy(&pad->q[wpos&63], pack, sizeof(struct SSMIDI_PAD_NOTE));
    ssmidi_atomic_add_rel(pad->wpos);
  }
}

int ssmidi_pad_has_note_room(struct SSMIDI_PAD const* pad)
{
  unsigned wpos = ssmidi_atomic_load(pad->wpos);
  unsigned rpos = ssmidi_atomic_load(pad->rpos);
  return ((wpos - rpos) < 64);
}
int ssmidi_pad_get_note(struct SSMIDI_PAD* pad, struct SSMIDI_PAD_NOTE* pack)
{
  unsigned wpos = ssmidi_atomic_load(pad->wpos);
  unsigned rpos = ssmidi_atomic_load(pad->rpos);
  if ((wpos - rpos) > 0)
  {
    memcpy(pack, &pad->q[rpos&63], sizeof(struct SSMIDI_PAD_NOTE));
    ssmidi_atomic_add_rel(pad->rpos);
    return 1;
  }
  return 0;
}

struct ALLEGRO_EVENT_SOURCE* ssmidi_pad_get_event_source(struct SSMIDI_PAD* pad)
{
  return &pad->event_source;
}
