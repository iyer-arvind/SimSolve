cmake_minimum_required(VERSION 2.6)
project(simsolve)

add_executable(simsolve main.cpp)

install(TARGETS simsolve RUNTIME DESTINATION bin)
