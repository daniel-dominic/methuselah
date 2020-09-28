# Game of life
add_executable(GameOfLife gameOfLife.cpp)
target_link_libraries(GameOfLife PUBLIC Utils)

# Fluid flow
add_executable(FluidFlow fluidFlow.cpp)
target_link_libraries(FluidFlow PUBLIC Utils)