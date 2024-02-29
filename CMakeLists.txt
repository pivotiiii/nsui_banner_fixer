cmake_minimum_required(VERSION 3.28.3)
project(
    nsui_banner_fixer 
    VERSION 1.5.0
    DESCRIPTION "Fixes banners for GBA forwarders generated with NSUIv28 on non-US consoles."
    LANGUAGES CXX)

include(CTest)
enable_testing()

add_executable(nsui_banner_fixer src/nsui_banner_fixer.cpp src/Game.cpp)
target_compile_features(nsui_banner_fixer PUBLIC cxx_std_20)

include(FetchContent)
FetchContent_Declare(
    tclap
    GIT_REPOSITORY      https://github.com/mirror/tclap.git
    GIT_TAG             799a8b1f99818e39fee19d0601030770af1221e1 #v1.4.0-rc1
    PATCH_COMMAND       git apply "${CMAKE_SOURCE_DIR}/patches/tclap.patch"
    UPDATE_DISCONNECTED 1
)
FetchContent_Declare(
    subprocess
    GIT_REPOSITORY      https://github.com/arun11299/cpp-subprocess.git
    GIT_TAG             40cd59c0970960a0ef41365ae9d96c6a72ee6922 #
    PATCH_COMMAND       git apply "${CMAKE_SOURCE_DIR}/patches/subprocess.patch"
    UPDATE_DISCONNECTED 1
)
FetchContent_MakeAvailable(tclap subprocess)

include_directories("${tclap_SOURCE_DIR}/include")
include_directories("${subprocess_SOURCE_DIR}")

add_custom_command(
    TARGET nsui_banner_fixer POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
            ${CMAKE_SOURCE_DIR}/src/tools/3dstool.exe
            $<TARGET_FILE_DIR:nsui_banner_fixer>/tools/3dstool.exe)

add_custom_command(
    TARGET nsui_banner_fixer POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
            ${CMAKE_SOURCE_DIR}/src/tools/ctrtool.exe
            $<TARGET_FILE_DIR:nsui_banner_fixer>/tools/ctrtool.exe)

add_custom_command(
    TARGET nsui_banner_fixer POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
            ${CMAKE_SOURCE_DIR}/src/tools/makerom.exe
            $<TARGET_FILE_DIR:nsui_banner_fixer>/tools/makerom.exe)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPACK)