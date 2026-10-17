cmake_minimum_required(VERSION 3.16)
project(CampusTourManagementSystem LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(campus_core
    src/Place.cpp
    src/CampusTree.cpp
    src/ItineraryLinkedList.cpp
    src/GeoUtils.cpp
)
target_include_directories(campus_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_options(campus_core PRIVATE
    $<$<CXX_COMPILER_ID:GNU,Clang>:-Wall;-Wextra;-Wpedantic>
    $<$<CXX_COMPILER_ID:MSVC>:/W4>
    $<$<CXX_COMPILER_ID:MSVC>:/utf-8>
)

add_executable(campus-tour src/main.cpp)
target_link_libraries(campus-tour PRIVATE campus_core)
target_compile_options(campus-tour PRIVATE
    $<$<CXX_COMPILER_ID:GNU,Clang>:-Wall;-Wextra;-Wpedantic>
    $<$<CXX_COMPILER_ID:MSVC>:/W4>
    $<$<CXX_COMPILER_ID:MSVC>:/utf-8>
)
target_compile_definitions(campus-tour PRIVATE CAMPUS_DEFAULT_DATA_PATH="${CMAKE_CURRENT_SOURCE_DIR}/data/places.tsv")

include(CTest)
if(BUILD_TESTING)
    add_executable(campus-tests tests/test_main.cpp)
    target_link_libraries(campus-tests PRIVATE campus_core)
    target_compile_options(campus-tests PRIVATE
        $<$<CXX_COMPILER_ID:GNU,Clang>:-Wall;-Wextra;-Wpedantic>
        $<$<CXX_COMPILER_ID:MSVC>:/W4>
        $<$<CXX_COMPILER_ID:MSVC>:/utf-8>
    )
    target_compile_definitions(campus-tests PRIVATE CAMPUS_TEST_DATA_PATH="${CMAKE_CURRENT_SOURCE_DIR}/data/places.tsv")
    add_test(NAME campus-tests COMMAND campus-tests)
endif()
