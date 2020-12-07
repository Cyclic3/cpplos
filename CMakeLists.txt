project(cpplos)

add_subdirectory(cppthings)

set(CMAKE_CXX_STANDARD 17)

file(GLOB_RECURSE source src/*.cpp)
add_executable(cpplos ${source})
target_include_directories(cpplos PUBLIC include/)
target_link_libraries(cpplos CppThings::cppthings ncurses)


