add_executable(GameOfLife gameOfLife.cpp)
target_link_libraries(GameOfLife PUBLIC Renderer)
target_include_directories(GameOfLife PUBLIC 
  "../renderer"
  "${METHUSALAH_INCLUDE_DIR}")
target_compile_features(GameOfLife PUBLIC cxx_std_17)