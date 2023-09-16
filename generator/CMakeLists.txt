
add_executable_m7(generator
    generator_cm7.cc
    M4_EXECUTABLE
    generator_m4
)

target_link_libraries(generator
    libs_base-m7_freertos
)

add_executable_m4(generator_m4
    generator_cm4.cc
)

target_link_libraries(generator_m4
    libs_base-m4_freertos
)