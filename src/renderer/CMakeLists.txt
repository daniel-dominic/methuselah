find_package(SDL2 REQUIRED)

add_library(Renderer INTERFACE)
target_link_libraries(Renderer INTERFACE SDL2)
target_include_directories(Renderer INTERFACE "." "${SDL2_INCLUDE_DIRS}" "${METHUSALAH_INCLUDE_DIR}")