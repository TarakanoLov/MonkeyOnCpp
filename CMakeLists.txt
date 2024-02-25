cmake_minimum_required(VERSION 3.16.3)

project(monkey)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(${CMAKE_SOURCE_DIR}/src)
link_directories(${CMAKE_SOURCE_DIR}/src)

file(GLOB SOURCES
    src/*/*.h
    src/*/*.cpp
)

add_executable(monkey.exe ${SOURCES})

target_sources(
  monkey.exe
  PRIVATE
    src/main.cpp
  )





enable_testing()

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

file(GLOB TEST_SOURCES
    tests/*/*.h
    tests/*/*.cpp
)

add_executable(tests.exe ${TEST_SOURCES})

target_link_libraries(
    tests.exe
  GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(tests.exe)