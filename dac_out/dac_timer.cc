#include "libs/base/analog.h"
#include "libs/base/check.h"
// #include "libs/base/timer.h"
#include "third_party/modified/nxp/rt1176-sdk/fsl_tickless_gpt.h"
#include "third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_gpt.h"
#include "third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_snvs_hp.h"
#include "third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_snvs_lp.h"

#include "dac_timer.h"

namespace coralmicro {

volatile uint16_t *chirpform;
volatile uint32_t nStep = 0;
volatile uint32_t Dir = 1;
volatile uint32_t nSamp = 0;



void DacTimerInit() {
    gpt_config_t gpt_config;
    GPT_GetDefaultConfig(&gpt_config);
    gpt_config.clockSource = kGPT_ClockSource_Periph;

    auto gpt6_root_freq = CLOCK_GetRootClockFreq(kCLOCK_Root_Gpt6);

    GPT_Init(GPT6, &gpt_config);
    GPT_EnableInterrupts(GPT6, kGPT_OutputCompare1InterruptEnable);
    GPT_ClearStatusFlags(GPT6, kGPT_OutputCompare1Flag);
    // Set the divider to get us close to 200KHz.
    GPT_SetClockDivider(GPT6, gpt6_root_freq / SAMLERATE);
    GPT_SetOutputCompareValue(GPT6, kGPT_OutputCompare_Channel1, 0);
    EnableIRQ(GPT6_IRQn);
    GPT_StartTimer(GPT6);

#if (__CORTEX_M == 7)
    snvs_hp_rtc_config_t hp_rtc_config;
    snvs_lp_srtc_config_t lp_rtc_config;
    SNVS_HP_RTC_GetDefaultConfig(&hp_rtc_config);
    SNVS_LP_SRTC_GetDefaultConfig(&lp_rtc_config);
    SNVS_HP_RTC_Init(SNVS, &hp_rtc_config);
    SNVS_LP_SRTC_Init(SNVS, &lp_rtc_config);
#endif

    DacInit();
    DacWrite(0);
    DacEnable(true);
}

extern "C" void GPT6_IRQHandler() {
    if (GPT_GetStatusFlags(GPT6, kGPT_OutputCompare1Flag)) {
        GPT_ClearStatusFlags(GPT6, kGPT_OutputCompare1Flag);
        DacWrite(chirpform[nStep]);
        nStep = nStep + Dir;
        if (nStep == nSamp - 1) nStep = 0;
        // if ((nStep == nSamp - 1) or (nStep == 0))  Dir = Dir * -1;
    }
}

}  // namespace coralmicro
