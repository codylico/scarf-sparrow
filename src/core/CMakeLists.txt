
set(ScarfSparrow_Core_SOURCES
  "node.cpp"
  "node.hpp"
  "graph.cpp"
  "graph.hpp"
  "sample.cpp"
  "sample.hpp"
  "ring.cpp"
  "ring.hpp"
  "port.cpp"
  "port.hpp"
  "zero.cpp"
  "zero.hpp"
  "rand.cpp"
  "rand.hpp"
  "square.cpp"
  "square.hpp"
  "saw.cpp"
  "saw.hpp"
  "xiphos.cpp"
  "xiphos.hpp"
  "dimmer.cpp"
  "dimmer.hpp"
)

add_library(scarfsparrow_core STATIC "${ScarfSparrow_Core_SOURCES}")
if (UNIX)
  find_library(unix_math NAMES m)
  target_link_libraries(scarfsparrow_core PUBLIC "${unix_math}")
endif ()

target_compile_features(scarfsparrow_core PUBLIC cxx_std_17)
