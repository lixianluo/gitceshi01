#include "app_OTA.h"
#include "usart.h"
#include "app_IOT2Board.h"
#include "app_task.h"
#include "flash.h"
#include "app_display.h"

/*˽�б���-----------------------------------------------------------------------*/
static volatile uint32_t OTA_ulMsgToTransmitBits = 0;		//���32����Ϣ�ܱ�����

static uint8_t handshakes = 0;		//���ִ���

static TDmaBuffDef OTA_ucTxBuffer;	//���͵�DMA���ݻ���
static TDmaBuffDef OTA_ucRxBuffer;	//���յ�DMA���ݻ���


static volatile TOTARxMessageObjDef OTA_tRxMsgQueue[OTA_RX_QUEUE];//OTA���ն���8��(0-7ѭ������)

static volatile uint8_t OTA_ucRxMsgHeadIndex = 0;    //��һ���µĽ��հ�������
static volatile uint8_t OTA_ucRxMsgTailIndex = 0;    //�����еĽ��հ�������

static  rt_sem_t usart2_rx_sem = RT_NULL;			//�����ź���
static  rt_sem_t usart2_tx_sem = RT_NULL;			//�����ź���
/*-------------------------------------------------------------------------------*/

/*˽�к���-----------------------------------------------------------------------*/
static void OTA_vTxInfo(TOTAMsgObjDef* tObject);		//OTA���ͻص�����
static void OTA_vRxInfo(TOTAMsgObjDef* tObject);		//OTA���ջص�����



static uint8_t OTA_ucGetTransmitMessageIndex(void);								 //�õ������б����
static uint8_t OTA_ucGetReceptionMessageIndex(uint8_t ucMsgUID);				 //�õ������б����
static void OTA_vCheckTransmitTask(void);										 //��鷢������
static uint8_t OTA_ucGetCheckSumResult(uint8_t* ptr, uint8_t len);				 //�õ�У��͵Ľ��
static void OTA_vCheckReceptionTask(TOTARxMessageObjDef* ptRxMessage);			 //����������
static void OTA_vTransmitHandler_entry(void* parameter);						 //���������߳�
static void OTA_vReceptionHandler_entry(void* parameter);						 //���������߳�
/*-------------------------------------------------------------------------------*/


void Usart2RxIrqCallback(void)
{
	/*�رն�ʱ���������־λ���ر�DMAͨ��*/
	TIMER0_Cmd(M4_TMR01, Tim0_ChannelB, Disable);
	USART_ClearStatus(USART_2, UsartRxTimeOut);
	DMA_ChannelCmd(DMA_UNIT_UART2_RX, DMA_CH_UART2_RX, Disable);

	/**��ȡDMA�������*/
	uint8_t ucRxByteCount = UART_DMA_BUFF_SIZE - UART_vGetDMACount(&uart2_handler);//GetDMACount�ǵ�ǰ����ʣ����� ÿ����һ�ξ�-1 ���Խ��յĸ���=����-ʣ��ֵ  
	/*���������Ϣ��������*/
	(void)memset((void*)OTA_tRxMsgQueue[OTA_ucRxMsgHeadIndex].ucRxData, 0, UART_DMA_BUFF_SIZE);
	/*�����յ�DMA���ݻ�����������Ϣ������*/
	(void)memcpy((void*)OTA_tRxMsgQueue[OTA_ucRxMsgHeadIndex].ucRxData, (const void*)OTA_ucRxBuffer.ucData, ucRxByteCount);
	/*�����յ��ĸ������������Ϣ������*/
	OTA_tRxMsgQueue[OTA_ucRxMsgHeadIndex].ulRxByteCount = ucRxByteCount;
	/*�����еĽ��հ�����+1 (0-7ѭ��)*/
	OTA_ucRxMsgHeadIndex = (OTA_ucRxMsgHeadIndex + 1) % OTA_RX_QUEUE;
	/*���DMA���ݻ���*/
	(void)memset((void*)OTA_ucRxBuffer.ucData, 0x00, UART_DMA_BUFF_SIZE);
	/*�򿪽�����һ��DMA����*/
	UART_vReceiveDMA(&uart2_handler, &OTA_ucRxBuffer, UART_DMA_BUFF_SIZE);	//��������DMA���ܴ����������Ϊ   UART_DMA_BUFF_SIZE
	/*�ͷ�һ�����ڽ����ź���*/
	rt_sem_release(usart2_rx_sem);
}

/*������Ϣ�б�--------------------------------------------------------*/
static const TOTAMsgTbDef OTA_tTransmitTb[] = {
	/*msgid*/						/*call back function*/
	{OTA_MESSAGE_HAND,				OTA_vTxInfo},
};
/*---------------------------------------------------------------------*/


