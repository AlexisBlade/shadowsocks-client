cmake_minimum_required(VERSION 3.10)

# Specify the project name and the programming language
project(shadowsocks_client C)

# Specify the C standard
set(CMAKE_C_STANDARD 11)

# Set the output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Add paths to find libraries and include files
if (NOT CMAKE_C_COMPILER MATCHES "x86_64-w64-mingw32-gcc")
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(SODIUM REQUIRED libsodium)
    include_directories(${SODIUM_INCLUDE_DIRS})
    link_directories(${SODIUM_LIBRARY_DIRS})
    
    # Shadowsocks-libev
    pkg_check_modules(SHADOWSOCKS REQUIRED shadowsocks-libev)
    include_directories(${SHADOWSOCKS_INCLUDE_DIRS})
    link_directories(${SHADOWSOCKS_LIBRARY_DIRS})
else()
    include_directories(/usr/x86_64-w64-mingw32/include)
    link_directories(/usr/x86_64-w64-mingw32/lib)
    
    # Shadowsocks-libev for Windows
    include_directories(/path/to/shadowsocks-libev/include)
    link_directories(/path/to/shadowsocks-libev/lib)
endif()

# Add executable for Linux or Windows
if (NOT CMAKE_C_COMPILER MATCHES "x86_64-w64-mingw32-gcc")
    add_executable(shadowsocks_client src/main.c)
    target_link_libraries(shadowsocks_client ${SODIUM_LIBRARIES} ${SHADOWSOCKS_LIBRARIES})
else()
    add_executable(shadowsocks_client_windows src/main.c)
    target_link_libraries(shadowsocks_client_windows sodium ws2_32 wininet shadowsocks-libev)
    set_target_properties(shadowsocks_client_windows PROPERTIES OUTPUT_NAME "shadowsocks_client")
endif()
