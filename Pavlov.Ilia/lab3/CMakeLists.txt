cmake_minimum_required (VERSION 3.8)

project ("lab3")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_compile_options(-Wall -Werror)

find_library(LIBRT rt)
find_package(Threads)

set(SRC src/main.cpp )

add_executable (lab3 ${SRC})
target_link_libraries(lab3 PUBLIC pthread rt)
