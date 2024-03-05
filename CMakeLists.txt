cmake_minimum_required(VERSION 3.28.3)
project(
    nsui_banner_fixer 
    VERSION 1.5.0
    DESCRIPTION "Fixes banners for GBA forwarders generated with NSUIv28 on non-US consoles."
    LANGUAGES CXX C)

add_executable(nsui_banner_fixer src/nsui_banner_fixer.cpp src/Game.cpp src/Ctrtool_cia_proc.cpp)
target_compile_features(nsui_banner_fixer PUBLIC cxx_std_20)
target_compile_options(nsui_banner_fixer PUBLIC )

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
target_link_libraries(nsui_banner_fixer PUBLIC TCLAP subprocess stdcapture)

#----------------------------------------------------------------------------------------

FetchContent_Declare(
    3dstool
    URL         https://github.com/dnasdw/3dstool/releases/download/v1.0.9/3dstool.zip
)

FetchContent_Declare(
    ctrtool
    URL         https://github.com/3DSGuy/Project_CTR/releases/download/ctrtool-v0.5/ctrtool-win_x86_64-v0.5.zip
)

FetchContent_Declare(
    makerom
    URL         https://github.com/3DSGuy/Project_CTR/releases/download/makerom-v0.15/makerom-win_x86_64-v0.15.zip
)

FetchContent_MakeAvailable(3dstool ctrtool makerom)

add_custom_command(
    TARGET nsui_banner_fixer POST_BUILD
    COMMAND mkdir -p $<TARGET_FILE_DIR:nsui_banner_fixer>/tools
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${3dstool_SOURCE_DIR}/3dstool.exe
            ${ctrtool_SOURCE_DIR}/ctrtool.exe
            ${makerom_SOURCE_DIR}/makerom.exe
            $<TARGET_FILE_DIR:nsui_banner_fixer>/tools)

#----------------------------------------------------------------------------------------

install(TARGETS nsui_banner_fixer DESTINATION .)
install(FILES ${3dstool_SOURCE_DIR}/3dstool.exe DESTINATION tools)
install(FILES ${ctrtool_SOURCE_DIR}/ctrtool.exe DESTINATION tools)
install(FILES ${makerom_SOURCE_DIR}/makerom.exe DESTINATION tools)

