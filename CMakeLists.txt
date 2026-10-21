cmake_minimum_required(VERSION 3.25)
project(BattleHeart VERSION 0.0.1 LANGUAGES C CXX)

add_library(gc_disc STATIC runtime/gc_disc.c)
target_include_directories(gc_disc PUBLIC runtime)
target_compile_features(gc_disc PUBLIC c_std_11)
if(CMAKE_C_COMPILER_ID MATCHES "Clang|GNU")
  target_compile_options(gc_disc PRIVATE -Wall -Wextra -Werror -Wconversion)
endif()

add_library(sonic_gc_media STATIC sonic/sonic_gc_media.c)
target_link_libraries(sonic_gc_media PUBLIC gc_disc)
target_include_directories(sonic_gc_media PUBLIC sonic)
target_compile_features(sonic_gc_media PUBLIC c_std_11)

find_package(Threads REQUIRED)
add_library(battleheart_media STATIC runtime/src/media/sonic_media_adapter.cpp)
target_link_libraries(battleheart_media PUBLIC sonic_gc_media Threads::Threads)
target_include_directories(battleheart_media PUBLIC runtime/src/media)
target_compile_features(battleheart_media PUBLIC cxx_std_17)
target_compile_options(battleheart_media PRIVATE -Wall -Wextra -Werror -Wconversion)

add_executable(battleheart_media_alpha runtime/src/media/media_alpha.cpp)
target_link_libraries(battleheart_media_alpha PRIVATE battleheart_media)
