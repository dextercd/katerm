find_program(TIC_PROGRAM tic)

if (NOT TIC_PROGRAM)
    message(WARNING "tic program not found, won't build terminfo file")
else()
    add_custom_command(
        OUTPUT
            k/katerm-color
        DEPENDS
            term.info
        COMMAND
            tic -sx ${CMAKE_CURRENT_SOURCE_DIR}/term.info
                -o ${CMAKE_CURRENT_BINARY_DIR}
    )

    add_custom_target(terminfo ALL
        DEPENDS
            k/katerm-color
    )
endif()
