
#define ALLEGRO_NO_MAGIC_MAIN
#include "../src/alleg/midipad.hpp"
#include "../src/alleg/stream.hpp"
#include "../src/alleg/util.hpp"
#include "../src/core/factory.hpp"
#include "../src/core/graph.hpp"
#include "../src/core/subgraph.hpp"
#include "../src/core/saw.hpp"
#include "../src/core/sum.hpp"
#include "../src/core/polyphony.hpp"
#include "../src/core/dimmer.hpp"
#include <allegro5/allegro.h>
#include <allegro5/allegro_audio.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <utility>
#include <cstdlib>
#include <cassert>

namespace Scarf = ScarfSparrow;


static std::string read_json(std::string const& json_name)
{
  std::ifstream json_src(json_name);
  if (!json_src)
  {
    std::cerr << "Failed to open JSON file: " << json_name << std::endl;
    return {};
  }
  std::ostringstream stash;
  stash << json_src.rdbuf();
  return stash.str();
}

int main(int argc, char**argv)
{
  constexpr unsigned batch_size = 512;
  constexpr unsigned concurrency = 8;
  constexpr unsigned batch_lag = 4;
  std::string device_name;
  std::string json_name;
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
    else if (json_name.empty())
      json_name = argv[argi];
  }
  if (device_name.empty() || json_name.empty())
  {
    std::cerr << "usage: scarfsparrow_ex_graphpad (device_name) (json)" << std::endl;
    return EXIT_FAILURE;
  }

  al_init();
  Scarf::Uninstaller uninit;
  if (!al_install_audio())
  {
    std::cerr << "Failed to install audio." << std::endl;
    return EXIT_FAILURE;
  }

  // Read in the JSON.
  std::string json = read_json(json_name);
  if (json.empty())
  {
    std::cerr << "JSON file provided empty string: " << json_name << std::endl;
    return EXIT_FAILURE;
  }

  Scarf::Factory toolbox;
  Scarf::factory_add_core(toolbox);

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
    auto subgraph_name = "subgraph_" + std::to_string(j);
    auto dimmer_name = "dimmer_" + std::to_string(j);
    auto subgraph = std::make_unique<Scarf::SubgraphNode>();
    if (!subgraph->configure(json, toolbox))
    {
      std::cerr << "Configuration failed." << std::endl;
      return EXIT_FAILURE;
    }
    bool const need_dimmer = (subgraph->countInputs() > 1);
    graph.addNode(std::move(subgraph), subgraph_name);
    if (need_dimmer)
    {
      graph.addNode(std::make_unique<Scarf::DimmerNode>(1.0f/concurrency, 0.f, mono_mode), dimmer_name);
      graph.connect({"polyphony", 0+j*2}, {subgraph_name,0});
      graph.connect({subgraph_name, 0}, {dimmer_name,0});
      graph.connect({"polyphony", 1+j*2}, {dimmer_name,1});
      graph.connect({dimmer_name, 0}, {"sum",j});
    }
    else
    {
      graph.connect({"polyphony", 0+j*2}, {subgraph_name,0});
      graph.connect({"polyphony", 1+j*2}, {subgraph_name,1});
      graph.connect({subgraph_name, 0}, {"sum",j});
    }
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
