
set(ScarfSparrow_Midi_SOURCES
  "pad.c"
  "pad.h"
  "internal/pad_struct.h"
)

add_library(scarfsparrow_midi STATIC "${ScarfSparrow_Midi_SOURCES}")
if (APPLE)
  enable_language(OBJC)
  target_sources(scarfsparrow_midi PRIVATE
    "pad_osx.m"
  )
endif()
target_compile_features(scarfsparrow_midi PUBLIC
  cxx_std_17)
target_link_libraries(scarfsparrow_midi PUBLIC
  AllegroBox)
