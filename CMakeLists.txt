# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.0.0)
set(toolchainVersion 13_2_Rel1)
set(picotoolVersion 2.0.0)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
cmake_minimum_required(VERSION 3.13)

set(PICO_BOARD pico_w CACHE STRING "Board type")

include(pico_sdk_import.cmake)

project(LightFlow C CXX ASM)

pico_sdk_init()

add_executable(LightFlow
    LightFlow.c
    lib/ssd1306.c
	lib/matriz.c
)

# Adiciona diretórios de inclusão
include_directories(LightFlow PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
	${CMAKE_CURRENT_LIST_DIR}/lib
)

# Adiciona o arquivo PIO
file(MAKE_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/generated)
pico_generate_pio_header(LightFlow ${CMAKE_CURRENT_LIST_DIR}/ws2812.pio)

target_link_libraries(LightFlow
    pico_stdlib
    hardware_i2c
    hardware_uart
    hardware_adc
    hardware_pwm
)

pico_enable_stdio_usb(LightFlow 1)
pico_enable_stdio_uart(LightFlow 0)

pico_add_extra_outputs(LightFlow)