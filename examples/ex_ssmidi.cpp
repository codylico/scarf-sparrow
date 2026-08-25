
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

  // Event loop.
  bool done = false;
  while (!done)
  {
    ALLEGRO_EVENT ev = {};
    al_wait_for_event_timed(q.get(), &ev, 1.0f);
  }
  std::cerr << "Done." << std::endl;
  return EXIT_SUCCESS;
}
