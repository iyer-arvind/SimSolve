cmake_minimum_required(VERSION 2.8)
project(simsolve)
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

find_package(NLOPT REQUIRED)
find_package(Units)

include_directories(${NLOPT_INCLUDE_DIRS})
link_directories(${NLOPT_LIBRARIES})


add_subdirectory(src)
