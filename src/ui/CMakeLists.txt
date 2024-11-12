add_custom_command(
    OUTPUT  ${CMAKE_BINARY_DIR}/gui/embedded/all.hpp
    COMMAND npx.cmd saucer embed ${CMAKE_SOURCE_DIR}/src/ui ${CMAKE_BINARY_DIR}/ui/embedded
    DEPENDS ${CMAKE_SOURCE_DIR}/src/ui/index.html
            ${CMAKE_SOURCE_DIR}/src/ui/script.js
            ${CMAKE_SOURCE_DIR}/src/ui/style.css
    COMMENT Generating embedded files for GUI
)

target_sources(nsui_banner_fixer PRIVATE ${CMAKE_BINARY_DIR}/gui/embedded/all.hpp) #necessary so it the command will be run at all
target_include_directories(nsui_banner_fixer PRIVATE ${CMAKE_BINARY_DIR}/gui)

add_custom_target(html_conversion
    COMMAND npx.cmd saucer embed ${CMAKE_SOURCE_DIR}/src/ui ${CMAKE_BINARY_DIR}/gui/embedded)

add_dependencies(nsui_banner_fixer html_conversion)