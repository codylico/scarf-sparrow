#include "util.hpp"
#include "../core/graph.hpp"
#include <allegro5/allegro_audio.h>
#include <allegro5/allegro.h>

namespace ScarfSparrow
{
  void EventQueueDeleter::operator()(ALLEGRO_EVENT_QUEUE* q) const noexcept
  {
    al_destroy_event_queue(q);
  }

  Uninstaller::~Uninstaller()
  {
    al_uninstall_system();
  }

  EventQueuePtr util_makeEventQueue() noexcept
  {
    return EventQueuePtr(al_create_event_queue());
  }

  bool util_runEventQueue(Graph& g, ALLEGRO_EVENT_QUEUE* q, ALLEGRO_AUDIO_STREAM* stream)
  {
    bool done = false;
    while (!done)
    {
      ALLEGRO_EVENT ev = {};
      al_wait_for_event_timed(q, &ev, 1.0f);
      if (ev.type == ALLEGRO_EVENT_AUDIO_STREAM_FRAGMENT)
      {
        // If no stream given, just blindly fill one fragment.
        unsigned const fragment_count = (stream ? al_get_available_audio_stream_fragments(stream) : 1);
        for (unsigned j = 0; j < fragment_count; ++j)
          g.step();
      }
      done = g.done();
    }
    return done;
  }
}
