cmake_minimum_required(VERSION 3.28.3)
project(
    nsui_banner_fixer 
    VERSION 2.0.0
    DESCRIPTION "Fixes banners for GBA forwarders generated with NSUIv28 on non-US consoles."
    LANGUAGES C CXX)

option(BUILD_TESTS "Build tests." ON)
option(DYNAMIC_LINKING "Link CURL and OPENSSL dynamic instead of static. (Linux only)" OFF)
option(STATIC_STD_LIBS "Link libstdc++ and libgcc static instead of dynamic. (Linux only)" OFF)

include(dependencies.cmake)
add_subdirectory(src)

if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(test)
endif()



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