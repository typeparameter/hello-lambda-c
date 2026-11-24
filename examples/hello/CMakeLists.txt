cmake_minimum_required(VERSION 3.10)
project(hello C)

find_package(PkgConfig REQUIRED)
pkg_check_modules(LAMBDA REQUIRED IMPORTED_TARGET lambda)

add_executable(bootstrap main.c)
target_link_libraries(bootstrap PRIVATE PkgConfig::LAMBDA)
