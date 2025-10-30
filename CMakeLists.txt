cmake_minimum_required(VERSION 3.14)
project(optionVol VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(include)

set(SOURCES
    src/main.cpp
    src/contracts.cpp
    src/http_utils.cpp
)

add_executable(optionVol ${SOURCES})

find_package(CURL REQUIRED)
target_link_libraries(optionVol PRIVATE CURL::libcurl pthread)

find_package(Threads REQUIRED)
target_link_libraries(optionVol PRIVATE Threads::Threads)

find_package(Matplot++ QUIET)

if(NOT Mathplot++_FOUND)
    include(FetchContent)
    FetchContent_Declare(
        matplotplusplus
        GIT_REPOSITORY https://github.com/alandefreitas/matplotplusplus
        GIT_TAG origin/master
    )
    FetchContent_GetProperties(matplotplusplus)
    if(NOT matplotplusplus_POPULATED)
        FetchContent_Populate(matplotplusplus)
        add_subdirectory(${matplotplusplus_SOURCE_DIR} ${matplotplusplus_BINARY_DIR} EXCLUDE_FROM_ALL)
    endif()
endif()

target_link_libraries(optionVol PRIVATE Matplot++::matplot)

target_compile_options(optionVol PRIVATE -Werror)