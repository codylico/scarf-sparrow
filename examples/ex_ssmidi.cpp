
#define ALLEGRO_NO_MAGIC_MAIN
#include "../src/midi/pad.h"
#include "../src/alleg/util.hpp"
#include <allegro5/allegro.h>
#include <allegro5/allegro_audio.h>
#include <iostream>
#include <utility>
#include <cstdlib>
#include <cassert>

namespace Scarf = ScarfSparrow;


struct PadDeleter
{
  void operator()(SSMIDI_PAD* pad) const noexcept
  {
    ssmidi_destroy_pad(pad);
  }
};

int main(int argc, char**argv)
{
  constexpr unsigned batch_size = 8192;
  if (argc < 2)
  {
    std::cerr << "usage: scarfsparrow_ex_ssmidi (addr)" << std::endl;
    return EXIT_FAILURE;
  }
  al_init();
  Scarf::Uninstaller uninit;
  if (!al_install_audio())
  {
    std::cerr << "Failed to install audio." << std::endl;
    return EXIT_FAILURE;
  }

  auto const q = Scarf::util_makeEventQueue();
  assert(q);

  std::unique_ptr<SSMIDI_PAD, PadDeleter> pad;
  pad.reset(ssmidi_create_pad(argv[1]));
  if (!pad)
  {
    std::cerr << "Pad construction failed." << std::endl;
    return EXIT_FAILURE;
  }

  al_register_event_source(q.get(), ssmidi_pad_get_event_source(pad.get()));

  // Event loop.
  bool done = false;
  while (!done)
  {
    ALLEGRO_EVENT ev = {};
    al_wait_for_event_timed(q.get(), &ev, 1.0f);
    if (ev.type == SSMIDI_PAD_EVENT_TYPE)
    {
      SSMIDI_PAD_NOTE note = {};
      while (ssmidi_pad_get_note(pad.get(), &note))
      {
        switch (note.cmd&240u)
        {
        case 0x90:
          fprintf(stderr, "on [%2u] %2u (%3u) @%g\n", note.cmd&15u, note.args[0], note.args[1], note.timestamp / 65536.0);
          break;
        case 0x80:
          fprintf(stderr, "off[%2u] %2u (%3u) @%g\n", note.cmd&15u, note.args[0], note.args[1], note.timestamp / 65536.0);
          break;
        }
      }
    }
  }
  std::cerr << "Done." << std::endl;
  return EXIT_SUCCESS;
}
