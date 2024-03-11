cmake_minimum_required(VERSION 3.28.3)
project(
    nsui_banner_fixer 
    VERSION 1.5.0
    DESCRIPTION "Fixes banners for GBA forwarders generated with NSUIv28 on non-US consoles."
    LANGUAGES CXX C)

add_executable(${PROJECT_NAME} 
    src/nsui_banner_fixer.cpp 
    src/Game.cpp 
    src/Tool.cpp)
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_20)

option(DYNAMIC_LINKING "Link CURL and OPENSSL dynamic instead of static. (Linux only)" OFF)
option(STATIC_STD_LIBS "Link libstdc++ and libgcc static instead of dynamic. (Linux only)" OFF)

#include(CTest)
enable_testing()

#----------------------------------------------------------------------------------------

include(FetchContent)
FetchContent_Declare(
    tclap
    GIT_REPOSITORY      https://github.com/mirror/tclap.git
    GIT_TAG             799a8b1f99818e39fee19d0601030770af1221e1 #v1.4.0-rc1
    PATCH_COMMAND       git apply "${CMAKE_CURRENT_SOURCE_DIR}/patches/tclap.patch"
    UPDATE_DISCONNECTED 1
)
FetchContent_Declare(
    subprocess
    GIT_REPOSITORY      https://github.com/arun11299/cpp-subprocess.git
    GIT_TAG             40cd59c0970960a0ef41365ae9d96c6a72ee6922 #
    PATCH_COMMAND       git apply "${CMAKE_CURRENT_SOURCE_DIR}/patches/subprocess.patch"
    UPDATE_DISCONNECTED 1
)
FetchContent_Declare(
    stdcapture
    GIT_REPOSITORY      https://github.com/dmikushin/stdcapture.git
    GIT_TAG             25ea65ba7933c4ce7baa48dcc90063476d539586 #
)
FetchContent_MakeAvailable(tclap subprocess stdcapture)
target_link_libraries(${PROJECT_NAME} PUBLIC TCLAP subprocess stdcapture)

#----------------------------------------------------------------------------------------

#FetchContent_Declare(
#    3dstool
#    URL         https://github.com/dnasdw/3dstool/releases/download/v1.0.9/3dstool.zip
#)
#
#FetchContent_Declare(
#    ctrtool
#    URL         https://github.com/3DSGuy/Project_CTR/releases/download/ctrtool-v0.5/ctrtool-win_x86_64-v0.5.zip
#)
#
#FetchContent_Declare(
#    makerom
#    URL         https://github.com/3DSGuy/Project_CTR/releases/download/makerom-v0.15/makerom-win_x86_64-v0.15.zip
#)
#
#FetchContent_MakeAvailable(3dstool ctrtool makerom)
#
#add_custom_command(
#    TARGET ${PROJECT_NAME} POST_BUILD
#    COMMAND mkdir -p $<TARGET_FILE_DIR:${PROJECT_NAME}>/tools
#    COMMAND ${CMAKE_COMMAND} -E copy_if_different
#            ${3dstool_SOURCE_DIR}/3dstool.exe
#            ${ctrtool_SOURCE_DIR}/ctrtool.exe
#            ${makerom_SOURCE_DIR}/makerom.exe
#            $<TARGET_FILE_DIR:${PROJECT_NAME}>/tools)

#----------------------------------------------------------------------------------------
# 3DSTool
#----------------------------------------------------------------------------------------

FetchContent_Declare(
    3dstool_download
    GIT_REPOSITORY      https://github.com/dnasdw/3dstool.git
    GIT_TAG             9c4336bca8898f3860b41241b8a7d9d4a6772e79
    GIT_PROGRESS        TRUE
    PATCH_COMMAND       git apply "${CMAKE_CURRENT_SOURCE_DIR}/patches/3dstool.patch"
    UPDATE_DISCONNECTED 1
)
FetchContent_Populate(3dstool_download)

