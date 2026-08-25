cmake_minimum_required(VERSION 3.25)
project(ScarfSparrow CXX C)

find_library(allegro NAMES allegro allegro-static)
find_library(allegro_audio NAMES allegro_audio allegro_audio-static)
find_path(allegro_include NAMES allegro5/allegro.h)

add_library(AllegroBox INTERFACE)
target_link_libraries(AllegroBox INTERFACE "${allegro_audio}" "${allegro}")
target_include_directories(AllegroBox INTERFACE "${allegro_include}")

option(ScarfSparrow_STATIC "Whether to use static libraries" ON)

if (ScarfSparrow_STATIC)
  set(AllegroBoxSupplements)
  if(APPLE)
    list(APPEND AllegroBoxSupplements
      CoreVideo
      CoreAudio
      CoreMIDI
      AudioToolbox
      OpenGL
      CoreGraphics
      AppKit
      IOKit
      UniformTypeIdentifiers
      Carbon
      vorbisfile
      vorbis
      ogg
      OpenAL
      harfbuzz
      freetype
      harfbuzz
      png
      bz2
      z
      jpeg
      Foundation
      )
  endif()

  foreach (supplement ${AllegroBoxSupplements})
    message(STATUS "static supplement: ${supplement}")
    find_library("AllegroSupp_${supplement}" NAMES ${supplement})
    target_link_libraries(AllegroBox INTERFACE "${AllegroSupp_${supplement}}")
  endforeach()
endif ()

add_subdirectory(deps)
add_subdirectory(src)

option(ScarfSparrow_BUILD_EXAMPLES "Build example programs." ON)

if (ScarfSparrow_BUILD_EXAMPLES)
  add_subdirectory(examples)
endif()

