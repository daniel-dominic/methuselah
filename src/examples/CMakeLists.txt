add_executable(GameOfLife gameOfLife.cpp)
target_link_libraries(GameOfLife PUBLIC Utils)
target_include_directories(GameOfLife PUBLIC 
  "../utils"
  "${METHUSALAH_INCLUDE_DIR}")
target_compile_features(GameOfLife PUBLIC cxx_std_17)