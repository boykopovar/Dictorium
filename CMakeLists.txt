cmake_minimum_required(VERSION 3.20)

project(Dictorium)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_VERBOSE_MAKEFILE ON)

set(DICTORIUM_SOURCES
       src/dtr.cpp
        include/Dictorium/Entities/RedBlackTree.h
)

add_library(Dictorium STATIC
        ${DICTORIUM_SOURCES}
)

target_include_directories(Dictorium PUBLIC include)
target_include_directories(Dictorium PUBLIC src)

add_executable(DictoriumMain
        tests/main.cpp
        tests/Utils/PrintUtils.cpp
)

target_link_libraries(DictoriumMain Dictorium)

add_executable(PerfTest
        tests/Universal/PerfTest.cpp
        tests/Utils/TestUtils.cpp
)
target_link_libraries(PerfTest Dictorium)

add_executable(CorrectnessTest
        tests/Universal/CorrectnessMain.cpp
        tests/Utils/TestUtils.cpp
)
target_link_libraries(CorrectnessTest Dictorium)


add_executable(ChainingHashSpeed
        tests/ChainingTest.cpp
        tests/Utils/TestUtils.cpp
)
target_link_libraries(ChainingHashSpeed Dictorium)

add_executable(AvlTest
        tests/AvlTest.cpp
        tests/Utils/TestUtils.cpp
)
target_link_libraries(AvlTest Dictorium)

add_executable(TreapTest
        tests/TreapTest.cpp
        tests/Utils/TestUtils.cpp
)
target_link_libraries(TreapTest Dictorium)

add_executable(SkipListTest
        tests/SkipListTest.cpp
        tests/Utils/TestUtils.cpp
)
target_link_libraries(SkipListTest Dictorium)
