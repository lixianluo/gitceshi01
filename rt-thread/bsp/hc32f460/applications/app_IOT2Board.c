#include "app_IOT2Board.h"
#include "usart.h"
#include "board.h"
#include "app_key.h"
#include "app_display.h"
#include "app_error.h"
#include "main.h"


/*˽�б���-----------------------------------------------------------------------*/
static volatile uint32_t IOT_ulMsgToTransmitBits = 0;		//���32����Ϣ�ܱ�����


static uint32_t Number_of_Transmit = 0;	//���ڷ��͵Ĵ���(ÿ�η���+1)

static TDmaBuffDef IOT_ucTxBuffer;	//���͵�DMA���ݻ���
static TDmaBuffDef IOT_ucRxBuffer;	//���յ�DMA���ݻ���


static volatile TIOTRxMessageObjDef IOT_tRxMsgQueue[IOT_RX_QUEUE];//IOT���ն���8��(0-7ѭ������)

static volatile uint8_t IOT_ucRxMsgHeadIndex = 0;    //��һ���µĽ��հ�������
static volatile uint8_t IOT_ucRxMsgTailIndex = 0;    //�����еĽ��հ�������

static  rt_sem_t usart3_rx_sem = RT_NULL;			//�����ź���
static  rt_sem_t usart3_tx_sem = RT_NULL;			//�����ź���
/*-------------------------------------------------------------------------------*/

/*˽�к���-----------------------------------------------------------------------*/

static void IOT_vTxInfo(TIOTMsgObjDef* tObject);		//IOT���ͻص�����
static void IOT_vRxInfo(TIOTMsgObjDef* tObject);		//����

static uint8_t IOT_ucGetTransmitMessageIndex(void);								 //�õ������б����
static uint8_t IOT_ucGetReceptionMessageIndex(uint8_t ucMsgUID);				 //�õ������б����
static uint16_t usChecksumCalculate(const uint8_t* pucBuffer, uint16_t usLength);//�������У��
static void IOT_vCheckTransmitTask(void);										 //��鷢������
static void IOT_vCheckReceptionTask(TIOTRxMessageObjDef* ptRxMessage);			 //����������
static void IOT_vTransmitHandler_entry(void* parameter);						 //���������߳�
static void IOT_vReceptionHandler_entry(void* parameter);						 //���������߳�
/*-------------------------------------------------------------------------------*/



/*������Ϣ�б�-------------------------------------------------------------------*/
static const TIOTMsgTbDef IOT_tTransmitTb[] = {
	/*msgid*/			/*call back function*/
	{IOT_INFO_REPORT,				IOT_vTxInfo},

};
/*-------------------------------------------------------------------------------*/

/*������Ϣ�б�-------------------------------------------------------------------*/
static const TIOTMsgTbDef IOT_tReceiveTb[] = {
	/*msgid*/			/*call back function*/
	{0x01,				IOT_vRxInfo},

};
/*-------------------------------------------------------------------------------*/



int IOT_iTaskHandler(void)
{
	rt_thread_t tid1,tid2;
	/* ����IOT_vTransmitHandler_entry�߳� */
	tid1 = rt_thread_create("IOT_vTransmitHandler_entry",
		IOT_vTransmitHandler_entry,
		RT_NULL,
		1024,
		7,
		20);
	/* ����IOT_vTaskHandler_entry�߳� */
	tid2 = rt_thread_create("IOT_vReceptionHandler_entry",
		IOT_vReceptionHandler_entry,
		RT_NULL,
		1024,
		7,
		20);
	
	/* �����ɹ��������߳� */
	if (tid1 != RT_NULL)rt_thread_startup(tid1);
	/* �����ɹ��������߳� */
	if (tid2 != RT_NULL)rt_thread_startup(tid2);
	return 0;
}
INIT_APP_EXPORT(IOT_iTaskHandler);

