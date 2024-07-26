set(CMSIS_PATH "${CMAKE_SOURCE_DIR}/coralmicro/third_party/CMSIS/CMSIS")
set(CMSIS_DSP_PATH "${CMAKE_SOURCE_DIR}/lib/CMSIS-DSP")
set(CMSIS_CORE_PATH "${CMSIS_PATH}/Core/Include")
set(CMSIS_INCLUDE_PATH "${CMSIS_DSP_PATH}/Include")
set(CMSIS_PRIVAT_PATH "${CMSIS_DSP_PATH}/PrivateInclude")

include_directories(${CMSIS_CORE_PATH})
include_directories(${CMSIS_INCLUDE_PATH})
include_directories(${CMSIS_PRIVAT_PATH})

set(libs_FreeRTOS_INCLUDE_DIRECTORIES_PUBLIC
    ${CMAKE_SOURCE_DIR}/coralmicro/third_party/modified/FreeRTOS
    ${CMAKE_SOURCE_DIR}/coralmicro/third_party/modified/nxp/rt1176-sdk/rtos/freertos/freertos_kernel/include
    ${CMAKE_SOURCE_DIR}/coralmicro/third_party/freertos_kernel/include
    ${CMAKE_SOURCE_DIR}/coralmicro/third_party/freertos_kernel/portable/GCC/ARM_CM4F
    )
include_directories(${libs_FreeRTOS_INCLUDE_DIRECTORIES_PUBLIC})

add_library_m7(lib_fCWT STATIC fcwt.cc)

target_link_libraries(lib_fCWT lib_fft)

set(CMAKE_C_FLAGS
    "${CMAKE_C_FLAGS} -mcpu=cortex-m7 -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -Os -g0"
)
set(CMAKE_CXX_FLAGS
    "${CMAKE_CXX_FLAGS} -mcpu=cortex-m7 -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -Os -g0"
)


