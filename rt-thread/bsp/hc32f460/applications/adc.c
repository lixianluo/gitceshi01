#include <rthw.h>
#include <rtthread.h>
#include "board.h"
#include "adc.h"


static void AdcClockConfig(void);
static void AdcInitConfig(void);
static void AdcChannelConfig(void);
static void AdcTriggerConfig(void);
static void AdcDmaConfig(void);
static void AdcSetChannelPinMode(const M4_ADC_TypeDef* ADCx, uint32_t u32Channel, en_pin_mode_t enMode);
static void AdcSetPinMode(uint8_t u8AdcPin, en_pin_mode_t enMode);

static uint16_t ADC_uiAdcDmaBuffer[(ADC1_SA_DMA_BLOCK_SIZE + ADC2_SA_DMA_BLOCK_SIZE)];
/*RT-thread for ADC*/
static TAdcInfoDef ADC_tInfo;

int ADC_iTaskHandler(void);
static void ADC_vTaskHandler_Entry(void* parameter);
static void ADC_vCalculateADValue(void);
static void ADC_vValueHanler0(uint32_t ulAdValue);
static void ADC_vValueHanler1(uint32_t ulAdValue);
static void ADC_vValueHanler2(uint32_t ulAdValue);
static void ADC_vValueHanler3(uint32_t ulAdValue);



/*1分钟电源电压滤波*/
static uint8_t supply_voltage_flag = 0;
static float supply_voltage_buff = 0;
static uint8_t supply_voltage_count = 0;
static  rt_timer_t supply_voltage_timer;
static void supply_voltage_timeout(void* parameter);


TAdcInfoDef* ADC_ptGetInfo(void);


void Adc_vConfig(void)
{
    AdcClockConfig();
    AdcInitConfig();
    AdcChannelConfig();
    AdcTriggerConfig();
    AdcDmaConfig();
    ADC_StartConvert(M4_ADC1);
    ADC_StartConvert(M4_ADC2);
}
/**
 *******************************************************************************
 ** \brief  ADC clock configuration.
 **
 ** \note   1) ADCLK max frequency is 60MHz.
 **         2) If PCLK2 and PCLK4 are selected as the ADC clock,
 **            the following conditions must be met:
 **            a. ADCLK(PCLK2) max 60MHz;
 **            b. PCLK4 : ADCLK = 1:1, 2:1, 4:1, 8:1, 1:2, 1:4
 **
 ******************************************************************************/
static void AdcClockConfig(void)
{
	CLK_SetPeriClkSource(ClkPeriSrcPclk); // 64MHz
}
/**
 *******************************************************************************
 ** \brief  ADC initial configuration.
 **
 ******************************************************************************/
static void AdcInitConfig(void)
{
    stc_adc_init_t stcAdcInit;

    MEM_ZERO_STRUCT(stcAdcInit);

    stcAdcInit.enResolution = AdcResolution_12Bit;
    stcAdcInit.enDataAlign = AdcDataAlign_Right;
    stcAdcInit.enAutoClear = AdcClren_Enable;
    stcAdcInit.enScanMode = AdcMode_SAContinuous;
    stcAdcInit.enRschsel = AdcRschsel_Restart;

    /* 1. Enable ADC1. */
    PWC_Fcg3PeriphClockCmd(PWC_FCG3_PERIPH_ADC1, Enable);
    /* 2. Initialize ADC1. */
    ADC_Init(M4_ADC1, &stcAdcInit);

    /**ADC 2: Vref*/
    stcAdcInit.enResolution = AdcResolution_12Bit;// AdcResolution_8Bit;
    stcAdcInit.enDataAlign = AdcDataAlign_Right;
    stcAdcInit.enAutoClear = AdcClren_Enable;
    stcAdcInit.enScanMode = AdcMode_SAContinuous;
    /* 1. Enable ADC2. */
    PWC_Fcg3PeriphClockCmd(PWC_FCG3_PERIPH_ADC2, Enable);
    /* 2. Initialize ADC2. */
    ADC_Init(M4_ADC2, &stcAdcInit);
}
/**
 *******************************************************************************
 ** \brief  ADC channel configuration.
 **
 ******************************************************************************/
