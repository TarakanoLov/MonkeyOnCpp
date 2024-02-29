cmake_minimum_required(VERSION 3.16.3)

project(monkey)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
find_package(fmt QUIET)
if (NOT fmt_FOUND)
    FetchContent_Declare(
            fmt
            GIT_REPOSITORY https://github.com/fmtlib/fmt
            GIT_TAG        8.1.1
    )
    FetchContent_MakeAvailable(fmt)
endif()

include_directories(${CMAKE_SOURCE_DIR}/src)
link_directories(${CMAKE_SOURCE_DIR}/src)

file(GLOB SOURCES
    src/*/*.h
    src/*/*.cpp
    src/main.cpp
)

add_executable(monkey.exe ${SOURCES})

target_link_libraries(
  monkey.exe
  fmt::fmt
)





enable_testing()

FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

file(GLOB TEST_SOURCES
    src/*/*.h
    src/*/*.cpp
    tests/*/*.h
    tests/*/*.cpp
)

add_executable(tests.exe ${TEST_SOURCES})

target_link_libraries(
    tests.exe
    GTest::gtest_main
    fmt::fmt
)

include(GoogleTest)
gtest_discover_tests(tests.exe)