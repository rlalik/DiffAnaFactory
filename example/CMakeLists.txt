add_executable(example_create example_create.cpp)
target_link_libraries(example_create Midas::Midas ${FMT_TARGET})

add_executable(example_read_and_fit example_read_and_fit.cpp)
target_link_libraries(example_read_and_fit Midas::Midas HelloFitty::HelloFitty)

add_executable(example_multi_read example_multi_read.cpp)
target_link_libraries(example_multi_read Midas::Midas)
