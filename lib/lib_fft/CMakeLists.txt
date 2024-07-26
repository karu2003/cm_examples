set(CMSIS_PATH "${CMAKE_SOURCE_DIR}/coralmicro/third_party/CMSIS/CMSIS")
set(CMSIS_DSP_PATH "${CMAKE_SOURCE_DIR}/lib/CMSIS-DSP")
set(CMSIS_CORE_PATH "${CMSIS_PATH}/Core/Include")
set(CMSIS_INCLUDE_PATH "${CMSIS_DSP_PATH}/Include")
set(CMSIS_PRIVAT_PATH "${CMSIS_DSP_PATH}/PrivateInclude")

include_directories(${CMSIS_CORE_PATH})
include_directories(${CMSIS_INCLUDE_PATH})
include_directories(${CMSIS_PRIVAT_PATH})

set(CMAKE_C_FLAGS
    "${CMAKE_C_FLAGS} -mcpu=cortex-m7 -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -Os -g0")
set(CMAKE_CXX_FLAGS
    "${CMAKE_CXX_FLAGS} -mcpu=cortex-m7 -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -Os -g0")


file(GLOB CMSIS_DSP_SOURCES "${CMSIS_DSP_PATH}/Source/TransformFunctions/*.c"
     "${CMSIS_DSP_PATH}/Source/CommonTables/*.c" "${CMSIS_DSP_PATH}/Source/StatisticsFunctions/*.c" "${CMSIS_DSP_PATH}/Source/FilteringFunctions/*.c")

# add_library(lib_fft STATIC ${CMSIS_DSP_SOURCES})
add_library_m7(lib_fft STATIC ${CMSIS_DSP_SOURCES})