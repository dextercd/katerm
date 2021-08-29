add_custom_command(
    OUTPUT
        k/katerm
    DEPENDS
        term.info
    COMMAND
        tic -sx ${CMAKE_CURRENT_SOURCE_DIR}/term.info
            -o ${CMAKE_CURRENT_BINARY_DIR}
)

add_custom_target(terminfo ALL
    DEPENDS
        k/katerm)
