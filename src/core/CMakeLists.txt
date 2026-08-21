
set(ScarfSparrow_Core_SOURCES
  "node.cpp"
  "node.hpp"
  "graph.cpp"
  "graph.hpp"
  "sample.hpp"
)

add_library(scarfsparrow_core STATIC "${ScarfSparrow_Core_SOURCES}")

target_compile_features(scarfsparrow_core PUBLIC cxx_std_17)
