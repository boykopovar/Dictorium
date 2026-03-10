cmake_minimum_required(VERSION 3.20)

project(Dictorium)

set(CMAKE_CXX_STANDARD 20)

file(GLOB DICTORIUM_SOURCES
        src/*.cpp
)

add_library(Dictorium STATIC
        ${DICTORIUM_SOURCES}
)

target_include_directories(Dictorium PUBLIC include)

add_executable(DictoriumTests tests/main.cpp)

target_link_libraries(DictoriumTests Dictorium)