project(cpplos)

add_subdirectory(cppthings)

set(CMAKE_CXX_STANDARD 17)

file(GLOB_RECURSE source src/*.cpp)
add_library(cpplos ${source})
target_include_directories(cpplos PUBLIC include/)
target_link_libraries(cpplos PUBLIC CppThings::cppthings ncurses)

file(GLOB_RECURSE game_source game/*.cpp)
add_executable(cpplos-game ${game_source})
target_link_libraries(cpplos-game cpplos)

#file(GLOB_RECURSE game_source srv/*.cpp)
#add_executable(cpplos-srv ${game_source})
#target_link_libraries(cpplos-src cpplos)
