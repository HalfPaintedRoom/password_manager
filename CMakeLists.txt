cmake_minimum_required(VERSION 3.26)
project(password_manager)

set(CMAKE_CXX_STANDARD 17)

add_executable(password_manager main.cpp)
find_package(unofficial-sodium CONFIG REQUIRED)
target_link_libraries(password_manager PRIVATE unofficial-sodium::sodium)
find_package(SQLiteCpp CONFIG REQUIRED)
target_link_libraries(password_manager PRIVATE SQLiteCpp)

install(
        TARGETS password_manager
        RUNTIME DESTINATION .
)

install(
        FILES
        ${CMAKE_CURRENT_BINARY_DIR}/libsodium.dll
        ${CMAKE_CURRENT_BINARY_DIR}/sqlite3.dll
        ${CMAKE_CURRENT_BINARY_DIR}/SQLiteCpp.dll
        DESTINATION .
)