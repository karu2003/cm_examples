set(CMSIS_PATH "${CMAKE_SOURCE_DIR}/coralmicro/third_party/CMSIS/CMSIS")
set(CMSIS_DSP_PATH "${CMAKE_SOURCE_DIR}/lib/CMSIS-DSP")
set(CMSIS_CORE_PATH "${CMSIS_PATH}/Core/Include")
set(CMSIS_INCLUDE_PATH "${CMSIS_DSP_PATH}/Include")
set(CMSIS_PRIVAT_PATH "${CMSIS_DSP_PATH}/PrivateInclude")

# message(STATUS "CMSIS_PATH: ${CMSIS_PATH}")
# message(STATUS "CMSIS_DSP_PATH:${CMSIS_DSP_PATH}")
# message(STATUS "CMSIS_CORE_PATH: ${CMSIS_CORE_PATH}")
# message(STATUS "CMSIS_INCLUDE_PATH: ${CMSIS_INCLUDE_PATH}")
# message(STATUS "CMSIS_PRIVAT_PATH: ${CMSIS_PRIVAT_PATH}")

include_directories(${CMSIS_CORE_PATH})
include_directories(${CMSIS_INCLUDE_PATH})
include_directories(${CMSIS_PRIVAT_PATH})

include_directories(${CMAKE_SOURCE_DIR}/lib/lib_fCWT)

set(CMAKE_C_FLAGS
    "${CMAKE_C_FLAGS} -mcpu=cortex-m7 -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -Os -g0"
)
set(CMAKE_CXX_FLAGS
    "${CMAKE_CXX_FLAGS} -mcpu=cortex-m7 -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -Os -g0"
)

add_executable_m7(main_fCWT arm_fCWT.cc)# fcwt.cc)

# file(
#   GLOB
#   CMSIS_DSP_SOURCES
#   "${CMSIS_DSP_PATH}/Source/TransformFunctions/*.c"
#   "${CMSIS_DSP_PATH}/Source/CommonTables/*.c"
#   "${CMSIS_DSP_PATH}/Source/StatisticsFunctions/*.c"
#   "${CMSIS_DSP_PATH}/Source/FilteringFunctions/*.c")

# add_library(CMSIS_DSP STATIC ${CMSIS_DSP_SOURCES})

# target_link_libraries(fCWT libs_base-m7_nocam_freertos CMSIS_DSP)
target_link_libraries(main_fCWT libs_base-m7_nocam_freertos lib_fft lib_fCWT)
# add_link_options(-Wl,--gc-sections)
