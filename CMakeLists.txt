cmake_minimum_required(VERSION 3.16)

set(PROJ serialization)
project(${PROJ})


set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -g")

add_executable(${PROJ}_example src/example.cc)
target_include_directories(${PROJ}_example PUBLIC include)
target_link_libraries(${PROJ}_example pthread)

# unit tests (simple assertion-based tests)
add_executable(${PROJ}_tests src/test_serialization.cc)
target_include_directories(${PROJ}_tests PUBLIC include)
target_link_libraries(${PROJ}_tests pthread)