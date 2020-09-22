add_executable(GameOfLife gameOfLife.cpp)
target_include_directories(GameOfLife PUBLIC "${METHUSALAH_INCLUDE_DIR}")
target_compile_features(GameOfLife PUBLIC cxx_std_17)