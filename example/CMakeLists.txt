add_executable(example_create example_create.cpp)
target_link_libraries(example_create Midas::Midas ${FMT_TARGET})

add_executable(example_read example_read.cpp)
target_link_libraries(example_read RT::Pandora Midas::Midas)

add_executable(example_multi_read example_multi_read.cpp)
target_link_libraries(example_multi_read RT::Pandora Midas::Midas)
