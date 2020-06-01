cmake_minimum_required (VERSION 3.0)
project(mini-cad)
set(CMAKE_C_COMPILER "gcc")

aux_source_directory(include include_var)

add_library(${PROJECT_NAME} STATIC ${include_var})

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_executable(minicad main.c)
target_link_libraries(minicad mini-cad)