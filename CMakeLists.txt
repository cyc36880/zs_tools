cmake_minimum_required(VERSION 3.10)

file(GLOB_RECURSE SOURCE "src/*.c")

add_library(zs_tools ${SOURCE})
