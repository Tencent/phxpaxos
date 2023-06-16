cmake_minimum_required(VERSION 3.10)

# specify project name
project(sample VERSION 1.0.0)
cmake_policy(SET CMP0048 NEW)

add_subdirectory(phxecho)
add_subdirectory(phxelection)
add_subdirectory(phxkv)