void Usart3RxIrqCallback(void)
{
	/*�رն�ʱ���������־λ���ر�DMAͨ��*/
	TIMER0_Cmd(M4_TMR02, Tim0_ChannelA, Disable);
	USART_ClearStatus(USART_3, UsartRxTimeOut);
	DMA_ChannelCmd(DMA_UNIT_UART3_RX, DMA_CH_UART3_RX, Disable);

	/**��ȡDMA�������*/
	uint8_t ucRxByteCount = UART_DMA_BUFF_SIZE - UART_vGetDMACount(&uart3_handler);//GetDMACount�ǵ�ǰ����ʣ����� ÿ����һ�ξ�-1 ���Խ��յĸ���=����-ʣ��ֵ  
	/*���������Ϣ��������*/
	(void)memset((void*)IOT_tRxMsgQueue[IOT_ucRxMsgHeadIndex].ucRxData, 0, UART_DMA_BUFF_SIZE);
	/*�����յ�DMA���ݻ�����������Ϣ������*/
	(void)memcpy((void*)IOT_tRxMsgQueue[IOT_ucRxMsgHeadIndex].ucRxData, (const void*)IOT_ucRxBuffer.ucData, ucRxByteCount);
	/*�����յ��ĸ������������Ϣ������*/
	IOT_tRxMsgQueue[IOT_ucRxMsgHeadIndex].ulRxByteCount = ucRxByteCount;
	/*�����еĽ��հ�����+1 (0-7ѭ��)*/
	IOT_ucRxMsgHeadIndex = (IOT_ucRxMsgHeadIndex + 1) % IOT_RX_QUEUE;
	/*���DMA���ݻ���*/
	(void)memset((void*)IOT_ucRxBuffer.ucData, 0x00, UART_DMA_BUFF_SIZE);
	/*�򿪽�����һ��DMA����*/
	UART_vReceiveDMA(&uart3_handler, &IOT_ucRxBuffer, UART_DMA_BUFF_SIZE);	//��������DMA���ܴ����������Ϊ   UART_DMA_BUFF_SIZE
	/*�ͷ�һ�����ڽ����ź���*/
	rt_sem_release(usart3_rx_sem);
}
void IOT_vTransmitMessage(uint8_t ucMessageIndex)
{
	if (ucMessageIndex < IOT_MSG_ID_NUM)
	{
		DEF_SET(IOT_ulMsgToTransmitBits, 1 << ucMessageIndex);
		/*�ͷ�һ�����ڽ����ź���*/
		rt_sem_release(usart3_tx_sem);
	}
}

static void IOT_vTxInfo(TIOTMsgObjDef* tObject)
{
	/*��Ч���ȡ��豸��š�֡��š���Ч�غ�����*/
	tObject->len = 10;
	tObject->deviceid = 2;
	tObject->seq= Number_of_Transmit;
	
	tObject->payload[0] = Key_ptGetInfo()->Work_Status;
	tObject->payload[1] = (Main_ptGetInfo()->IOT_systick_min & 0xFF);
	tObject->payload[2] = (Main_ptGetInfo()->IOT_systick_min >> 8 & 0xFF);
	tObject->payload[3] = (Main_ptGetInfo()->IOT_systick_min >> 16 & 0xFF);
	tObject->payload[4] = (Main_ptGetInfo()->IOT_systick_min >> 24 & 0xFF);
	tObject->payload[5] = Display_ptGetInfo()->batter_info;
	tObject->payload[6] = Error_ptGetInfo() & 0xFF;
	tObject->payload[7] = Error_ptGetInfo() >> 8 & 0xFF;
	tObject->payload[8] = Error_ptGetInfo() >> 16 & 0xFF;
	tObject->payload[9] = Error_ptGetInfo() >> 24 & 0xFF;
	Number_of_Transmit++;
}
static void IOT_vRxInfo(TIOTMsgObjDef* tObject)
{
	Display_ptGetInfo()->wifi_switch = tObject->payload[0];
}

static uint8_t IOT_ucGetTransmitMessageIndex(void)
{
	uint8_t i = 0;

	for (i = 0; i < 32; i++)
	{
		if (DEF_TEST(IOT_ulMsgToTransmitBits, 1 << i))
		{
			return i;
		}
	}

	return 0xFF;	//!< û�з���
}

