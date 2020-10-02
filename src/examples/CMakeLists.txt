# Game of life
add_executable(GameOfLife gameOfLife.cpp)
target_link_libraries(GameOfLife PUBLIC Utils)

# Game of life 3D
add_executable(GameOfLife3D gameOfLife3D.cpp)
target_link_libraries(GameOfLife3D PUBLIC Utils)

# Fluid flow
add_executable(FluidFlow fluidFlow.cpp)
target_link_libraries(FluidFlow PUBLIC Utils)

# Sandpile
add_executable(Sandpile sandpile.cpp)
target_link_libraries(Sandpile PUBLIC Utils)