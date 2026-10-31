
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
elseif(LINUX)
  target_sources(scarfsparrow_midi PRIVATE
    "pad_alsa.c"
  )
  target_link_libraries(scarfsparrow_midi PUBLIC asound)
elseif(WIN32)
  target_sources(scarfsparrow_midi PRIVATE
    "pad_w32.c"
  )
  find_library(scarfsparrow_winmm_lib NAMES winmm)
  target_link_libraries(scarfsparrow_midi PUBLIC "${scarfsparrow_winmm_lib}")
  if (BUILD_SHARED_LIBS)
    target_compile_definitions(scarfsparrow_midi
        PUBLIC "ScarfSparrowMIDI_SHARED=1"
        PRIVATE "ScarfSparrowMIDI_WantExport=1")
  endif ()
endif()
target_compile_features(scarfsparrow_midi PUBLIC
  cxx_std_17)
target_link_libraries(scarfsparrow_midi PUBLIC
  AllegroBox)
