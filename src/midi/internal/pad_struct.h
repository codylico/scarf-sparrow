
#if !(defined hg_ScarfSparrowMidi_padStruct_h_)
#define hg_ScarfSparrowMidi_padStruct_h_

#include "../pad.h"
#include <allegro5/allegro.h>

#if !(defined _WIN32)
#  include <stdatomic.h>
#endif //_WIN32

struct ALLEGRO_THREAD;

struct SSMIDI_PAD
{
  ALLEGRO_EVENT_SOURCE event_source;
  char* addr;
  struct ALLEGRO_THREAD* worker;
  int done;
#if (defined _WIN32)
  unsigned volatile rpos;
  unsigned volatile wpos;
#else
  _Atomic(unsigned) rpos;
  _Atomic(unsigned) wpos;
#endif //_WIN32
  struct SSMIDI_PAD_NOTE q[64];
};

int ssmidi_init_pad(struct SSMIDI_PAD* pad, char const* addr);
void ssmidi_shutdown_pad(struct SSMIDI_PAD* pad);
void ssmidi_fast_pad_cleanup(struct SSMIDI_PAD* pad);
struct SSMIDI_PAD* ssmidi_pad_os_alloc(void);
int ssmidi_pad_has_note_room(struct SSMIDI_PAD const* pad);
void ssmidi_pad_add_note(struct SSMIDI_PAD* pad, struct SSMIDI_PAD_NOTE const* pack);

#endif //hg_ScarfSparrowMidi_padStruct_h_