static void AdcChannelConfig(void)
{
    stc_adc_ch_cfg_t stcChCfg;
    uint8_t au8Adc1SaSampTime[ADC1_SA_CHANNEL_COUNT] = ADC1_SA_CHANNEL_SAMPLE_TIME;
    uint8_t au8Adc2SaSampTime[ADC2_SA_CHANNEL_COUNT] = ADC2_SA_CHANNEL_SAMPLE_TIME;
 
    MEM_ZERO_STRUCT(stcChCfg);

    /* 0. Remap the correspondence between channels and analog input pins. */
    ADC_ChannelRemap(M4_ADC1, ADC1_CH0, ADC1_IN14);
    ADC_ChannelRemap(M4_ADC1, ADC1_CH1, ADC1_IN15);
    
    ADC_ChannelRemap(M4_ADC2, ADC2_CH0, ADC12_IN6);
    ADC_ChannelRemap(M4_ADC2, ADC2_CH1, ADC12_IN9);
    

 
    /**************************** Add ADC1 channels ****************************/
    /* 1. Set the ADC pin to analog mode. */
    AdcSetChannelPinMode(M4_ADC1, ADC1_CHANNEL, Pin_Mode_Ana);
    AdcSetChannelPinMode(M4_ADC2, ADC2_CHANNEL, Pin_Mode_Ana);


    stcChCfg.u32Channel = ADC1_SA_CHANNEL;
    stcChCfg.u8Sequence = ADC_SEQ_A;
    stcChCfg.pu8SampTime = au8Adc1SaSampTime;
    /* 2. Add ADC channel. */
    ADC_AddAdcChannel(M4_ADC1, &stcChCfg);

    /**ADC2*/
    stcChCfg.u32Channel = ADC2_SA_CHANNEL;
    stcChCfg.u8Sequence = ADC_SEQ_A;
    stcChCfg.pu8SampTime = au8Adc2SaSampTime;
    ADC_AddAdcChannel(M4_ADC2, &stcChCfg);

}
/**
 *******************************************************************************
 ** \brief  ADC trigger source configuration.
 **
 ******************************************************************************/
static void AdcTriggerConfig(void)
{
    stc_adc_trg_cfg_t stcTrgCfg;

    MEM_ZERO_STRUCT(stcTrgCfg);

    /*
     * If select an event(@ref en_event_src_t) to trigger ADC,
     * AOS must be enabled first.
     */
    PWC_Fcg0PeriphClockCmd(PWC_FCG0_PERIPH_AOS, Enable);

    /* ADC1 sequence A will be started by software. */
    ADC_TriggerSrcCmd(M4_ADC1, ADC_SEQ_A, Disable);
}
/**
 *******************************************************************************
 ** \brief  DMA configuration for ADC1 and ADC2.
 **
 ******************************************************************************/
