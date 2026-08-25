
#include "internal/pad_struct.h"
#include <allegro5/allegro.h>
#include <time.h>
#include <stdio.h>

void* ssmidi_pad_worker_osx(ALLEGRO_THREAD *th, void *pad_ptr)
{
  struct SSMIDI_PAD* const pad = (struct SSMIDI_PAD*)pad_ptr;
  while (!al_get_thread_should_stop(th))
  {
    struct timespec rqtp = {0};
    rqtp.tv_sec = 1;
    nanosleep(&rqtp, NULL);
    puts("yo.");
  }
  return NULL;
}


int ssmidi_init_pad(struct SSMIDI_PAD* pad, char const* addr)
{
  pad->worker = al_create_thread(ssmidi_pad_worker_osx, pad);
  if (!pad->worker)
  {
    ssmidi_fast_pad_cleanup(pad);
    return 0;
  }
  return 1;
}
void ssmidi_shutdown_pad(struct SSMIDI_PAD* pad)
{
  if (pad->worker)
  {
    al_destroy_thread(pad->worker);
  }
}

void ssmidi_fast_pad_cleanup(struct SSMIDI_PAD* pad)
{
  if (pad->worker)
    al_destroy_thread(pad->worker);
}