static uint8_t IOT_ucGetReceptionMessageIndex(uint8_t msgid)
{
	uint8_t i = 0;
	uint8_t ucSize = sizeof(IOT_tReceiveTb) / sizeof(IOT_tReceiveTb[0]);

	for (i = 0; i < ucSize; i++)
	{
		if (msgid == IOT_tReceiveTb[i].msgid)
		{
			return i;
		}
	}

	return 0xFF;	//!< not founds
}


static uint16_t usChecksumCalculate(const uint8_t* pucBuffer, uint16_t usLength)
{
	uint16_t usChecksum = 0;

	for (uint16_t i = 0; i < usLength; i++)
	{
		usChecksum += pucBuffer[i];
	}

	return usChecksum;
}


static void IOT_vCheckTransmitTask(void)
{
	uint8_t ucMessageIndex = 0;
	
	TIOTMsgObjDef IOT_tMessageToSend;
	/*
	�ж��Ƿ���Ȼ���ڷ���״̬���Ǿ�return-------
	code
	code
	code
	---------------------------------------------
	*/

	/*��鷢��λ�Ƿ���λ*/
	if (IOT_ulMsgToTransmitBits)
	{
		/*�õ����������ֵ*/
		ucMessageIndex = IOT_ucGetTransmitMessageIndex();
		if (ucMessageIndex < sizeof(IOT_tTransmitTb) / sizeof(IOT_tTransmitTb[0]))
		{
			/*������ͱ�־λ*/  
			DEF_RES(IOT_ulMsgToTransmitBits, 1 << ucMessageIndex);

			IOT_tMessageToSend.head = IOT_BUFFER_HEAD;			//֡ͷ
			IOT_tMessageToSend.tail = IOT_BUFFER_TAIL;			//֡β
			IOT_tMessageToSend.len = 0;				//���������жϲ�����0�����
			IOT_tMessageToSend.msgid = IOT_tTransmitTb[ucMessageIndex].msgid;		//���ı��
			IOT_tTransmitTb[ucMessageIndex].pvServiceFunc(&IOT_tMessageToSend);		
			IOT_tMessageToSend.checksum = usChecksumCalculate(&IOT_tMessageToSend.head, IOT_tMessageToSend.len+5);//�����
			/*�Ȱ�֡ͷ+���ı��+��Ч�غɳ���+�豸���+֡���*/
			(void)memcpy((void*)IOT_ucTxBuffer.ucData, (const void*)&IOT_tMessageToSend, 5);
			if (0 != IOT_tMessageToSend.len)
			{	/*��Ч�غ�����*/
				(void)memcpy((void*)(IOT_ucTxBuffer.ucData + 5), (const void*)&(IOT_tMessageToSend.payload), IOT_tMessageToSend.len);
			}
			/*У���(2byte)+֡β*/
			(void)memcpy((void*)(IOT_ucTxBuffer.ucData+5+IOT_tMessageToSend.len), (const void*)&(IOT_tMessageToSend.checksum), 3);
		
			IOT_ucTxBuffer.uiDataLen = IOT_tMessageToSend.len + 8;			
			UART_vTransmitDMA(&uart3_handler, &IOT_ucTxBuffer);
		}
	}
}
static void IOT_vCheckReceptionTask(TIOTRxMessageObjDef* ptRxMessage)
{
	TIOTMsgObjDef tMessageToReceive;
	uint16_t ucCheckSum = 0;
	uint8_t ucMessageIndex = 0;

	if (ptRxMessage->ulRxByteCount < IOT_RX_BUFFER_MIN_SIZE)//��С����=֡ͷ+���ı��+���ݳ���+�豸���+֡���+����+У���(2)+֡β
	{
		return;
	}
	/*1.�ж�֡ͷ+֡β*/
	if ((ptRxMessage->ucRxData[0] == IOT_BUFFER_HEAD)&&(ptRxMessage->ucRxData[ptRxMessage->ulRxByteCount-1]== IOT_BUFFER_TAIL))
	{
		/*2.�ж�*���ճ���>=���ݳ���+����8�� &&  ���ݳ���<DMA�����ճ���-��С����*/
		if ((ptRxMessage->ulRxByteCount >= (ptRxMessage->ucRxData[2] + IOT_RX_BUFFER_MIN_SIZE)) && (ptRxMessage->ucRxData[2] <= (IOT_RX_BUFFER_SIZE - IOT_RX_BUFFER_MIN_SIZE)))
		{
			/*3.���Checksum��ֵ*/
			ucCheckSum = usChecksumCalculate(ptRxMessage->ucRxData, ptRxMessage->ucRxData[2] + 5);
			/*4.���Checksum��ֵ*/
			if (ucCheckSum == ((ptRxMessage->ucRxData[ptRxMessage->ucRxData[2] + 5]) << 8 | (ptRxMessage->ucRxData[ptRxMessage->ucRxData[2] + 6])))
			{
				/*5.ȡ�����ݵ���ʱ����*/
				tMessageToReceive.head = ptRxMessage->ucRxData[0];
				tMessageToReceive.msgid = ptRxMessage->ucRxData[1];
				tMessageToReceive.len = ptRxMessage->ucRxData[2];
				tMessageToReceive.deviceid = ptRxMessage->ucRxData[3];
				tMessageToReceive.seq = ptRxMessage->ucRxData[4];
				memcpy((void*)&tMessageToReceive.payload, (const void*)(ptRxMessage->ucRxData+5), ptRxMessage->ucRxData[2]);
				tMessageToReceive.checksum = ucCheckSum;
				tMessageToReceive.tail = ptRxMessage->ucRxData[ptRxMessage->ulRxByteCount - 1];
				/*6.ȡ���������к�ID*/
				ucMessageIndex = IOT_ucGetReceptionMessageIndex(tMessageToReceive.msgid);
				if (ucMessageIndex == 0xFF)
				{
					return;	//û���ҵ�
				}
				/*7.�ҵ����к�ID��ִ�лص�����*/
				IOT_tReceiveTb[ucMessageIndex].pvServiceFunc(&tMessageToReceive);
			}
			
		}
	}
} 


