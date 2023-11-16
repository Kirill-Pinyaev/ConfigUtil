cmake_minimum_required(VERSION 3.16)

project(ConfigUtils VERSION 1.0.0)



#option(MYLIB_TESTING "Включить модульное тестирование" ON)
message(STATUS "OS: ${CMAKE_SYSTEM_NAME}")

add_executable(main main.cpp)

add_library(message-a
STATIC 
    lib/ConfigUtil.cpp
    lib/ConfigUtil.h
)

target_compile_options(message-a 
PRIVATE
    -Wall
    -Werror
    -Wextra
)

target_link_libraries(main message-a)

    