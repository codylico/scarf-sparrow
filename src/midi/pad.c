
#include "internal/pad_struct.h"
#include <stdlib.h>

struct SSMIDI_PAD* ssmidi_create_pad(char const* addr)
{
  struct SSMIDI_PAD* const pad = (struct SSMIDI_PAD*)calloc(1, sizeof(struct SSMIDI_PAD));
  if (!pad)
    return NULL;
  pad->worker = NULL;
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
    ssmidi_shutdown_pad(pad);
  free(pad);
}