static void AdcDmaConfig(void)
{
    stc_dma_config_t stcDmaCfg;

    MEM_ZERO_STRUCT(stcDmaCfg);

    stcDmaCfg.u16BlockSize = ADC1_SA_DMA_BLOCK_SIZE;
    stcDmaCfg.u16TransferCnt = ADC1_SA_DMA_TRANS_COUNT;
    stcDmaCfg.u32SrcAddr = (uint32_t)(&M4_ADC1->DR0);
    stcDmaCfg.u32DesAddr = (uint32_t)(&ADC_uiAdcDmaBuffer[0]);
    stcDmaCfg.u16DesRptSize = ADC1_SA_DMA_BLOCK_SIZE;
    stcDmaCfg.u16SrcRptSize = ADC1_SA_DMA_BLOCK_SIZE;
    stcDmaCfg.u32DmaLlp = 0u;
    stcDmaCfg.stcSrcNseqCfg.u16Cnt = 0u;
    stcDmaCfg.stcSrcNseqCfg.u32Offset = 0u;
    stcDmaCfg.stcDesNseqCfg.u16Cnt = 0u;
    stcDmaCfg.stcDesNseqCfg.u32Offset = 0u;
    stcDmaCfg.stcDmaChCfg.enSrcInc = AddressIncrease;
    stcDmaCfg.stcDmaChCfg.enDesInc = AddressIncrease;
    stcDmaCfg.stcDmaChCfg.enSrcRptEn = Enable;
    stcDmaCfg.stcDmaChCfg.enDesRptEn = Enable;
    stcDmaCfg.stcDmaChCfg.enSrcNseqEn = Disable;
    stcDmaCfg.stcDmaChCfg.enDesNseqEn = Disable;
    stcDmaCfg.stcDmaChCfg.enTrnWidth = Dma16Bit;
    stcDmaCfg.stcDmaChCfg.enLlpEn = Disable;
    stcDmaCfg.stcDmaChCfg.enIntEn = Disable;

    PWC_Fcg0PeriphClockCmd(ADC1_SA_DMA_PWC, Enable);
    DMA_InitChannel(ADC1_SA_DMA_UNIT, ADC1_SA_DMA_CH, &stcDmaCfg);
    DMA_Cmd(ADC1_SA_DMA_UNIT, Enable);
    DMA_ChannelCmd(ADC1_SA_DMA_UNIT, ADC1_SA_DMA_CH, Enable);
    DMA_ClearIrqFlag(ADC1_SA_DMA_UNIT, ADC1_SA_DMA_CH, TrnCpltIrq);

    stcDmaCfg.u16BlockSize = ADC2_SA_DMA_BLOCK_SIZE;
    stcDmaCfg.u16TransferCnt = ADC2_SA_DMA_TRANS_COUNT;
    stcDmaCfg.u32SrcAddr = (uint32_t)(&M4_ADC2->DR0);
    stcDmaCfg.u32DesAddr = (uint32_t)(&ADC_uiAdcDmaBuffer[2]);
    stcDmaCfg.u16DesRptSize = ADC2_SA_DMA_BLOCK_SIZE;
    stcDmaCfg.u16SrcRptSize = ADC2_SA_DMA_BLOCK_SIZE;
    stcDmaCfg.u32DmaLlp = 0u;
    stcDmaCfg.stcSrcNseqCfg.u16Cnt = 0u;
    stcDmaCfg.stcSrcNseqCfg.u32Offset = 0u;
    stcDmaCfg.stcDesNseqCfg.u16Cnt = 0u;
    stcDmaCfg.stcDesNseqCfg.u32Offset = 0u;
    stcDmaCfg.stcDmaChCfg.enSrcInc = AddressIncrease;
    stcDmaCfg.stcDmaChCfg.enDesInc = AddressIncrease;
    stcDmaCfg.stcDmaChCfg.enSrcRptEn = Enable;
    stcDmaCfg.stcDmaChCfg.enDesRptEn = Enable;
    stcDmaCfg.stcDmaChCfg.enSrcNseqEn = Disable;
    stcDmaCfg.stcDmaChCfg.enDesNseqEn = Disable;
    stcDmaCfg.stcDmaChCfg.enTrnWidth = Dma16Bit;
    stcDmaCfg.stcDmaChCfg.enLlpEn = Disable;
    stcDmaCfg.stcDmaChCfg.enIntEn = Disable;

    PWC_Fcg0PeriphClockCmd(ADC1_SA_DMA_PWC, Enable);
    DMA_InitChannel(ADC1_SA_DMA_UNIT, ADC2_SA_DMA_CH, &stcDmaCfg);
    DMA_Cmd(ADC1_SA_DMA_UNIT, Enable);
    DMA_ChannelCmd(ADC1_SA_DMA_UNIT, ADC2_SA_DMA_CH, Enable);
    DMA_ClearIrqFlag(ADC1_SA_DMA_UNIT, ADC2_SA_DMA_CH, TrnCpltIrq);

    /* AOS must be enabled to use DMA */
    /* AOS enabled at first. */
    /* If you have enabled AOS before, then the following statement is not needed. */
    PWC_Fcg0PeriphClockCmd(PWC_FCG0_PERIPH_AOS, Enable);
    DMA_SetTriggerSrc(ADC1_SA_DMA_UNIT, ADC1_SA_DMA_CH, ADC1_SA_DMA_TRGSRC);
    DMA_SetTriggerSrc(ADC1_SA_DMA_UNIT, ADC2_SA_DMA_CH, ADC2_SA_DMA_TRGSRC);
}
/**
 *******************************************************************************
 ** \brief  Config the pin which is mapping the channel to analog or digit mode.
 **
 ******************************************************************************/
