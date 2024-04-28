include(FetchContent)

FetchContent_Declare(
    cmakehelpers
    GIT_REPOSITORY      https://github.com/halex2005/CMakeHelpers.git
    GIT_TAG             0814803829022ce3f326b14348971ff556a1dee3 #
)

FetchContent_MakeAvailable(cmakehelpers)
list(APPEND CMAKE_MODULE_PATH "${cmakehelpers_SOURCE_DIR}")

#----------------------------------------------------------------------------------------

FetchContent_Declare(
    tclap
    GIT_REPOSITORY      https://git.code.sf.net/p/tclap/code
    GIT_TAG             4dd8ae4d5c7354db2f38acd76a226e593522c68f #
    PATCH_COMMAND       git apply "${CMAKE_SOURCE_DIR}/patches/tclap.patch"
    UPDATE_DISCONNECTED 1
)
FetchContent_MakeAvailable(tclap)

#----------------------------------------------------------------------------------------
# Windows
#----------------------------------------------------------------------------------------

if(CMAKE_SYSTEM_NAME MATCHES "Windows")
    FetchContent_Declare(
      Boost
      URL https://github.com/boostorg/boost/releases/download/boost-1.84.0/boost-1.84.0.tar.xz
      URL_MD5 893b5203b862eb9bbd08553e24ff146a
      DOWNLOAD_EXTRACT_TIMESTAMP ON
      EXCLUDE_FROM_ALL
    )
    set(BOOST_INCLUDE_LIBRARIES process)
    FetchContent_MakeAvailable(Boost)

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

    #----------------------------------------------------------------------------------------

    install(FILES ${3dstool_SOURCE_DIR}/3dstool.exe DESTINATION tools)
    install(FILES ${ctrtool_SOURCE_DIR}/ctrtool.exe DESTINATION tools)
    install(FILES ${makerom_SOURCE_DIR}/makerom.exe DESTINATION tools)
endif()

#----------------------------------------------------------------------------------------
# Linux
#----------------------------------------------------------------------------------------

if (CMAKE_SYSTEM_NAME MATCHES "Linux")
    FetchContent_Declare(
        stdcapture
        GIT_REPOSITORY      https://github.com/dmikushin/stdcapture.git
        GIT_TAG             25ea65ba7933c4ce7baa48dcc90063476d539586 #
    )
    FetchContent_MakeAvailable(stdcapture)

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
    file(GLOB 3dstool_sources ${3dstool_download_SOURCE_DIR}/src/*.cpp)

    file(GLOB capstone_sources 
    ${3dstool_download_SOURCE_DIR}/dep/src/capstone-3.0.5/include/*.h
    ${3dstool_download_SOURCE_DIR}/dep/src/capstone-3.0.5/*.h
    ${3dstool_download_SOURCE_DIR}/dep/src/capstone-3.0.5/*.c
    ${3dstool_download_SOURCE_DIR}/dep/src/capstone-3.0.5/arch/ARM/*.c
    ${3dstool_download_SOURCE_DIR}/dep/src/capstone-3.0.5/arch/ARM/*.h
    ${3dstool_download_SOURCE_DIR}/dep/src/capstone-3.0.5/arch/ARM/*.inc)

    if(DYNAMIC_LINKING)
        find_package(CURL REQUIRED)
        find_package(OpenSSL REQUIRED)
    else()
        if(CMAKE_SIZEOF_VOID_P EQUAL 8)
            set(BUILD64 1)
        endif()
        include(${3dstool_download_SOURCE_DIR}/cmake/AddDep.cmake)
        ADD_DEP_INCLUDE_DIR("${3dstool_download_SOURCE_DIR}/dep")
        ADD_DEP_LIBRARY_DIR("${3dstool_download_SOURCE_DIR}/dep")
        set(3dstool_deps libcurl libssl libcrypto)
        foreach(LIB IN LISTS 3dstool_deps)
            add_library(${LIB} STATIC IMPORTED)
            add_dependencies(${LIB} 3dstool_check)
            target_include_directories(${LIB} INTERFACE ${DEP_INCLUDE_DIR})
            set_target_properties(${LIB} PROPERTIES IMPORTED_LOCATION ${DEP_LIBRARY_DIR}/${LIB}.a)
        endforeach()
        target_link_libraries(libcrypto INTERFACE libssl libcurl)
        target_link_libraries(libcurl INTERFACE libssl libcrypto)
        target_link_libraries(libssl INTERFACE libcrypto libcurl)
        target_link_libraries(libcrypto INTERFACE dl)
    endif()

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

    #----------------------------------------------------------------------------------------

    FILE(
        COPY ${project_ctr_download_SOURCE_DIR}/makerom/src/ 
        DESTINATION ${project_ctr_download_SOURCE_DIR}/include/makerom
        FILES_MATCHING PATTERN "*.h")
    
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

    add_library(libmbedtls2 STATIC IMPORTED)
    add_dependencies(libmbedtls2 makerom_make)
    target_include_directories(libmbedtls2 INTERFACE ${project_ctr_download_SOURCE_DIR}/makerom/deps/libmbedtls/include)
    set_target_properties(libmbedtls2 PROPERTIES IMPORTED_LOCATION ${project_ctr_download_SOURCE_DIR}/makerom/deps/libmbedtls/bin/libmbedtls.a)
    target_link_libraries(makerom_lib INTERFACE libmbedtls2)
endif()