target_include_directories(${PROJECT_NAME} PRIVATE ${3dstool_download_SOURCE_DIR}/src)
file(GLOB 3dstool_sources ${3dstool_download_SOURCE_DIR}/src/*.cpp)
target_sources(${PROJECT_NAME} PRIVATE ${3dstool_sources})

if(MSVC_VERSION LESS 1600)
    target_include_directories(${PROJECT_NAME} PRIVATE ${3dstool_download_SOURCE_DIR}/dep/src/capstone/msvc)
endif()
target_include_directories(${PROJECT_NAME} PRIVATE ${3dstool_download_SOURCE_DIR}/dep/src/capstone-3.0.5/include)
file(GLOB capstone_sources 
    ${3dstool_download_SOURCE_DIR}/dep/src/capstone-3.0.5/include/*.h
    ${3dstool_download_SOURCE_DIR}/dep/src/capstone-3.0.5/*.h
    ${3dstool_download_SOURCE_DIR}/dep/src/capstone-3.0.5/*.c
    ${3dstool_download_SOURCE_DIR}/dep/src/capstone-3.0.5/arch/ARM/*.c
    ${3dstool_download_SOURCE_DIR}/dep/src/capstone-3.0.5/arch/ARM/*.h
    ${3dstool_download_SOURCE_DIR}/dep/src/capstone-3.0.5/arch/ARM/*.inc)
target_sources(${PROJECT_NAME} PRIVATE ${capstone_sources})

target_compile_definitions(${PROJECT_NAME} PRIVATE SDW_MAIN CAPSTONE_USE_SYS_DYN_MEM CAPSTONE_HAS_ARM _3DSTOOL_VERSION="1.2.6")

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(BUILD64 1)
endif()

if (WIN32)
    set(LIB_FILE_EXT lib)
else()
    set(LIB_FILE_EXT a)
endif()

if(DYNAMIC_LINKING)
    find_package(CURL REQUIRED)
    target_link_libraries(${PROJECT_NAME} PUBLIC CURL::libcurl)
    find_package(OpenSSL)
    target_link_libraries(${PROJECT_NAME} PUBLIC OpenSSL::SSL OpenSSL::Crypto)
else()
    target_compile_definitions(${PROJECT_NAME} PRIVATE CURL_STATICLIB OPENSSL_USE_STATIC_LIBS)
    include(${3dstool_download_SOURCE_DIR}/cmake/AddDep.cmake)
    ADD_DEP_INCLUDE_DIR("${3dstool_download_SOURCE_DIR}/dep")
    target_include_directories(${PROJECT_NAME} PUBLIC ${DEP_INCLUDE_DIR})
    ADD_DEP_LIBRARY_DIR("${3dstool_download_SOURCE_DIR}/dep")

    set(3dstool_deps libcurl libssl libcrypto)
    foreach(LIB IN LISTS 3dstool_deps)
        add_library(${LIB} STATIC IMPORTED)
        add_dependencies(${LIB} 3dstool_check)
        target_include_directories(${LIB} INTERFACE ${DEP_INCLUDE_DIR})
        set_target_properties(${LIB} PROPERTIES IMPORTED_LOCATION ${DEP_LIBRARY_DIR}/${LIB}.${LIB_FILE_EXT})
        target_link_libraries(${PROJECT_NAME} PUBLIC ${LIB})
    endforeach()
    target_link_libraries(libcrypto INTERFACE libssl libcurl)
    target_link_libraries(libcurl INTERFACE libssl libcrypto)
    target_link_libraries(libssl INTERFACE libcrypto libcurl)
    target_link_libraries(libcrypto INTERFACE dl)
endif()

if(UNIX OR MINGW)
    if(NOT APPLE)
        target_compile_options(${PROJECT_NAME} PRIVATE -static-libgcc -static-libstdc++)
    endif()
    if((UNIX AND BUILD64) OR (MINGW AND CMAKE_SIZEOF_VOID_P EQUAL 8))
        target_compile_options(${PROJECT_NAME} PRIVATE -m64)
    else()
        target_compile_options(${PROJECT_NAME} PRIVATE -m32)
    endif()
endif()
if(UNIX OR MINGW)
    target_compile_definitions(${PROJECT_NAME} PRIVATE _FILE_OFFSET_BITS=64)
    target_compile_options(${PROJECT_NAME} PRIVATE -Wno-multichar -Wno-shift-overflow -Wno-unused-result)
endif()

if(WIN32)
  if(MSVC)
    if(MSVC_VERSION LESS 1800)
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4005")
    endif()
    target_link_libraries(${PROJECT_NAME} PUBLIC libcurl libcrypto WS2_32 Wldap32 Crypt32)
    #target_link_libraries(${PROJECT_NAME} PUBLIC WS2_32 Wldap32 Crypt32)
    set(nsui_banner_fixer_LINK_FLAGS_DEBUG "${nsui_banner_fixer_LINK_FLAGS_DEBUG} /NODEFAULTLIB:LIBCMT")
    if(MSVC_VERSION GREATER 1700)
      set(nsui_banner_fixer_LINK_FLAGS_DEBUG "${nsui_banner_fixer_LINK_FLAGS_DEBUG} /IGNORE:4099")
      set(nsui_banner_fixer_LINK_FLAGS_RELWITHDEBINFO "${nsui_banner_fixer_LINK_FLAGS_RELWITHDEBINFO} /IGNORE:4099")
    endif()
    set_target_properties(${PROJECT_NAME} PROPERTIES
      LINK_FLAGS_DEBUG "${nsui_banner_fixer_LINK_FLAGS_DEBUG}"
      LINK_FLAGS_RELWITHDEBINFO "${nsui_banner_fixer_LINK_FLAGS_RELWITHDEBINFO}")
  else()
    #target_link_libraries(${PROJECT_NAME} PUBLIC curl crypto)
  endif()
else()
  #target_link_libraries(${PROJECT_NAME} PUBLIC curl ssl crypto)
  if(APPLE)
    target_link_libraries(${PROJECT_NAME} PUBLIC ldap)
  else()
    target_link_libraries(${PROJECT_NAME} PUBLIC pthread dl)
  endif()
  if(APPLE OR CYGWIN)
    target_link_libraries(${PROJECT_NAME} PUBLIC iconv)
  endif()
endif()


#----------------------------------------------------------------------------------------
# Project_CTR
#----------------------------------------------------------------------------------------

FetchContent_Declare(
    project_ctr_download
    GIT_REPOSITORY      https://github.com/3DSGuy/Project_CTR.git
    GIT_TAG             master
    GIT_PROGRESS        TRUE
    PATCH_COMMAND       git apply "${CMAKE_CURRENT_SOURCE_DIR}/patches/ctr.patch"
    UPDATE_DISCONNECTED 1
)
FetchContent_MakeAvailable(project_ctr_download)

#----------------------------------------------------------------------------------------
# CTRTool
#----------------------------------------------------------------------------------------

FILE(
    COPY ${project_ctr_download_SOURCE_DIR}/ctrtool/src/ 
    DESTINATION ${project_ctr_download_SOURCE_DIR}/include/ctrtool
    FILES_MATCHING PATTERN "*.h")

if (LINUX)
    add_custom_command(
        OUTPUT ${project_ctr_download_SOURCE_DIR}/ctrtool/bin/ctrtool.a
        ${project_ctr_download_SOURCE_DIR}/ctrtool/deps/libbroadon-es/bin/libbroadon-es.a
        ${project_ctr_download_SOURCE_DIR}/ctrtool/deps/libfmt/bin/libfmt.a
        ${project_ctr_download_SOURCE_DIR}/ctrtool/deps/libmbedtls/bin/libmbedtls.a
        ${project_ctr_download_SOURCE_DIR}/ctrtool/deps/libnintendo-n3ds/bin/libnintendo-n3ds.a
        ${project_ctr_download_SOURCE_DIR}/ctrtool/deps/libtoolchain/bin/libtoolchain.a
        COMMAND make -C ${project_ctr_download_SOURCE_DIR}/ctrtool deps static_lib
        COMMENT "=================== running make on project_ctr/ctrtool ..."
    )
    add_custom_target(ctrtool_make ALL DEPENDS ${project_ctr_download_SOURCE_DIR}/ctrtool/bin/ctrtool.a)

    add_library(ctrtool_lib STATIC IMPORTED)
    add_dependencies(ctrtool_lib ctrtool_make)
    target_include_directories(ctrtool_lib INTERFACE ${project_ctr_download_SOURCE_DIR}/include)
    set_target_properties(ctrtool_lib PROPERTIES IMPORTED_LOCATION ${project_ctr_download_SOURCE_DIR}/ctrtool/bin/ctrtool.a)

    set(ctrtool_deps libbroadon-es libfmt libmbedtls libnintendo-n3ds libtoolchain)
    foreach(LIB IN LISTS ctrtool_deps)
        add_library(${LIB} STATIC IMPORTED)
        add_dependencies(${LIB} ctrtool_make)
        target_include_directories(${LIB} INTERFACE ${project_ctr_download_SOURCE_DIR}/ctrtool/deps/${LIB}/include)
        set_target_properties(${LIB} PROPERTIES IMPORTED_LOCATION ${project_ctr_download_SOURCE_DIR}/ctrtool/deps/${LIB}/bin/${LIB}.a)
        target_link_libraries(ctrtool_lib INTERFACE ${LIB})
    endforeach()
    target_link_libraries(libtoolchain INTERFACE libmbedtls)
endif()
target_link_libraries(${PROJECT_NAME} PUBLIC ctrtool_lib)

#----------------------------------------------------------------------------------------
# makerom
#----------------------------------------------------------------------------------------

FILE(
    COPY ${project_ctr_download_SOURCE_DIR}/makerom/src/ 
    DESTINATION ${project_ctr_download_SOURCE_DIR}/include/makerom
    FILES_MATCHING PATTERN "*.h")

if (LINUX)
    add_custom_command(
        OUTPUT ${project_ctr_download_SOURCE_DIR}/makerom/bin/makerom.a
        ${project_ctr_download_SOURCE_DIR}/makerom/deps/libblz/bin/libblz.a
        ${project_ctr_download_SOURCE_DIR}/makerom/deps/libyaml/bin/libyaml.a
        ${project_ctr_download_SOURCE_DIR}/makerom/deps/libmbedtls/bin/libmbedtls.a
        COMMAND make -C ${project_ctr_download_SOURCE_DIR}/makerom deps static_lib
        COMMENT "=================== running make on project_ctr/makerom ..."
    )
    add_custom_target(makerom_make ALL DEPENDS ${project_ctr_download_SOURCE_DIR}/makerom/bin/makerom.a)

    add_library(makerom_lib STATIC IMPORTED)
    add_dependencies(makerom_lib makerom_make)
    target_include_directories(makerom_lib INTERFACE ${project_ctr_download_SOURCE_DIR}/include)
    set_target_properties(makerom_lib PROPERTIES IMPORTED_LOCATION ${project_ctr_download_SOURCE_DIR}/makerom/bin/makerom.a)

    set(makerom_deps libblz libyaml)
    foreach(LIB IN LISTS makerom_deps)
        add_library(${LIB} STATIC IMPORTED)
        add_dependencies(${LIB} makerom_make)
        target_include_directories(${LIB} INTERFACE ${project_ctr_download_SOURCE_DIR}/makerom/deps/${LIB}/include)
        set_target_properties(${LIB} PROPERTIES IMPORTED_LOCATION ${project_ctr_download_SOURCE_DIR}/makerom/deps/${LIB}/bin/${LIB}.a)
        target_link_libraries(makerom_lib INTERFACE ${LIB})
    endforeach()
    #target_link_libraries(libtoolchain INTERFACE libmbedtls)
endif()
target_link_libraries(${PROJECT_NAME} PUBLIC makerom_lib)





#----------------------------------------------------------------------------------------

set(CMAKE_INSTALL_PREFIX ${CMAKE_SOURCE_DIR}/dist)
install(TARGETS ${PROJECT_NAME} DESTINATION .)
#install(FILES ${3dstool_SOURCE_DIR}/3dstool.exe DESTINATION tools)
#install(FILES ${ctrtool_SOURCE_DIR}/ctrtool.exe DESTINATION tools)
#install(FILES ${makerom_SOURCE_DIR}/makerom.exe DESTINATION tools)

set(CPACK_PACKAGE_NAME ${PROJECT_NAME})
set(CPACK_PACKAGE_VERSION_MAJOR "${nsui_banner_fixer_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${nsui_banner_fixer_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_MICRO "${nsui_banner_fixer_VERSION_MICRO}")
set(CPACK_SOURCE_GENERATOR "TGZ")
set(CPACK_BINARY_NSIS OFF)
set(CPACK_BINARY_ZIP ON)
include(CPack)
#in build/ : cpack -G ZIP -C Release
