project(nsui_banner_fixer_tests LANGUAGES CXX)

set(test_names 
            test_fix_banner
            test_get_version)
set(test_files test_v28.cia test_v27.cia)

foreach(test_name IN LISTS test_names)
    add_executable(${test_name} ${test_name}.cpp)
    target_compile_features(${test_name} PRIVATE cxx_std_20)
    target_link_libraries(${test_name} PRIVATE nsui_banner_fixer_game)
    if(CMAKE_SYSTEM_NAME MATCHES Windows)
        target_link_libraries(${test_name} PRIVATE Boost::process)
    endif()
    add_test(NAME ${test_name} COMMAND $<TARGET_FILE:${test_name}> WORKING_DIRECTORY $<TARGET_FILE_DIR:${test_name}>)
endforeach()

list(GET test_names 0 DUMMY_TARGET)

if(CMAKE_SYSTEM_NAME MATCHES Windows)
    add_custom_command(
            TARGET  ${DUMMY_TARGET} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E make_directory
                    $<TARGET_FILE_DIR:${DUMMY_TARGET}>/tools
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    ${3dstool_SOURCE_DIR}/3dstool.exe
                    ${ctrtool_SOURCE_DIR}/ctrtool.exe
                    ${makerom_SOURCE_DIR}/makerom.exe
                    $<TARGET_FILE_DIR:${DUMMY_TARGET}>/tools)
endif()

foreach(test_file IN LISTS test_files)
    add_custom_command(
        TARGET  ${DUMMY_TARGET} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
                ${CMAKE_CURRENT_SOURCE_DIR}/${test_file}
                $<TARGET_FILE_DIR:${DUMMY_TARGET}>)
endforeach()