static void AdcSetChannelPinMode(const M4_ADC_TypeDef* ADCx, uint32_t u32Channel, en_pin_mode_t enMode)
{
    uint8_t u8ChIndex;
#if (ADC_CH_REMAP)
    uint8_t u8AdcPin;
#else
    uint8_t u8ChOffset = 0u;
#endif

    if (M4_ADC1 == ADCx)
    {
        u32Channel &= ADC1_PIN_MASK_ALL;
    }
    else
    {
        u32Channel &= ADC2_PIN_MASK_ALL;
#if (!ADC_CH_REMAP)
        u8ChOffset = 4u;
#endif
    }

    u8ChIndex = 0u;
    while (0u != u32Channel)
    {
        if (u32Channel & 0x1ul)
        {
#if (ADC_CH_REMAP)
            u8AdcPin = ADC_GetChannelPinNum(ADCx, u8ChIndex);
            AdcSetPinMode(u8AdcPin, enMode);
#else
            AdcSetPinMode((u8ChIndex + u8ChOffset), enMode);
#endif
        }

        u32Channel >>= 1u;
        u8ChIndex++;
    }
}
/**
 *******************************************************************************
 ** \brief  Set an ADC pin as analog input mode or digit mode.
 **
 ******************************************************************************/
static void AdcSetPinMode(uint8_t u8AdcPin, en_pin_mode_t enMode)
{
    en_port_t enPort = PortA;
    en_pin_t enPin = Pin00;
    bool bFlag = true;
    stc_port_init_t stcPortInit;

    MEM_ZERO_STRUCT(stcPortInit);
    stcPortInit.enPinMode = enMode;
    stcPortInit.enPullUp = Disable;

    switch (u8AdcPin)
    {
    case ADC1_IN0:
        enPort = PortA;
        enPin = Pin00;
        break;

    case ADC1_IN1:
        enPort = PortA;
        enPin = Pin01;
        break;

    case ADC1_IN2:
        enPort = PortA;
        enPin = Pin02;
        break;

    case ADC1_IN3:
        enPort = PortA;
        enPin = Pin03;
        break;

    case ADC12_IN4:
        enPort = PortA;
        enPin = Pin04;
        break;

    case ADC12_IN5:
        enPort = PortA;
        enPin = Pin05;
        break;

    case ADC12_IN6:
        enPort = PortA;
        enPin = Pin06;
        break;

    case ADC12_IN7:
        enPort = PortA;
        enPin = Pin07;
        break;

    case ADC12_IN8:
        enPort = PortB;
        enPin = Pin00;
        break;

    case ADC12_IN9:
        enPort = PortB;
        enPin = Pin01;
        break;

    case ADC12_IN10:
        enPort = PortC;
        enPin = Pin00;
        break;

    case ADC12_IN11:
        enPort = PortC;
        enPin = Pin01;
        break;

    case ADC1_IN12:
        enPort = PortC;
        enPin = Pin02;
        break;

    case ADC1_IN13:
        enPort = PortC;
        enPin = Pin03;
        break;

    case ADC1_IN14:
        enPort = PortC;
        enPin = Pin04;
        break;

    case ADC1_IN15:
        enPort = PortC;
        enPin = Pin05;
        break;

    default:
        bFlag = false;
        break;
    }

    if (true == bFlag)
    {
        PORT_Init(enPort, enPin, &stcPortInit);
    }
}

/*电流公式计算 AD = (0.13453 + 电流 *内阻 * 12/12.51)*5.31*1241.21 */
static void ADC_vValueHanler0(uint32_t ulAdValue)
{
	float fAdcurrent = 0;
	fAdcurrent = ((float)(ulAdValue - 886.663701f)) / (6327.192096f * 0.0028f);   //886.663701和都是硬件上面计算得来 0.0028是内阻
	ADC_tInfo.fConvertValue[ADC_IDX_0] = fAdcurrent;
}
static void ADC_vValueHanler1(uint32_t ulAdValue)
{
    float fAdcurrent = 0;
    fAdcurrent = ((float)(ulAdValue - 886.663701f)) / (6327.192096f * 0.0028f);   //886.663701和都是硬件上面计算得来 0.0028是内阻
    ADC_tInfo.fConvertValue[ADC_IDX_1] = fAdcurrent;
}
static void ADC_vValueHanler2(uint32_t ulAdValue)
{
    supply_voltage_flag = 1;
    float fAdvlaue = 0;
    fAdvlaue = (float)(ulAdValue) / 1241 * 11;
    ADC_tInfo.fConvertValue[ADC_IDX_2] = fAdvlaue;
}
static void ADC_vValueHanler3(uint32_t ulAdValue)
{
    //硬件没接，预留
}





