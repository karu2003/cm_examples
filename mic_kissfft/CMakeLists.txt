# Copyright 2022 Google LLC
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

# find_package(PythonInterp REQUIRED)
find_package(Python3 REQUIRED)
message(STATUS "Python3_EXECUTABLE: ${Python3_EXECUTABLE}")

set (CMAKE_INCLUDE_CURRENT_DIR ON)

add_custom_command(
    PRE_BUILD
    # OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/test_signal.h"
    OUTPUT "test_signal.h"
    # COMMAND "${PYTHON_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/test_signal.py"
    COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/test_signal.py"
    #DEPENDS mic_kissfft
    )

# add_custom_command(
#     OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/test_signal.h"
#     COMMAND Python3::Interpreter "${CMAKE_CURRENT_SOURCE_DIR}/test_signal.py"
#     COMMENT "Generating test_signal.h using Python script"
#     VERBATIM
# )
 

add_executable_m7(mic_kissfft
    mic_kissfft.cc
    kiss_fft.cpp
    kiss_fftr.cpp
    hamming.cpp
    # "${CMAKE_CURRENT_BINARY_DIR}/test_signal.h"
    test_signal.h
)

target_link_libraries(mic_kissfft
    libs_base-m7_freertos
)

