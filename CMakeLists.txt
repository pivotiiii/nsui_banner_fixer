cmake_minimum_required(VERSION 3.28.3)
project(
    nsui_banner_fixer 
    VERSION 2.0.0
    DESCRIPTION "Fixes banners for GBA forwarders generated with NSUIv28 on non-US consoles."
    LANGUAGES CXX)

include(dependencies.cmake)
add_subdirectory(src)

#include(CTest)
#enable_testing()

#----------------------------------------------------------------------------------------

set(CPACK_PACKAGE_NAME ${PROJECT_NAME})
set(CPACK_PACKAGE_VERSION_MAJOR "${nsui_banner_fixer_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${nsui_banner_fixer_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_MICRO "${nsui_banner_fixer_VERSION_PATCH}")
set(CPACK_SOURCE_GENERATOR "TGZ")
set(CPACK_BINARY_NSIS OFF)
set(CPACK_BINARY_ZIP ON)
include(CPack)
#in build/ : cpack -G ZIP -C Release