set(CPACK_PACKAGE_NAME ${PROJECT_NAME})
set(CPACK_PACKAGE_VERSION_MAJOR "${nsui_banner_fixer_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${nsui_banner_fixer_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_MICRO "${nsui_banner_fixer_VERSION_MICRO}")
set(CPACK_SOURCE_GENERATOR "TGZ")
set(CPACK_BINARY_NSIS OFF)
set(CPACK_BINARY_ZIP ON)
include(CPack)
#in build/ : cpack -G ZIP -C Release

#----------------------------------------------------------------------------------------

#option(CURL_STATICLIB "Set to ON to build libcurl with static linking."  ON)
#if(WIN32)
#    add_definitions("-DCURL_STATICLIB")
#endif()
#set(CURL_LIBRARY "-lcurl") 
#find_package(CURL REQUIRED) 
#include_directories(${CURL_INCLUDE_DIR})
#
#if(LIBCURL_ENABLE)
#    target_link_libraries(${PROJECT_NAME} ${CURL_LIBRARIES})
#endif()
#
#set(OPENSSL_USE_STATIC_LIBS TRUE)
#find_package(OpenSSL REQUIRED)
#include_directories(${OPENSSL_INCLUDE_DIR})
#target_link_libraries(${PROJECT_NAME} ${OPENSSL_LIBRARIES})

FetchContent_Declare(
    3dstool_download
    GIT_REPOSITORY      https://github.com/dnasdw/3dstool.git
    GIT_TAG             9c4336bca8898f3860b41241b8a7d9d4a6772e79
    GIT_PROGRESS        TRUE
    PATCH_COMMAND       git apply "${CMAKE_CURRENT_SOURCE_DIR}/patches/3dstool.patch"
    UPDATE_DISCONNECTED 1
)
FetchContent_Populate(3dstool_download)
add_custom_command(
    OUTPUT ${3dstool_download_SOURCE_DIR}/src/utility.cpp
    COMMAND echo test
    COMMENT "running custom 3dstool command")
add_custom_target(3dstool_check ALL DEPENDS ${3dstool_download_SOURCE_DIR}/src/utility.cpp)
add_dependencies(nsui_banner_fixer 3dstool_check)
target_include_directories(nsui_banner_fixer PRIVATE ${3dstool_download_SOURCE_DIR}/src)
file(GLOB 3dstool_sources ${3dstool_download_SOURCE_DIR}/src/*.cpp)
target_sources(nsui_banner_fixer PRIVATE ${3dstool_sources})

if(MSVC_VERSION LESS 1600)
    target_include_directories(nsui_banner_fixer PRIVATE ${3dstool_download_SOURCE_DIR}/dep/src/capstone/msvc)
endif()
target_include_directories(nsui_banner_fixer PRIVATE ${3dstool_download_SOURCE_DIR}/dep/src/capstone-3.0.5/include)
file(GLOB capstone_sources 
    ${3dstool_download_SOURCE_DIR}/dep/src/capstone-3.0.5/include/*.h
    ${3dstool_download_SOURCE_DIR}/dep/src/capstone-3.0.5/*.h
    ${3dstool_download_SOURCE_DIR}/dep/src/capstone-3.0.5/*.c
    ${3dstool_download_SOURCE_DIR}/dep/src/capstone-3.0.5/arch/ARM/*.c
    ${3dstool_download_SOURCE_DIR}/dep/src/capstone-3.0.5/arch/ARM/*.h
    ${3dstool_download_SOURCE_DIR}/dep/src/capstone-3.0.5/arch/ARM/*.inc)
target_sources(nsui_banner_fixer PRIVATE ${capstone_sources})

#target_compile_definitions(nsui_banner_fixer PRIVATE SDW_MAIN CURL_STATICLIB OPENSSL_USE_STATIC_LIBS CAPSTONE_USE_SYS_DYN_MEM CAPSTONE_HAS_ARM _3DSTOOL_VERSION="1.2.6")
target_compile_definitions(nsui_banner_fixer PRIVATE SDW_MAIN CAPSTONE_USE_SYS_DYN_MEM CAPSTONE_HAS_ARM _3DSTOOL_VERSION="1.2.6")
if(APPLE)
    target_compile_definitions(nsui_banner_fixer PRIVATE SDW_XCONVERT)
endif()

if(WIN32)
  if(MSVC)
    if(MSVC_VERSION LESS 1800)
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4005")
    endif()
    target_link_libraries(nsui_banner_fixer PUBLIC libcurl libcrypto WS2_32 Wldap32 Crypt32)
    #target_link_libraries(nsui_banner_fixer PUBLIC WS2_32 Wldap32 Crypt32)
    set(nsui_banner_fixer_LINK_FLAGS_DEBUG "${nsui_banner_fixer_LINK_FLAGS_DEBUG} /NODEFAULTLIB:LIBCMT")
    if(MSVC_VERSION GREATER 1700)
      set(nsui_banner_fixer_LINK_FLAGS_DEBUG "${nsui_banner_fixer_LINK_FLAGS_DEBUG} /IGNORE:4099")
      set(nsui_banner_fixer_LINK_FLAGS_RELWITHDEBINFO "${nsui_banner_fixer_LINK_FLAGS_RELWITHDEBINFO} /IGNORE:4099")
    endif()
    set_target_properties(nsui_banner_fixer PROPERTIES
      LINK_FLAGS_DEBUG "${nsui_banner_fixer_LINK_FLAGS_DEBUG}"
      LINK_FLAGS_RELWITHDEBINFO "${nsui_banner_fixer_LINK_FLAGS_RELWITHDEBINFO}")
  else()
    #target_link_libraries(nsui_banner_fixer PUBLIC curl crypto)
  endif()
else()
  #target_link_libraries(nsui_banner_fixer PUBLIC curl ssl crypto)
  if(APPLE)
    target_link_libraries(nsui_banner_fixer PUBLIC ldap)
  else()
    target_link_libraries(nsui_banner_fixer PUBLIC pthread dl)
  endif()
  if(APPLE OR CYGWIN)
    target_link_libraries(nsui_banner_fixer PUBLIC iconv)
  endif()
endif()


if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(BUILD64 1)
endif()
include(${3dstool_download_SOURCE_DIR}/cmake/AddDep.cmake)
ADD_DEP_INCLUDE_DIR("${3dstool_download_SOURCE_DIR}/dep")
target_include_directories(nsui_banner_fixer PUBLIC ${DEP_INCLUDE_DIR})
ADD_DEP_LIBRARY_DIR("${3dstool_download_SOURCE_DIR}/dep")

if (WIN32)
    set(LIB_FILE_EXT lib)
else()
    set(LIB_FILE_EXT a)
endif()
set(3dstool_deps libssl libcrypto)#set(3dstool_deps libcurl libssl libcrypto)
#foreach(LIB IN LISTS 3dstool_deps)
#    add_library(${LIB} STATIC IMPORTED)
#    add_dependencies(${LIB} 3dstool_check)
#    target_include_directories(${LIB} INTERFACE ${DEP_INCLUDE_DIR})
#    set_target_properties(${LIB} PROPERTIES IMPORTED_LOCATION ${DEP_LIBRARY_DIR}/${LIB}.${LIB_FILE_EXT})
#    target_link_libraries(nsui_banner_fixer PUBLIC ${LIB})
#endforeach()
#target_link_libraries(libcrypto INTERFACE libssl)#target_link_libraries(libcrypto INTERFACE libssl libcurl)
#target_link_libraries(libcurl INTERFACE libssl libcrypto)
#target_link_libraries(libssl INTERFACE libcrypto)#target_link_libraries(libssl INTERFACE libcrypto libcurl)

find_package(CURL REQUIRED)
target_link_libraries(${PROJECT_NAME} PUBLIC CURL::libcurl)
find_package(OpenSSL)
target_link_libraries(${PROJECT_NAME} PUBLIC OpenSSL::SSL OpenSSL::Crypto)

#target_link_libraries(libcrypto INTERFACE dl)

target_compile_options(nsui_banner_fixer PUBLIC -Wl,--no-as-needed)



#----------------------------------------------------------------------------------------

FetchContent_Declare(
    project_ctr_download
    GIT_REPOSITORY      https://github.com/3DSGuy/Project_CTR.git
    GIT_TAG master
)
FetchContent_MakeAvailable(project_ctr_download)

FILE(
    COPY ${project_ctr_download_SOURCE_DIR}/ctrtool/src/ 
    DESTINATION ${project_ctr_download_SOURCE_DIR}/include/ctrtool
    FILES_MATCHING PATTERN "*.h")


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

target_link_libraries(nsui_banner_fixer PUBLIC ctrtool_lib)




if(UNIX OR MINGW)
  if(CYGWIN)
    #set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++11")
  else()
    #set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
    if(NOT APPLE)
      #set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static-libgcc -static-libstdc++")
      #set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static")
    endif()
  endif()
  if((UNIX AND BUILD64) OR (MINGW AND CMAKE_SIZEOF_VOID_P EQUAL 8))
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -m64")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m64")
  else()
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -m32")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m32")
  endif()
endif()
if(UNIX OR MINGW)
  add_definitions(-D_FILE_OFFSET_BITS=64)
  add_definitions(-Wno-multichar -Wno-shift-overflow -Wno-unused-result)
endif()

