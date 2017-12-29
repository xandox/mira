project(format)

set(TARGET ${CMAKE_PROJECT_NAME}-${PROJECT_NAME})

set(INC ${CMAKE_CURRENT_SOURCE_DIR}/include/mira/${PROJECT_NAME})
set(SRC ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(TESTS ${CMAKE_CURRENT_SOURCE_DIR}/tests)

set(HEADERS
    ${INC}/format.hpp
)

add_library(${TARGET}
    ${SRC}/format.cpp
)

set_target_properties(${TARGET} PROPERTIES PUBLIC_HEADER "${HEADERS}")

target_include_directories(${TARGET} PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

install(TARGETS ${TARGET}
    ARCHIVE DESTINATION lib
    PUBLIC_HEADER DESTINATION include/mira/${PROJECT_NAME}
    COMPONENT mira-static-library
)


target_compile_features(${TARGET}
PUBLIC
    cxx_std_17
)

add_executable(${TARGET}-tests
    ${TESTS}/format.tests.cpp
)

target_link_libraries(${TARGET}-tests ${TARGET} gtest gtest_main)
gtest_discover_tests(${TARGET}-tests)

install(TARGETS ${TARGET}-tests
    DESTINATION tests
    COMPONENT tests)
