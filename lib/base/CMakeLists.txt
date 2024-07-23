add_library_m7(libs_base-m7_nocam_freertos STATIC
    main_freertos_m7_nocam.cc
    ../../coralmicro/libs/base/analog.cc
    ../../coralmicro/libs/base/console_m7.cc
    ../../coralmicro/libs/base/filesystem.cc
    ../../coralmicro/libs/base/gpio.cc
    ../../coralmicro/libs/base/i2c.cc
    ../../coralmicro/libs/base/ipc.cc
    ../../coralmicro/libs/base/ipc_m7.cc
    ../../coralmicro/libs/base/led.cc
    ../../coralmicro/libs/base/network.cc
    ../../coralmicro/libs/base/ntp.cc
    ../../coralmicro/libs/base/pwm.cc
    ../../coralmicro/libs/base/random.cc
    ../../coralmicro/libs/base/reset.cc
    ../../coralmicro/libs/base/strings.cc
    ../../coralmicro/libs/base/spi.cc
    ../../coralmicro/libs/base/tempsense.cc
    ../../coralmicro/libs/base/timer.cc
    ../../coralmicro/libs/base/utils.cc
    ../../coralmicro/libs/base/watchdog.cc
)

set(libs_base-m7_nocam_freertos_LINK_LIBRARIES_REVISION
        libs_audio_freertos
        libs_pmic_freertos
        libs_littlefs
        libs_nxp_rt1176-sdk_lwip
        libs_nxp_rt1176-sdk_lwip_httpd
)

target_link_libraries(libs_base-m7_nocam_freertos
    libs_nxp_rt1176-sdk-mbedtls
    libs_nxp_rt1176-sdk_freertos
    libs_nxp_rt1176-sdk-mcmgr_m7
    libs_cdc_acm_freertos
    libs_cdc_eem_freertos
    libs_tpu_dfu_task_freertos
    libs_tpu_task_freertos
    libs_usb_device_task_freertos
    libs_usb_host_task_freertos
    ${libs_base-m7_freertos_LINK_LIBRARIES_REVISION}
)

#target_link_libraries(libs_base-m7_nocam_freertos
#    libs_base-m7_freertos
#)

target_sources(libs_base-m7_nocam_freertos PUBLIC $<TARGET_OBJECTS:libs_nxp_rt1176-sdk_freertos>)
target_sources(libs_base-m7_nocam_freertos PUBLIC $<TARGET_OBJECTS:libs_FreeRTOS>)
target_sources(libs_base-m7_nocam_freertos PUBLIC $<TARGET_OBJECTS:libs_nxp_rt1176-sdk-mcmgr_m7>)
