#Inspired by https://github.com/jasonyang-ee/STM32-CMAKE-TEMPLATE
cmake_minimum_required(VERSION 3.29)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)
set(CMAKE_C_STANDARD                11)
set(CMAKE_C_STANDARD_REQUIRED       ON)
set(CMAKE_C_EXTENSIONS              ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS   ON)
set(CMAKE_TOOLCHAIN_FILE            gcc-arm-none-eabi)
project(AxxSolder C ASM)
set(linker_script_SRC               ${CMAKE_SOURCE_DIR}/STM32G431CBTX_FLASH.ld)
set(EXECUTABLE                      ${CMAKE_PROJECT_NAME})

file(GLOB source_list
    ${CMAKE_SOURCE_DIR}/Core/Src/*.c
    ${CMAKE_SOURCE_DIR}/Drivers/STM32G4xx_HAL_Driver/Src/*.c
    ${CMAKE_SOURCE_DIR}/Drivers/LCD/*.c
    ${CMAKE_SOURCE_DIR}/Drivers/UGUI/*.c
    ${CMAKE_SOURCE_DIR}/Drivers/UGUI/Fonts/*.c
)

set(include_list ${include_list}
    ${CMAKE_SOURCE_DIR}/Core/Inc
    ${CMAKE_SOURCE_DIR}/Drivers/LCD
    ${CMAKE_SOURCE_DIR}/Drivers/UGUI
    ${CMAKE_SOURCE_DIR}/Drivers/CMSIS/Device/ST/STM32G4xx/Include
    ${CMAKE_SOURCE_DIR}/Drivers/CMSIS/Include
    ${CMAKE_SOURCE_DIR}/Drivers/STM32G4xx_HAL_Driver/Inc
    ${CMAKE_SOURCE_DIR}/Drivers/STM32G4xx_HAL_Driver/Inc/Legacy
)

add_executable(${EXECUTABLE} ${source_list} ${CMAKE_SOURCE_DIR}/Core/Startup/startup_stm32g431cbtx.s)

target_include_directories(${EXECUTABLE} PRIVATE ${include_list})

target_compile_definitions(${EXECUTABLE} PRIVATE 
    "USE_HAL_DRIVER"
    "STM32G431xx"
)

set(CPU_PARAMETERS ${CPU_PARAMETERS}
	-mthumb
	-mcpu=cortex-m4
	-mfpu=fpv4-sp-d16
	-mfloat-abi=hard
)

target_compile_options(${EXECUTABLE} PRIVATE
    ${CPU_PARAMETERS}
	-Wall
	-Wpedantic
	-Wno-unused-parameter
)

target_link_options(${EXECUTABLE} PRIVATE
	-T${linker_script_SRC}
	${CPU_PARAMETERS}
	-Wl,-Map=${CMAKE_PROJECT_NAME}.map
	--specs=nosys.specs
	-u _printf_float                # STDIO float formatting support
	-Wl,--start-group
	-lc
	-lm
	-Wl,--end-group
	-Wl,--print-memory-usage
)

add_custom_command(TARGET ${EXECUTABLE} POST_BUILD
	COMMAND ${CMAKE_SIZE} $<TARGET_FILE:${EXECUTABLE}>
)

add_custom_command(TARGET ${EXECUTABLE} POST_BUILD
	COMMAND ${CMAKE_OBJCOPY} -O ihex $<TARGET_FILE:${EXECUTABLE}> ${EXECUTABLE}.hex
)

add_custom_command(TARGET ${EXECUTABLE} POST_BUILD
	COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:${EXECUTABLE}> ${EXECUTABLE}.bin
)