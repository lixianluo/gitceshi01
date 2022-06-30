#ifndef _ADC_H_
#define _ADC_H_


/*
 * Specifies the ADC channels according to the application.
 * NOTE!!! Sequence A and sequence B CANNOT contain the same channel.
 */
#define	ADC_CH_REMAP				1

#define ADC1_REMAP_CH               (ADC1_CH0 | ADC1_CH1)
#define ADC2_REMAP_CH               (ADC2_CH0 | ADC2_CH1 )

#define ADC2_SA_CHANNEL             (ADC2_REMAP_CH)
#define ADC1_SA_CHANNEL             (ADC1_REMAP_CH)
#define ADC1_SA_CHANNEL_COUNT       (2u)
#define ADC2_SA_CHANNEL_COUNT       (2u)

#define ADC1_CHANNEL                (ADC1_SA_CHANNEL)//| ADC1_SB_CHANNEL)
#define ADC2_CHANNEL                (ADC2_SA_CHANNEL)//| ADC2_SB_CHANNEL)
 /* ADC1 channel sampling time.      ADC1_CH0  ADC1_CH1 */
#define ADC1_SA_CHANNEL_SAMPLE_TIME {60,60}
#define ADC2_SA_CHANNEL_SAMPLE_TIME {60,60}


/* DMA definition for ADC1. */
#define ADC1_SA_DMA_UNIT            (M4_DMA2)
#define ADC1_SA_DMA_PWC             (PWC_FCG0_PERIPH_DMA2)
#define ADC1_SA_DMA_CH              (DmaCh0)
#define ADC1_SA_DMA_TRGSRC          (EVT_ADC1_EOCA)
#define ADC1_SA_DMA_BLOCK_SIZE		(2u)
#define ADC1_SA_DMA_TRANS_COUNT		(0u)



/* DMA definition for ADC2. */


#define ADC2_SA_DMA_CH              (DmaCh1)
#define ADC2_SA_DMA_TRGSRC          (EVT_ADC2_EOCA)
#define ADC2_SA_DMA_BLOCK_SIZE		(2u)
#define ADC2_SA_DMA_TRANS_COUNT		(0u)


/*ADC_vTaskHanlder struct*/
#define ADC_SAMPLE_TIMES			6


enum {
	ADC_IDX_0 = 0,		//吸风电机电流
	ADC_IDX_1,			//盘刷电机电流
	ADC_IDX_2,			//电源电压
	ADC_IDX_3,			//温度
	ADC_IDX_NUM
};

typedef struct {
	rt_uint32_t ulADRawValue[ADC_IDX_NUM][ADC_SAMPLE_TIMES];
	rt_uint32_t ulADValue[ADC_IDX_NUM];
	float fConvertValue[ADC_IDX_NUM];
	rt_uint32_t ulVrefintAD;
	rt_uint8_t ucAdIndex;
	rt_uint8_t ucADSampleCount;
	float supply_voltage;
}TAdcInfoDef;



extern void Adc_vConfig(void);

extern TAdcInfoDef* ADC_ptGetInfo(void);

#endif // ! _ADC_H_
