cmake_minimum_required(VERSION 3.10)

project(mira)


# testings
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/contrib/googletest-release-1.8.0)
include(GoogleTest)
enable_testing()

include(CPack)

# libraries
add_subdirectory(libs)