int ADC_iTaskHandler(void)
{
    rt_thread_t tid;
    /* 创建ADC_vTaskHandler_entry线程 */
    tid = rt_thread_create("ADC_vTaskHandler_entry",
                            ADC_vTaskHandler_Entry,
                            RT_NULL,
                            1024,
                            3,
                            20);
    /* 创建成功则启动线程 */
    if (tid != RT_NULL)rt_thread_startup(tid);


    /*1s软件周期定时器*/
    supply_voltage_timer = rt_timer_create("supply_voltage_timer", supply_voltage_timeout,
        RT_NULL, 100,
        RT_TIMER_FLAG_PERIODIC | RT_TIMER_FLAG_SOFT_TIMER);
    if (supply_voltage_timer != RT_NULL)rt_timer_start(supply_voltage_timer);

	return 0;
}
INIT_APP_EXPORT(ADC_iTaskHandler);

static void ADC_vTaskHandler_Entry(void* parameter)
{
    //rt_thread_mdelay(100);
    while (1)
    {
        if (ADC_tInfo.ucADSampleCount < ADC_SAMPLE_TIMES)
        {
            for (uint8_t i = 0; i < ADC_IDX_NUM; ++i)
            {
                ADC_tInfo.ulADRawValue[i][ADC_tInfo.ucADSampleCount] = ADC_uiAdcDmaBuffer[i];  //!< update data from dma
            }

            ADC_tInfo.ucADSampleCount++;

            if (ADC_tInfo.ucADSampleCount >= ADC_SAMPLE_TIMES)
            {
                ADC_tInfo.ucADSampleCount = 0;
                ADC_vCalculateADValue();
            }
        }
        rt_thread_mdelay(20);
    }
}
static void ADC_vCalculateADValue(void)
{
    uint32_t ulMax = 0;
    uint32_t ulMin = 0;
    uint32_t ulSum = 0;
    uint8_t i = 0;
    uint8_t j = 0;


    for (i = 0; i < ADC_IDX_NUM; ++i)
    {
        ulMax = ADC_tInfo.ulADRawValue[i][0];	//!< set init value
        ulMin = ADC_tInfo.ulADRawValue[i][0];
        ulSum = 0;

        for (j = 0; j < ADC_SAMPLE_TIMES; j++)
        {
            ulMax = (ulMax >= ADC_tInfo.ulADRawValue[i][j]) ? ulMax : ADC_tInfo.ulADRawValue[i][j];
            ulMin = (ulMin <= ADC_tInfo.ulADRawValue[i][j]) ? ulMin : ADC_tInfo.ulADRawValue[i][j];
            ulSum += ADC_tInfo.ulADRawValue[i][j];
        }

        ADC_tInfo.ulADValue[i] = (ulSum - ulMax - ulMin) / (ADC_SAMPLE_TIMES - 2);
    }
    ADC_vValueHanler0(ADC_tInfo.ulADValue[ADC_IDX_0]);
    ADC_vValueHanler1(ADC_tInfo.ulADValue[ADC_IDX_1]);
    ADC_vValueHanler2(ADC_tInfo.ulADValue[ADC_IDX_2]);
    ADC_vValueHanler3(ADC_tInfo.ulADValue[ADC_IDX_3]);


}

static void supply_voltage_timeout(void* parameter)
{
    
    if (supply_voltage_flag == 1)
    {
        ADC_tInfo.supply_voltage = (ADC_tInfo.supply_voltage < 10) ? ADC_tInfo.fConvertValue[ADC_IDX_2] : ADC_tInfo.supply_voltage;//第一次写入和关机时候AD为2.5v

        supply_voltage_buff += ADC_tInfo.fConvertValue[ADC_IDX_2];
        supply_voltage_count++;
        if (supply_voltage_count == 60)    //1s进入一次，当到60次时，就是1分钟
        {
            ADC_tInfo.supply_voltage = supply_voltage_buff / supply_voltage_count;
            supply_voltage_buff = 0;
            supply_voltage_count = 0;
        }
    }

}
TAdcInfoDef* ADC_ptGetInfo(void)
{
    return &ADC_tInfo;
}
