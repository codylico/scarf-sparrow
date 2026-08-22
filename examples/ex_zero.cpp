
#define ALLEGRO_NO_MAGIC_MAIN
#include "../src/core/graph.hpp"
#include "../src/core/zero.hpp"
#include "../src/alleg/stream.hpp"
#include <allegro5/allegro.h>
#include <allegro5/allegro_audio.h>
#include <iostream>
#include <utility>
#include <cstdlib>
#include <cassert>

namespace Scarf = ScarfSparrow;

struct EventQueueDeleter
{
  void operator()(ALLEGRO_EVENT_QUEUE* q) const noexcept
  {
    al_destroy_event_queue(q);
  }
};

struct VoiceDeleter
{
  void operator()(ALLEGRO_VOICE* vox) const noexcept
  {
    al_destroy_voice(vox);
  }
};
struct Uninstaller
{
  ~Uninstaller()
  {
    al_uninstall_system();
  }
};


int main(int argc, char**argv)
{
  constexpr unsigned batch_size = 8192;
  al_init();
  Uninstaller uninit;
  if (!al_install_audio())
  {
    std::cerr << "Failed to install audio." << std::endl;
    return EXIT_FAILURE;
  }
  std::unique_ptr<ALLEGRO_EVENT_QUEUE, EventQueueDeleter> q;
  q.reset(al_create_event_queue());
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
  graph.addNode(std::make_unique<Scarf::ZeroNode>(), "zero");
  graph.addNode(std::move(stream), "al_out");
  graph.connect({"zero", 0}, {"al_out",0});
  graph.connect({"zero", 0}, {"al_out",1});
  if (!graph.start(batch_size))
  {
    std::cerr << "Failed to start graph." << std::endl;
    return EXIT_FAILURE;
  }

  // Event loop.
  bool done = false;
  while (!done)
  {
    ALLEGRO_EVENT ev = {};
    al_wait_for_event_timed(q.get(), &ev, 1.0f);
    if (ev.type == ALLEGRO_EVENT_AUDIO_STREAM_FRAGMENT)
    {
      graph.step();
    }
    done = graph.done();
  }
  std::cerr << "Done." << std::endl;
  return EXIT_SUCCESS;
}
