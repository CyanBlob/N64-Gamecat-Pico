add_library(n64_pi_console INTERFACE)
add_library(n64_pi_cart INTERFACE)

target_sources(n64_pi_console INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/n64_pi.c
)

target_sources(n64_pi_cart INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/n64_pi.c
)

pico_generate_pio_header(n64_pi_console
    ${CMAKE_CURRENT_LIST_DIR}/n64_pi_console.pio
)

pico_generate_pio_header(n64_pi_cart
    ${CMAKE_CURRENT_LIST_DIR}/n64_pi_cart.pio
)

target_include_directories(n64_pi_console INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/include
)

target_include_directories(n64_pi_cart INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/include
)

target_link_libraries(n64_pi_console INTERFACE
    hardware_pio
)

target_link_libraries(n64_pi_cart INTERFACE
    hardware_pio
)