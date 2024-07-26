cmake_minimum_required(VERSION 3.10)

# Укажите имя проекта и используемый язык
project(shadowsocks_client C)

# Укажите стандарт языка Си
set(CMAKE_C_STANDARD 11)

# Установите выходные каталоги
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Добавьте пути для поиска библиотек и include файлов
if (NOT CMAKE_C_COMPILER MATCHES "x86_64-w64-mingw32-gcc")
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(SODIUM REQUIRED libsodium)
    include_directories(${SODIUM_INCLUDE_DIRS})
    link_directories(${SODIUM_LIBRARY_DIRS})
else()
    include_directories(/usr/x86_64-w64-mingw32/include)
    link_directories(/usr/x86_64-w64-mingw32/lib)
endif()

# Добавьте исполняемый файл для Linux или Windows
if (NOT CMAKE_C_COMPILER MATCHES "x86_64-w64-mingw32-gcc")
    add_executable(shadowsocks_client src/main.c)
    target_link_libraries(shadowsocks_client ${SODIUM_LIBRARIES})
else()
    add_executable(shadowsocks_client_windows src/main.c)
    target_link_libraries(shadowsocks_client_windows sodium ws2_32)
    set_target_properties(shadowsocks_client_windows PROPERTIES OUTPUT_NAME "shadowsocks_client")
endif()
