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
# find_library(PFFFT_LIBRARY NAMES pffft PATHS ${CMAKE_SOURCE_DIR}/lib/pffft)

# find_package(PFFFT REQUIRED)
# include_directories("/path/to/pffft/headers")

# if(NOT PFFFT_LIBRARY)
#     message(FATAL_ERROR "pffft library not found")
# endif()

# add_subdirectory("../lib/pffft")

# include("../lib/pffft/CMakeLists.txt")

add_executable_m7(pffft
    pffft.cc
    ../lib/pffft/pffft.c
)

target_link_libraries(pffft
    libs_base-m7_freertos
)

include_directories("../lib/pffft")
