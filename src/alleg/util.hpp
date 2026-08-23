#pragma once
#include <memory>

struct ALLEGRO_EVENT_QUEUE;
struct ALLEGRO_VOICE;

namespace ScarfSparrow
{
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
}
