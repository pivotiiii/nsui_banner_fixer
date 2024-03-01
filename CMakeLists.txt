cmake_minimum_required(VERSION 3.28.3)
project(
    nsui_banner_fixer 
    VERSION 1.5.0
    DESCRIPTION "Fixes banners for GBA forwarders generated with NSUIv28 on non-US consoles."
    LANGUAGES CXX)

add_executable(nsui_banner_fixer src/nsui_banner_fixer.cpp src/Game.cpp)
target_compile_features(nsui_banner_fixer PUBLIC cxx_std_20)

#include(CTest)
enable_testing()

#----------------------------------------------------------------------------------------

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
target_link_libraries(nsui_banner_fixer TCLAP subprocess)

#----------------------------------------------------------------------------------------

FetchContent_Populate(
    3dstool
    URL         https://github.com/dnasdw/3dstool/releases/download/v1.0.9/3dstool.zip
    SOURCE_DIR  tools/3dstool
)

add_custom_command(
    TARGET nsui_banner_fixer POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
            ${CMAKE_SOURCE_DIR}/build/tools/3dstool/3dstool.exe
            $<TARGET_FILE_DIR:nsui_banner_fixer>/tools/3dstool.exe)

FetchContent_Populate(
    ctrtool
    URL         https://github.com/3DSGuy/Project_CTR/releases/download/ctrtool-v0.5/ctrtool-win_x86_64-v0.5.zip
    SOURCE_DIR  tools/ctrtool
)

add_custom_command(
    TARGET nsui_banner_fixer POST_BUILD 
    COMMAND ${CMAKE_COMMAND} -E copy 
            ${CMAKE_SOURCE_DIR}/build/tools/ctrtool/ctrtool.exe 
            $<TARGET_FILE_DIR:nsui_banner_fixer>/tools/ctrtool.exe)

FetchContent_Populate(
    makerom
    URL         https://github.com/3DSGuy/Project_CTR/releases/download/makerom-v0.15/makerom-win_x86_64-v0.15.zip
    SOURCE_DIR  tools/makerom
)

add_custom_command(
    TARGET nsui_banner_fixer POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
            ${CMAKE_SOURCE_DIR}/build/tools/makerom/makerom.exe
            $<TARGET_FILE_DIR:nsui_banner_fixer>/tools/makerom.exe)

#----------------------------------------------------------------------------------------

install(TARGETS nsui_banner_fixer DESTINATION .)
install(FILES "${CMAKE_SOURCE_DIR}/build/tools/3dstool/3dstool.exe" DESTINATION tools)
install(FILES "${CMAKE_SOURCE_DIR}/build/tools/ctrtool/ctrtool.exe" DESTINATION tools)
install(FILES "${CMAKE_SOURCE_DIR}/build/tools/makerom/makerom.exe" DESTINATION tools)

set(CPACK_PACKAGE_NAME ${PROJECT_NAME})
set(CPACK_PACKAGE_VERSION_MAJOR "${nsui_banner_fixer_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${nsui_banner_fixer_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_MICRO "${nsui_banner_fixer_VERSION_MICRO}")
set(CPACK_SOURCE_GENERATOR "TGZ")
set(CPACK_NSIS OFF)
set(CPACK_SOURCE_ZIP OFF)
include(CPack)
#in build/ : cpack -G ZIP -C Release