/*������Ϣ�б�-------------------------------------------------------------------*/
static const TOTAMsgTbDef OTA_tReceiveTb[] = {
	/*msgid*/					/*call back function*/
	{OTA_MESSAGE_HAND,				OTA_vRxInfo},

};
/*-------------------------------------------------------------------------------*/

static void OTA_vTxInfo(TOTAMsgObjDef* tObject)
{
	tObject->ucPayload[tObject->ucPayloadLen++] = OTA_MESSAGE_HAND;
	tObject->ucPayload[tObject->ucPayloadLen++] = 0x01;

}

static void OTA_vRxInfo(TOTAMsgObjDef* tObject)
{
	switch (handshakes)
	{
		case First_Handshake:
		{
			if (tObject->ucPayload[1] == 0x00)
			{
				OTA_vTransmitMessage(OTA_MSG_MESSAGE_HAND);
				handshakes = Second_Handshake;
			}
		break;
		}
		case Second_Handshake:
		{
			if (tObject->ucPayload[1] == 0x02)
			{
				Flash_ptGetInfo()->tTaskState = FLASH_TASK_SAVE;
				Display_ptGetInfo()->tTaskState = DISPLAY_TASK_OTA;
				App_Task(SYS_TASK_OTA);
				
			}
			else
			{
				handshakes = Second_Handshake;
			}
			
		break;
		}
		default:
		{
			handshakes = First_Handshake;
			break;
		}
	}
}

static uint8_t OTA_ucGetTransmitMessageIndex(void)
{
	uint8_t i = 0;

	for (i = 0; i < 32; i++)
	{
		if (DEF_TEST(OTA_ulMsgToTransmitBits, 1 << i))
		{
			return i;
		}
	}

	return 0xFF;	//!< û�з��� (ע�������б��ô��������ֵ)
}

static uint8_t OTA_ucGetCheckSumResult(uint8_t* ptr, uint8_t len)
{
	uint8_t ucByte = 0;

	ucByte = *ptr++;
	len--;

	while (len)
	{
		ucByte ^= *ptr++;
		len--;
	}

	return ucByte;
}
static uint8_t OTA_ucGetReceptionMessageIndex(uint8_t msgid)
{
	uint8_t i = 0;
	uint8_t ucSize = sizeof(OTA_tReceiveTb) / sizeof(OTA_tReceiveTb[0]);

	for (i = 0; i < ucSize; i++)
	{
		if (msgid == OTA_tReceiveTb[i].msgid)
		{
			return i;
		}
	}

	return 0xFF;	//!< not founds
}


static void OTA_vCheckTransmitTask(void)
{
	uint8_t ucMessageIndex = 0;
	uint8_t ucCheckSum = 0;
	TOTAMsgObjDef OTA_tMessageToSend;
	/*
	�ж��Ƿ���Ȼ���ڷ���״̬���Ǿ�return-------
	code
	code
	code
	---------------------------------------------
	*/

	/*��鷢��λ�Ƿ���λ*/
	if (OTA_ulMsgToTransmitBits)
	{
		/*�õ����������ֵ*/
		ucMessageIndex = OTA_ucGetTransmitMessageIndex();
		if (ucMessageIndex < sizeof(OTA_tTransmitTb) / sizeof(OTA_tTransmitTb[0]))
		{
			/*������ͱ�־λ*/
			DEF_RES(OTA_ulMsgToTransmitBits, 1 << ucMessageIndex);
			OTA_tMessageToSend.ucHeader0 = OTA_MSG_HEADER0;
			OTA_tMessageToSend.ucHeader1 = OTA_MSG_HEADER1;
			OTA_tMessageToSend.ucPayloadLen = 0;
			OTA_tTransmitTb[ucMessageIndex].pvServiceFunc(&OTA_tMessageToSend);

			ucCheckSum = OTA_ucGetCheckSumResult(&OTA_tMessageToSend.ucPayloadLen, OTA_tMessageToSend.ucPayloadLen + 1);//��Ч�غ�����+��Ч�غ����ݳ��� ����Len+1

			if (OTA_tMessageToSend.ucPayloadLen < OTA_TX_BUFFER_SIZE - 4)
			{
				(void)memcpy((void*)OTA_ucTxBuffer.ucData, (const void*)&OTA_tMessageToSend, OTA_tMessageToSend.ucPayloadLen + 3);	//!< including Header0, Header1, Payload length
				OTA_ucTxBuffer.ucData[OTA_tMessageToSend.ucPayloadLen + 3] = ucCheckSum;		// ����ucPayloadLen+3�ĳ��� �൱�ڸ���ucData[0-PayloadLen+2]������ ��������+3��checkSumλ
			}
			OTA_ucTxBuffer.uiDataLen = OTA_tMessageToSend.ucPayloadLen + 4;
			UART_vTransmitDMA(&uart2_handler, &OTA_ucTxBuffer);
		}
	}
}



