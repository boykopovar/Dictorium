cmake_minimum_required(VERSION 3.20)

project(Dictorium)

set(CMAKE_CXX_STANDARD 20)

set(DICTORIUM_SOURCES
       src/dtr.cpp
)

add_library(Dictorium STATIC
        ${DICTORIUM_SOURCES}
)

target_include_directories(Dictorium PUBLIC include)
target_include_directories(Dictorium PUBLIC src)

add_executable(DictoriumRun
        tests/main.cpp
        tests/Utils/PrintUtils.cpp
)

target_link_libraries(DictoriumRun Dictorium)

add_executable(PerfHashSpeed
        tests/PerfTest.cpp
        tests/Utils/TestUtils.cpp
)

target_link_libraries(PerfHashSpeed Dictorium)