
#define ALLEGRO_NO_MAGIC_MAIN
#include "../src/core/graph.hpp"
#include "../src/core/saw.hpp"
#include "../src/alleg/stream.hpp"
#include "../src/alleg/util.hpp"
#include <allegro5/allegro.h>
#include <allegro5/allegro_audio.h>
#include <iostream>
#include <utility>
#include <cstdlib>
#include <cassert>

namespace Scarf = ScarfSparrow;

int main(int argc, char**argv)
{
  constexpr unsigned batch_size = 8192;
  al_init();
  Scarf::Uninstaller uninit;
  if (!al_install_audio())
  {
    std::cerr << "Failed to install audio." << std::endl;
    return EXIT_FAILURE;
  }

  auto const q = Scarf::util_makeEventQueue();
  assert(q);
  Scarf::Graph graph;
  auto stream = std::make_unique<Scarf::AllegroStreamNode>(4, batch_size);
  if (!stream)
  {
    std::cerr << "Failed to create audio stream." << std::endl;
    return EXIT_FAILURE;
  }
  al_register_event_source(q.get(), al_get_audio_stream_event_source(stream->get()));

  // Make the voice.
  al_reserve_samples(0);
  al_attach_audio_stream_to_mixer(stream->get(), al_get_default_mixer());

  // Make the graph.
  graph.addNode(std::make_unique<Scarf::SawNode>(), "saw");
  graph.addNode(std::move(stream), "al_out");
  graph.connect({"saw", 0}, {"al_out",0});
  graph.connect({"saw", 0}, {"al_out",1});
  if (!graph.start(batch_size))
  {
    std::cerr << "Failed to start graph." << std::endl;
    return EXIT_FAILURE;
  }

  // Event loop.
  Scarf::util_runEventQueue(graph, q.get());
  std::cerr << "Done." << std::endl;
  return EXIT_SUCCESS;
}
