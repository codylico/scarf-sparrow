
set(ScarfSparrow_Allegro_SOURCES
  "stream.cpp"
  "stream.hpp"
  )

add_library(scarfsparrow_allegro STATIC "${ScarfSparrow_Allegro_SOURCES}")
find_library(allegro NAMES allegro allegro-static)
find_library(allegro_audio NAMES allegro_audio allegro_audio-static)
find_path(allegro_include NAMES allegro5/allegro.h)

target_compile_features(scarfsparrow_allegro PUBLIC cxx_std_17)
target_link_libraries(scarfsparrow_allegro PUBLIC allegro_audio allegro)
target_include_directories(scarfsparrow_allegro PRIVATE "${allegro_include}")
