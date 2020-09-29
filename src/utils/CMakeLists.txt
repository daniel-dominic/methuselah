find_package(SDL2 REQUIRED)
find_package(SDL2_image REQUIRED)

add_library(Utils INTERFACE)
target_link_libraries(Utils INTERFACE SDL2 SDL2_image)
target_include_directories(Utils INTERFACE 
  "."
  "${SDL2_INCLUDE_DIRS}"
  "${METHUSALAH_INCLUDE_DIR}")
target_compile_features(Utils INTERFACE cxx_std_17)