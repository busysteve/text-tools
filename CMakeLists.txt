cmake_minimum_required(VERSION 3.24)

project(text-tools LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(html2txt src/html2txt.cpp src/extract.cpp)
target_include_directories(html2txt PRIVATE src)

add_executable(breakout src/breakout.cpp src/split.cpp)
target_include_directories(breakout PRIVATE src)

add_executable(utf8rm src/utf8rm.cpp src/ascii.cpp)
target_include_directories(utf8rm PRIVATE src)

add_executable(splitter src/splitter.cpp src/chunk.cpp)
target_include_directories(splitter PRIVATE src)

add_executable(html2txt_tests tests/test_html2txt.cpp src/extract.cpp)
target_include_directories(html2txt_tests PRIVATE src)

add_executable(breakout_tests tests/test_breakout.cpp src/split.cpp)
target_include_directories(breakout_tests PRIVATE src)

add_executable(utf8rm_tests tests/test_utf8rm.cpp src/ascii.cpp)
target_include_directories(utf8rm_tests PRIVATE src)

add_executable(splitter_tests tests/test_splitter.cpp src/chunk.cpp)
target_include_directories(splitter_tests PRIVATE src)

enable_testing()
add_test(NAME extract COMMAND html2txt_tests)
add_test(NAME split COMMAND breakout_tests)
add_test(NAME ascii COMMAND utf8rm_tests)
add_test(NAME chunk COMMAND splitter_tests)
