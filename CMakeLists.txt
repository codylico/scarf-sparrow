cmake_minimum_required(VERSION 3.25)
project(ScarfSparrow CXX C)

find_library(allegro NAMES allegro allegro-static)
find_library(allegro_audio NAMES allegro_audio allegro_audio-static)
find_path(allegro_include NAMES allegro5/allegro.h)

add_library(AllegroBox INTERFACE)
target_link_libraries(AllegroBox INTERFACE allegro_audio allegro)
target_include_directories(AllegroBox INTERFACE "${allegro_include}")

add_subdirectory(deps)
add_subdirectory(src)

option(ScarfSparrow_BUILD_EXAMPLES "Build example programs." ON)

if (ScarfSparrow_BUILD_EXAMPLES)
  add_subdirectory(examples)
endif()

