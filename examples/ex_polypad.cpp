
#define ALLEGRO_NO_MAGIC_MAIN
#include "../src/core/graph.hpp"
#include "../src/core/saw.hpp"
#include "../src/alleg/midipad.hpp"
#include "../src/core/sum.hpp"
#include "../src/core/polyphony.hpp"
#include "../src/core/dimmer.hpp"
#include "../src/alleg/stream.hpp"
#include "../src/alleg/util.hpp"
#include <allegro5/allegro.h>
#include <allegro5/allegro_audio.h>
#include <iostream>
#include <fstream>
#include <string>
#include <utility>
#include <cstdlib>
#include <cassert>

namespace Scarf = ScarfSparrow;



int main(int argc, char**argv)
{
  constexpr unsigned batch_size = 1024;
  constexpr unsigned concurrency = 8;
  constexpr unsigned batch_lag = 2;
  std::string device_name;
  bool mono_mode = false;
  for (int argi = 1; argi < argc; ++argi)
  {
    if (argv[argi][0] == '-')
    {
      if (std::strcmp(argv[argi], "--mono") == 0)
        mono_mode = true;
    }
    else if (device_name.empty())
      device_name = argv[argi];
  }
  if (device_name.empty())
  {
    std::cerr << "usage: scarfsparrow_ex_polypad (device_name)" << std::endl;
    return EXIT_FAILURE;
  }

  al_init();
  Scarf::Uninstaller uninit;
  if (!al_install_audio())
  {
    std::cerr << "Failed to install audio." << std::endl;
    return EXIT_FAILURE;
  }

  auto pad = std::make_unique<Scarf::MidiPadNode>(device_name, batch_size);

  auto const q = Scarf::util_makeEventQueue();
  assert(q);
  Scarf::Graph graph;
  auto stream = std::make_unique<Scarf::AllegroStreamNode>(batch_lag, batch_size);
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
  graph.addNode(std::move(pad), "pad");
  graph.addNode(std::make_unique<Scarf::PolyphonyNode>(concurrency), "polyphony");
  graph.addNode(std::make_unique<Scarf::SumNode>(concurrency), "sum");
  auto const stream_ptr = stream->get();
  graph.addNode(std::move(stream), "al_out");
  graph.connect({"pad", 0}, {"polyphony",0});
  graph.connect({"pad", 1}, {"polyphony",1});
  graph.connect({"sum", 0}, {"al_out",0});
  graph.connect({"sum", 0}, {"al_out",1});
  for (unsigned j = 0; j < concurrency; ++j)
  {
    auto saw_name = "saw_" + std::to_string(j);
    auto dimmer_name = "dimmer_" + std::to_string(j);
    graph.addNode(std::make_unique<Scarf::SawNode>(), saw_name);
    graph.addNode(std::make_unique<Scarf::DimmerNode>(1.0f/concurrency, 0.f, mono_mode), dimmer_name);
    graph.connect({"polyphony", 0+j*2}, {saw_name,0});
    graph.connect({saw_name, 0}, {dimmer_name,0});
    graph.connect({"polyphony", 1+j*2}, {dimmer_name,1});
    graph.connect({dimmer_name, 0}, {"sum",j});
  }
  if (!graph.start(batch_size))
  {
    std::cerr << "Failed to start graph." << std::endl;
    return EXIT_FAILURE;
  }

  // Event loop.
  Scarf::util_runEventQueue(graph, q.get(), stream_ptr);
  std::cerr << "Done." << std::endl;
  return EXIT_SUCCESS;
}
