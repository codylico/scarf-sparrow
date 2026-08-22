
add_executable(scarfsparrow_ex_zero "ex_zero.cpp")
target_link_libraries(scarfsparrow_ex_zero
  PUBLIC scarfsparrow_allegro scarfsparrow_core AllegroBox)

add_executable(scarfsparrow_ex_rand "ex_rand.cpp")
target_link_libraries(scarfsparrow_ex_rand
  PUBLIC scarfsparrow_allegro scarfsparrow_core AllegroBox)
