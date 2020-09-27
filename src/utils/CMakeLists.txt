find_package(SDL2 REQUIRED)

add_library(Utils INTERFACE)
target_link_libraries(Utils INTERFACE SDL2)
target_include_directories(Utils INTERFACE 
  "."
  "${SDL2_INCLUDE_DIRS}"
  "${METHUSALAH_INCLUDE_DIR}")