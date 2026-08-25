
add_executable(scarfsparrow_ex_zero "ex_zero.cpp")
target_link_libraries(scarfsparrow_ex_zero
  PUBLIC scarfsparrow_allegro scarfsparrow_core AllegroBox)

add_executable(scarfsparrow_ex_rand "ex_rand.cpp")
target_link_libraries(scarfsparrow_ex_rand
  PUBLIC scarfsparrow_allegro scarfsparrow_core AllegroBox)

add_executable(scarfsparrow_ex_square "ex_square.cpp")
target_link_libraries(scarfsparrow_ex_square
  PUBLIC scarfsparrow_allegro scarfsparrow_core AllegroBox)

add_executable(scarfsparrow_ex_saw "ex_saw.cpp")
target_link_libraries(scarfsparrow_ex_saw
  PUBLIC scarfsparrow_allegro scarfsparrow_core AllegroBox)

add_executable(scarfsparrow_ex_xiphos "ex_xiphos.cpp")
target_link_libraries(scarfsparrow_ex_xiphos
  PUBLIC scarfsparrow_allegro scarfsparrow_core AllegroBox)

add_executable(scarfsparrow_ex_ssmidi "ex_ssmidi.cpp")
target_link_libraries(scarfsparrow_ex_ssmidi
  PUBLIC scarfsparrow_allegro scarfsparrow_core scarfsparrow_midi AllegroBox)