static void OTA_vCheckReceptionTask(TOTARxMessageObjDef* ptRxMessage)
{
	TOTAMsgObjDef OTA_tMessageToReceive;
	uint8_t ucCheckSum = 0;
	uint8_t ucMessageIndex = 0;

	if (ptRxMessage->ulRxByteCount < 3)	//!< Header0, Header1, Payload length
	{
		/**no enough data received*/
		return;
	}

	if ((ptRxMessage->ucRxData[0] == OTA_MSG_HEADER0) && (ptRxMessage->ucRxData[1] == OTA_MSG_HEADER1))
	{
		/**received bytes must be more than message data length + 4 (including Header0, Header1, Payload length, CheckSum)*/
		if ((ptRxMessage->ulRxByteCount >= (ptRxMessage->ucRxData[2] + 4)) && (ptRxMessage->ucRxData[2] <= OTA_RX_BUFFER_SIZE))
		{
			(void)memcpy((void*)&OTA_tMessageToReceive, (const void*)ptRxMessage->ucRxData, ptRxMessage->ucRxData[2] + 4);

			ucCheckSum = OTA_ucGetCheckSumResult(&OTA_tMessageToReceive.ucPayloadLen, OTA_tMessageToReceive.ucPayloadLen + 1);

			if (ucCheckSum == OTA_tMessageToReceive.ucPayload[OTA_tMessageToReceive.ucPayloadLen])
			{
				/***/
				ucMessageIndex = OTA_ucGetReceptionMessageIndex(OTA_tMessageToReceive.ucPayload[0]);

				if (0xFF == ucMessageIndex)
				{
					return;	//!< not found
				}
				OTA_tReceiveTb[ucMessageIndex].pvServiceFunc(&OTA_tMessageToReceive);
			}
		}
	}
}

void OTA_vTransmitMessage(uint8_t ucMessageIndex)
{
	if (ucMessageIndex < OTA_MSG_ID_NUM)
	{
		DEF_SET(OTA_ulMsgToTransmitBits, 1 << ucMessageIndex);
		/*�ͷ�һ�����ڽ����ź���*/
		rt_sem_release(usart2_tx_sem);
	}
}





static void OTA_vTransmitHandler_entry(void* parameter)
{
	/*����һ�������ź���*/
	usart2_tx_sem = rt_sem_create("usart2_tx_sem", 0, RT_IPC_FLAG_PRIO);
	if (usart2_tx_sem == RT_NULL)
	{
		rt_kprintf("create usart2_tx_sem failed.\n");

	}
	else
	{
		rt_kprintf("create done.usart2_tx_sem value = 0.\n");
	}
	while (1)
	{
		if (rt_sem_take(usart2_tx_sem, 10) == RT_EOK)
		{
			OTA_vCheckTransmitTask();
		}
	}

}
static void OTA_vReceptionHandler_entry(void* parameter)
{
	/*����һ�������ź���*/
	usart2_rx_sem = rt_sem_create("usart2_rx_sem", 0, RT_IPC_FLAG_PRIO);
	if (usart2_rx_sem == RT_NULL)
	{
		rt_kprintf("create usart2_rx_sem failed.\n");

	}
	else
	{
		rt_kprintf("create done.usart2_rx_sem value = 0.\n");
	}
	Usart2RxIrqCallback(); //�ϵ�ִ��һ�£���Ϊ��һ�ν�������Ϊ0
	while (1)
	{

		if (rt_sem_take(usart2_rx_sem, 10) == RT_EOK)
		{
			if (OTA_ucRxMsgTailIndex != OTA_ucRxMsgHeadIndex)
			{
				OTA_vCheckReceptionTask((TOTARxMessageObjDef*)&OTA_tRxMsgQueue[OTA_ucRxMsgTailIndex]);

				OTA_ucRxMsgTailIndex = (OTA_ucRxMsgTailIndex + 1) % OTA_RX_QUEUE;
			}
		}
	}
}

int OTA_iTaskHandler(void)
{
	rt_thread_t tid1, tid2;
	/* ����OTA_vTransmitHandler_entry�߳� */
	tid1 = rt_thread_create("OTA_vTransmitHandler_entry",
		OTA_vTransmitHandler_entry,
		RT_NULL,
		1024,
		7,
		20);
	/* ����OTA_vTaskHandler_entry�߳� */
	tid2 = rt_thread_create("OTA_vReceptionHandler_entry",
		OTA_vReceptionHandler_entry,
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
INIT_APP_EXPORT(OTA_iTaskHandler);
