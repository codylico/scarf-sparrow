#pragma once
#include <memory>

struct ALLEGRO_EVENT_QUEUE;
struct ALLEGRO_VOICE;
struct ALLEGRO_AUDIO_STREAM;

namespace ScarfSparrow
{
  class Graph;
  struct EventQueueDeleter
  {
    void operator()(ALLEGRO_EVENT_QUEUE* q) const noexcept;
  };
  struct Uninstaller
  {
    ~Uninstaller();
  };

  using EventQueuePtr = std::unique_ptr<ALLEGRO_EVENT_QUEUE, EventQueueDeleter>;

  EventQueuePtr util_makeEventQueue() noexcept;
  bool util_runEventQueue(Graph& g, ALLEGRO_EVENT_QUEUE* q, ALLEGRO_AUDIO_STREAM* stream = nullptr);
}
