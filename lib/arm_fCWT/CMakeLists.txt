cmake_minimum_required(VERSION 3.10)

project(arm_fCWT)

set(CMSIS_PATH "${CMAKE_SOURCE_DIR}/coralmicro/third_party/CMSIS/CMSIS")
set(CMSIS_DSP_PATH "${CMAKE_SOURCE_DIR}/lib/CMSIS-DSP")
# set(CMSIS_DSP_PATH "${CMSIS_PATH}/DSP")
set(CMSIS_CORE_PATH "${CMSIS_PATH}/Core/Include")
set(CMSIS_INCLUDE_PATH "${CMSIS_DSP_PATH}/Include")
set(CMSIS_PRIVAT_PATH "${CMSIS_DSP_PATH}/PrivateInclude")

# message(STATUS "CMSIS_PATH: ${CMSIS_PATH}") message(STATUS "CMSIS_DSP_PATH:
# ${CMSIS_DSP_PATH}") message(STATUS "CMSIS_CORE_PATH: ${CMSIS_CORE_PATH}")
# message(STATUS "CMSIS_INCLUDE_PATH: ${CMSIS_INCLUDE_PATH}") message(STATUS
# "CMSIS_PRIVAT_PATH: ${CMSIS_PRIVAT_PATH}")

include_directories(${CMSIS_CORE_PATH})
include_directories(${CMSIS_INCLUDE_PATH})
include_directories(${CMSIS_PRIVAT_PATH})

set(libs_FreeRTOS_INCLUDE_DIRECTORIES_PUBLIC
    ${CMAKE_SOURCE_DIR}/third_party/modified/FreeRTOS
    ${CMAKE_SOURCE_DIR}/third_party/modified/nxp/rt1176-sdk/rtos/freertos/freertos_kernel/include
    ${CMAKE_SOURCE_DIR}/third_party/freertos_kernel/include
    ${CMAKE_SOURCE_DIR}/third_party/freertos_kernel/portable/GCC/ARM_CM4F)
include_directories(${libs_FreeRTOS_INCLUDE_DIRECTORIES_PUBLIC})
include_directories(lib/arm_fCWT)

target_include_directories(
  libs_FreeRTOS
  PUBLIC ${libs_FreeRTOS_INCLUDE_DIRECTORIES_PUBLIC}
  PRIVATE ${libs_FreeRTOS_INCLUDE_DIRECTORIES_PRIVATE})

message(STATUS "CMAKE_SOURCE_DIR: ${CMAKE_SOURCE_DIR}")
message(STATUS "PROJECT_SOURCE_DIR: ${PROJECT_SOURCE_DIR}")

set(CMAKE_C_FLAGS
    "${CMAKE_C_FLAGS} -mcpu=cortex-m7 -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -Os -g0"
)
set(CMAKE_CXX_FLAGS
    "${CMAKE_CXX_FLAGS} -mcpu=cortex-m7 -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -Os -g0"
)

add_library(arm_fCWT STATIC fcwt.cc)
