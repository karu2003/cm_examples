cmake_minimum_required(VERSION 3.10)
project(test_cwt)

set(CMAKE_CXX_STANDARD 14)
# Find Python3 libraries
find_package(Python3 COMPONENTS Interpreter Development REQUIRED)
include_directories(${Python3_INCLUDE_DIRS})

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../dac_out/)


add_executable(test_cwt
    test_cwt.cc
    ../cwt.cc
    ../../dac_out/chirp.cc)

add_executable(plot_cwt
    plot_cwt.cc
    ../cwt.cc
    ../../dac_out/chirp.cc)

add_executable(imshow
    imshow.cc)

include_directories("../../lib/eigen-3.4")
include_directories("../../lib/matplotlibcpp")
include_directories("/usr/lib/python3/dist-packages/numpy/core/include")
# add_subdirectory(${CMAKE_SOURCE_DIR}/../../lib/matplotlibcpp)

# target_link_libraries(test_cwt ${PYTHON_LIBRARIES})
# target_link_libraries(imshow  ${PYTHON_LIBRARIES})
# target_link_libraries(test_cwt Python3::Python)
target_link_libraries(imshow Python3::Python)
target_link_libraries(plot_cwt Python3::Python)


