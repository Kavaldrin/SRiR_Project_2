cmake_minimum_required(VERSION 3.10)
project(AntColonyTSPSolver)

add_definitions("-Wall -pedantic-errors -Werror -O2")

include_directories(${PROJECT_SOURCE_DIR}/include)
file(GLOB SRC_FILES ${PROJECT_SOURCE_DIR}/src/*.cpp)
find_package(UPCXX)


add_executable(AntColonyTSPSolver ${SRC_FILES})
target_link_libraries(AntColonyTSPSolver UPCXX::upcxx)

set_target_properties(AntColonyTSPSolver PROPERTIES
                        CXX_STANDARD 17
                        CXX_STANDARD_REQUIRED YES)


