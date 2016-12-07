cmake_minimum_required(VERSION 3.6)
project(lguf_brightness)

set(CMAKE_CXX_STANDARD 14)

include_directories(/usr/local/include)
link_directories(/usr/local/lib)

set(SOURCE_FILES main.cpp)
add_executable(lguf_brightness ${SOURCE_FILES})
target_link_libraries(lguf_brightness usb-1.0)
