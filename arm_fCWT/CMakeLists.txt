# Copyright 2022 Google LLC
#
# Licensed under the Apache License, Version 2.0 (the "License"); you may not
# use this file except in compliance with the License. You may obtain a copy of
# the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
# WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
# License for the specific language governing permissions and limitations under
# the License.

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

# add_definitions(-DAVX)

set(CMAKE_C_FLAGS
    "${CMAKE_C_FLAGS} -mcpu=cortex-m7 -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb")
set(CMAKE_CXX_FLAGS
    "${CMAKE_CXX_FLAGS} -mcpu=cortex-m7 -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb"
)

add_executable_m7(fCWT arm_fCWT.cc fcwt.cc)

file(GLOB CMSIS_DSP_SOURCES "${CMSIS_DSP_PATH}/Source/TransformFunctions/*.c"
     "${CMSIS_DSP_PATH}/Source/CommonTables/*.c" "${CMSIS_DSP_PATH}/Source/StatisticsFunctions/*.c" "${CMSIS_DSP_PATH}/Source/FilteringFunctions/*.c")

add_library(CMSIS_DSP STATIC ${CMSIS_DSP_SOURCES})

if(NOT CMSIS_DSP_SOURCES)
  message(FATAL_ERROR "No CMSIS DSP sources found")
endif()

target_link_libraries(fCWT libs_base-m7_freertos CMSIS_DSP)
