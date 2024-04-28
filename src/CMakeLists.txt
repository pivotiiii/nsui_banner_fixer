add_executable(nsui_banner_fixer nsui_banner_fixer.cpp)
target_compile_features(nsui_banner_fixer PRIVATE cxx_std_20)

target_compile_definitions(nsui_banner_fixer PRIVATE VERSION=\"${CMAKE_PROJECT_VERSION}\")
string(TIMESTAMP year %Y UTC)
target_compile_definitions(nsui_banner_fixer PRIVATE YEAR="${year}")
string(TIMESTAMP compile_time "%Y-%m-%d %H:%M:%S UTC" UTC)
target_compile_definitions(nsui_banner_fixer PRIVATE COMPILE_TIME="${compile_time}")

add_library(nsui_banner_fixer_game 
            Game.hpp 
            Game.cpp 
            Settings.hpp)
target_compile_features(nsui_banner_fixer_game PRIVATE cxx_std_20)
target_include_directories(nsui_banner_fixer_game INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(nsui_banner_fixer PRIVATE nsui_banner_fixer_game TCLAP)

#----------------------------------------------------------------------------------------
# Windows
#----------------------------------------------------------------------------------------

if (CMAKE_SYSTEM_NAME MATCHES "Windows")
        #target_sources(nsui_banner_fixer_game PRIVATE GameWin.cpp)
        target_link_libraries(nsui_banner_fixer_game PRIVATE Boost::process)
        add_custom_command(
                TARGET nsui_banner_fixer POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E make_directory
                        $<TARGET_FILE_DIR:nsui_banner_fixer>/tools
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                        ${3dstool_SOURCE_DIR}/3dstool.exe
                        ${ctrtool_SOURCE_DIR}/ctrtool.exe
                        ${makerom_SOURCE_DIR}/makerom.exe
                        $<TARGET_FILE_DIR:nsui_banner_fixer>/tools)
endif()

#----------------------------------------------------------------------------------------
# Linux
#----------------------------------------------------------------------------------------

if(CMAKE_SYSTEM_NAME MATCHES "Linux")
        target_sources(nsui_banner_fixer_game PRIVATE Tool.hpp Tool.cpp)
        target_link_libraries(nsui_banner_fixer_game PRIVATE stdcapture)

        target_include_directories(nsui_banner_fixer_game PRIVATE ${3dstool_download_SOURCE_DIR}/src)
        target_sources(nsui_banner_fixer_game PRIVATE ${3dstool_sources})

        target_include_directories(nsui_banner_fixer_game PRIVATE ${3dstool_download_SOURCE_DIR}/dep/src/capstone-3.0.5/include)
        target_sources(nsui_banner_fixer_game PRIVATE ${capstone_sources})
        target_compile_definitions(nsui_banner_fixer_game PRIVATE SDW_MAIN CAPSTONE_USE_SYS_DYN_MEM CAPSTONE_HAS_ARM _3DSTOOL_VERSION="1.2.6")

        if(DYNAMIC_LINKING)
            target_link_libraries(nsui_banner_fixer_game PRIVATE CURL::libcurl)
            target_link_libraries(nsui_banner_fixer_game PRIVATE OpenSSL::SSL OpenSSL::Crypto)
        else()
            target_compile_definitions(nsui_banner_fixer_game PRIVATE CURL_STATICLIB OPENSSL_USE_STATIC_LIBS)
            target_include_directories(nsui_banner_fixer_game PRIVATE ${DEP_INCLUDE_DIR})
            target_link_libraries(nsui_banner_fixer_game PRIVATE libcurl libssl libcrypto)
        endif()

        if(STATIC_STD_LIBS)
                target_compile_options(nsui_banner_fixer_game PRIVATE -static-libgcc -static-libstdc++)
        endif()

        if(CMAKE_SIZEOF_VOID_P EQUAL 8)
            target_compile_options(nsui_banner_fixer_game PRIVATE -m64)
        else()
            target_compile_options(nsui_banner_fixer_game PRIVATE -m32)
        endif()
        target_compile_definitions(nsui_banner_fixer_game PUBLIC _FILE_OFFSET_BITS=64)
        target_compile_options(nsui_banner_fixer_game PRIVATE -Wno-multichar -Wno-shift-overflow -Wno-unused-result)
        target_link_libraries(nsui_banner_fixer_game PRIVATE pthread dl)

        target_link_libraries(nsui_banner_fixer_game PUBLIC ctrtool_lib)
        target_link_libraries(nsui_banner_fixer_game PRIVATE makerom_lib)
endif()



include(generate_product_version)
generate_product_version(ProductVersionFiles
                        NAME ${CMAKE_PROJECT_NAME}
                        ICON ${CMAKE_SOURCE_DIR}/resources/icon.ico
                        VERSION_MAJOR ${CMAKE_PROJECT_VERSION_MAJOR}
                        VERSION_MINOR ${CMAKE_PROJECT_VERSION_MINOR}
                        VERSION_PATCH ${CMAKE_PROJECT_VERSION_PATCH}
                        COMPANY_NAME pivotiii
                        FILE_DESCRIPTION ${CMAKE_PROJECT_DESCRIPTION})
target_sources(nsui_banner_fixer PRIVATE ${ProductVersionFiles})



install(TARGETS nsui_banner_fixer DESTINATION .)