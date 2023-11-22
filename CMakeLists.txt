cmake_minimum_required(VERSION 3.16)

project(ConfigUtils VERSION 1.0.0)



#option(MYLIB_TESTING "Включить модульное тестирование" ON)
message(STATUS "OS: ${CMAKE_SYSTEM_NAME}")

add_executable(config-util main.cpp Arguments.cpp SqlDatabase.cpp lib/sqlite/sqlite3.c)




    