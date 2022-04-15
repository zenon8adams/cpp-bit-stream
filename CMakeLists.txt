cmake_minimum_required(VERSION 3.15)
project(cpp_bit_stream)

set(CMAKE_CXX_STANDARD 14)

add_executable(cpp_bit_stream
        bitstream.cpp
        example.cpp bitstream.hpp)
