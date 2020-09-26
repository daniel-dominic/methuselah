add_executable(GameOfLife gameOfLife.cpp)

# SDL 2
# -----
find_package(SDL2 REQUIRED)

target_link_libraries(GameOfLife PUBLIC SDL2)
target_include_directories(GameOfLife PUBLIC "${SDL2_INCLUDE_DIRS}")
# -----

target_include_directories(GameOfLife PUBLIC "${METHUSALAH_INCLUDE_DIR}")
target_compile_features(GameOfLife PUBLIC cxx_std_17)