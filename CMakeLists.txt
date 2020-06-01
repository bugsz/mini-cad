cmake_minimum_required (VERSION 3.0)
project(mini-cad)
set(CMAKE_C_COMPILER "gcc")

aux_source_directory(include include_var)
aux_source_directory(utils utils_var)


add_library(cad_lib STATIC ${include_var} ${utils_var})


include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/utils
)

add_executable(minicad main.c)
target_link_libraries(minicad cad_lib)