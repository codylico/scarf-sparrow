#include "util.hpp"
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
}
