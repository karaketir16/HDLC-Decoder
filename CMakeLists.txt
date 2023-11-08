cmake_minimum_required(VERSION 3.12)

# Pull in SDK (must be before project)
include(pico_sdk_import.cmake)

project(hdlc_decoder C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

if (PICO_SDK_VERSION_STRING VERSION_LESS "1.3.0")
    message(FATAL_ERROR "Raspberry Pi Pico SDK version 1.3.0 (or later) required. Your version is ${PICO_SDK_VERSION_STRING}")
endif()


# Initialize the SDK
pico_sdk_init()




if (TARGET tinyusb_device)
    add_executable(${PROJECT_NAME}
            src/main.cpp
            )

    # Add pico_multicore which is required for multicore functionality
    target_link_libraries(${PROJECT_NAME}
            pico_stdlib
            pico_multicore)

    # create map/bin/hex file etc.
    

    add_compile_options(-Wall)

    if (CMAKE_C_COMPILER_ID STREQUAL "GNU")
        add_compile_options(-Wno-maybe-uninitialized)
    endif()

    # enable usb output, disable uart output
    pico_enable_stdio_usb(${PROJECT_NAME} 1)
    pico_enable_stdio_uart(${PROJECT_NAME} 0)

    pico_add_extra_outputs(${PROJECT_NAME})

elseif(PICO_ON_DEVICE)
    message(WARNING "not building hello_usb because TinyUSB submodule is not initialized in the SDK")
endif()
