cmake_minimum_required(VERSION 3.14)

project(Methuselah)

option(METHUSALAH_BuildExamples "Build the example targets." ON)

set(METHUSALAH_TARGET_NAME "Methusalah")
set(METHUSALAH_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/include/")

add_library(${METHUSALAH_TARGET_NAME} INTERFACE)
target_include_directories(
    ${METHUSALAH_TARGET_NAME}
    INTERFACE
    ${METHUSALAH_INCLUDE_DIR}
)

if (METHUSALAH_BuildExamples)
    add_subdirectory(examples)
endif()