static void IOT_vTransmitHandler_entry(void* parameter)
{
	/*����һ�������ź���*/
	usart3_tx_sem = rt_sem_create("usart3_tx_sem", 0, RT_IPC_FLAG_PRIO);
	if (usart3_tx_sem == RT_NULL)
	{
		rt_kprintf("create usart3_tx_sem failed.\n");

	}
	else
	{
		rt_kprintf("create done.usart3_tx_sem value = 0.\n");
	}
	while (1)
	{
		if (rt_sem_take(usart3_tx_sem, 10) == RT_EOK)
		{
			IOT_vCheckTransmitTask();
		}
	}
	
}
static void IOT_vReceptionHandler_entry(void* parameter)
{
	/*����һ�������ź���*/
	usart3_rx_sem = rt_sem_create("usart3_rx_sem", 0, RT_IPC_FLAG_PRIO);
	if (usart3_rx_sem == RT_NULL)
	{
		rt_kprintf("create usart3_rx_sem failed.\n");

	}
	else
	{
		rt_kprintf("create done.usart3_rx_sem value = 0.\n");
	}
	Usart3RxIrqCallback(); //�ϵ�ִ��һ�£���Ϊ��һ�ν�������Ϊ0
	while (1)
	{
		
		if (rt_sem_take(usart3_rx_sem, 10) == RT_EOK)
		{
			if (IOT_ucRxMsgTailIndex != IOT_ucRxMsgHeadIndex)
			{
				IOT_vCheckReceptionTask((TIOTRxMessageObjDef*)&IOT_tRxMsgQueue[IOT_ucRxMsgTailIndex]);

				IOT_ucRxMsgTailIndex = (IOT_ucRxMsgTailIndex + 1) % IOT_RX_QUEUE;
			}
		}
	}
}
