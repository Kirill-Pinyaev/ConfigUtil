cmake_minimum_required(VERSION 3.16)

project(ConfigUtils VERSION 1.0.0)
include(FetchContent)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")

FetchContent_Declare(json URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz)
FetchContent_MakeAvailable(json)




#option(MYLIB_TESTING "Включить модульное тестирование" ON)
message(STATUS "OS: ${CMAKE_SYSTEM_NAME}")

add_executable(config-util main.cpp Arguments.cpp SqlDatabase.cpp lib/sqlite/sqlite3.c)
target_link_libraries(config-util PRIVATE nlohmann_json::nlohmann_json)




    