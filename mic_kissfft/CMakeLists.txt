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

find_package(Python3 REQUIRED)
message(STATUS "Python3_EXECUTABLE: ${Python3_EXECUTABLE}")

set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_custom_command(
  OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/test_signal.h"
  COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/test_signal.py"
  DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/test_signal.py"
  COMMENT "Generating test_signal.h using Python script")

add_custom_target(generate_test_signal_h
                  DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/test_signal.h")

add_executable_m7(mic_kissfft mic_kissfft.cc ../lib/kissfft/kiss_fft.c
                  ../lib/kissfft/kiss_fftr.c ../lib/hamming/hamming.cpp)

add_dependencies(mic_kissfft generate_test_signal_h)
target_link_libraries(mic_kissfft libs_base-m7_nocam_freertos)

include_directories("../lib/kissfft")
include_directories("../lib/hamming")
