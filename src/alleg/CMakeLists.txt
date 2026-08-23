
set(ScarfSparrow_Allegro_SOURCES
  "stream.cpp"
  "stream.hpp"
  "util.cpp"
  "util.hpp"
  )

add_library(scarfsparrow_allegro STATIC "${ScarfSparrow_Allegro_SOURCES}")

target_compile_features(scarfsparrow_allegro PUBLIC cxx_std_17)
target_link_libraries(scarfsparrow_allegro PUBLIC AllegroBox)
