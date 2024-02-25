cmake_minimum_required(VERSION 3.16.3)

project(monkey)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(${CMAKE_SOURCE_DIR})
link_directories(${CMAKE_SOURCE_DIR})

add_executable(hello.x "")

target_sources(
  hello.x
  PRIVATE
    main.cpp
  )