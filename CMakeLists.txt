cmake_minimum_required(VERSION 3.15)

project(abmoid)
enable_testing()

include(quick_project/Test OPTIONAL RESULT_VARIABLE quick_project)
if (quick_project)
  quick_project_add_check_target()
endif(quick_project)

add_definitions(-std=c++23)
add_definitions(-Wall)
add_definitions(-Wextra)

include_directories(${abmoid_project_SOURCE_DIR}/include)

install(FILES include/abmoid.hpp DESTINATION include)
install(DIRECTORY include/abmoid DESTINATION include)

if (quick_project)
  add_subdirectory(test)
endif(quick_project)
