#include "app_OTA.h"
#include "usart.h"
#include "app_IOT2Board.h"
#include "app_task.h"
#include "flash.h"
#include "app_display.h"

/*私有变量-----------------------------------------------------------------------*/
static volatile uint32_t OTA_ulMsgToTransmitBits = 0;		//最大32个消息能被发送

static uint8_t handshakes = 0;		//握手次数

static TDmaBuffDef OTA_ucTxBuffer;	//发送的DMA数据缓存
static TDmaBuffDef OTA_ucRxBuffer;	//接收的DMA数据缓存


static volatile TOTARxMessageObjDef OTA_tRxMsgQueue[OTA_RX_QUEUE];//OTA接收队列8个(0-7循环队列)

static volatile uint8_t OTA_ucRxMsgHeadIndex = 0;    //下一次新的接收包的索引
static volatile uint8_t OTA_ucRxMsgTailIndex = 0;    //处理中的接收包的索引

static  rt_sem_t usart2_rx_sem = RT_NULL;			//接收信号量
static  rt_sem_t usart2_tx_sem = RT_NULL;			//发送信号量
/*-------------------------------------------------------------------------------*/

/*私有函数-----------------------------------------------------------------------*/
static void OTA_vTxInfo(TOTAMsgObjDef* tObject);		//OTA发送回调函数
static void OTA_vRxInfo(TOTAMsgObjDef* tObject);		//OTA接收回调函数



static uint8_t OTA_ucGetTransmitMessageIndex(void);								 //得到发送列表序号
static uint8_t OTA_ucGetReceptionMessageIndex(uint8_t ucMsgUID);				 //得到接收列表序号
static void OTA_vCheckTransmitTask(void);										 //检查发送任务
static uint8_t OTA_ucGetCheckSumResult(uint8_t* ptr, uint8_t len);				 //得到校验和的结果
static void OTA_vCheckReceptionTask(TOTARxMessageObjDef* ptRxMessage);			 //检查接收任务
static void OTA_vTransmitHandler_entry(void* parameter);						 //发送任务线程
static void OTA_vReceptionHandler_entry(void* parameter);						 //接收任务线程
/*-------------------------------------------------------------------------------*/


void Usart2RxIrqCallback(void)
{
	/*关闭定时器，清除标志位，关闭DMA通道*/
	TIMER0_Cmd(M4_TMR01, Tim0_ChannelB, Disable);
	USART_ClearStatus(USART_2, UsartRxTimeOut);
	DMA_ChannelCmd(DMA_UNIT_UART2_RX, DMA_CH_UART2_RX, Disable);

	/**获取DMA传输个数*/
	uint8_t ucRxByteCount = UART_DMA_BUFF_SIZE - UART_vGetDMACount(&uart2_handler);//GetDMACount是当前传输剩余次数 每接收一次就-1 所以接收的个数=总数-剩余值  
	/*清除接收消息队列数据*/
	(void)memset((void*)OTA_tRxMsgQueue[OTA_ucRxMsgHeadIndex].ucRxData, 0, UART_DMA_BUFF_SIZE);
	/*将接收的DMA数据缓存放入接收消息队列中*/
	(void)memcpy((void*)OTA_tRxMsgQueue[OTA_ucRxMsgHeadIndex].ucRxData, (const void*)OTA_ucRxBuffer.ucData, ucRxByteCount);
	/*将接收到的个数放入接收消息队列中*/
	OTA_tRxMsgQueue[OTA_ucRxMsgHeadIndex].ulRxByteCount = ucRxByteCount;
	/*处理中的接收包索引+1 (0-7循环)*/
	OTA_ucRxMsgHeadIndex = (OTA_ucRxMsgHeadIndex + 1) % OTA_RX_QUEUE;
	/*清除DMA数据缓存*/
	(void)memset((void*)OTA_ucRxBuffer.ucData, 0x00, UART_DMA_BUFF_SIZE);
	/*打开接收下一次DMA数据*/
	UART_vReceiveDMA(&uart2_handler, &OTA_ucRxBuffer, UART_DMA_BUFF_SIZE);	//！！！将DMA的总传输次数设置为   UART_DMA_BUFF_SIZE
	/*释放一个串口接收信号量*/
	rt_sem_release(usart2_rx_sem);
}

/*发送消息列表--------------------------------------------------------*/
static const TOTAMsgTbDef OTA_tTransmitTb[] = {
	/*msgid*/						/*call back function*/
	{OTA_MESSAGE_HAND,				OTA_vTxInfo},
};
/*---------------------------------------------------------------------*/


/*接收消息列表-------------------------------------------------------------------*/
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

	return 0xFF;	//!< 没有发现 (注：发送列表不得大于这个数值)
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
	判断是否仍然处于发送状态，是就return-------
	code
	code
	code
	---------------------------------------------
	*/

	/*检查发送位是否被置位*/
	if (OTA_ulMsgToTransmitBits)
	{
		/*得到数组的索引值*/
		ucMessageIndex = OTA_ucGetTransmitMessageIndex();
		if (ucMessageIndex < sizeof(OTA_tTransmitTb) / sizeof(OTA_tTransmitTb[0]))
		{
			/*清除发送标志位*/
			DEF_RES(OTA_ulMsgToTransmitBits, 1 << ucMessageIndex);
			OTA_tMessageToSend.ucHeader0 = OTA_MSG_HEADER0;
			OTA_tMessageToSend.ucHeader1 = OTA_MSG_HEADER1;
			OTA_tMessageToSend.ucPayloadLen = 0;
			OTA_tTransmitTb[ucMessageIndex].pvServiceFunc(&OTA_tMessageToSend);

			ucCheckSum = OTA_ucGetCheckSumResult(&OTA_tMessageToSend.ucPayloadLen, OTA_tMessageToSend.ucPayloadLen + 1);//有效载荷数据+有效载荷数据长度 所以Len+1

			if (OTA_tMessageToSend.ucPayloadLen < OTA_TX_BUFFER_SIZE - 4)
			{
				(void)memcpy((void*)OTA_ucTxBuffer.ucData, (const void*)&OTA_tMessageToSend, OTA_tMessageToSend.ucPayloadLen + 3);	//!< including Header0, Header1, Payload length
				OTA_ucTxBuffer.ucData[OTA_tMessageToSend.ucPayloadLen + 3] = ucCheckSum;		// 复制ucPayloadLen+3的长度 相当于赋予ucData[0-PayloadLen+2]的数据 所以这里+3是checkSum位
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
		/*释放一个串口接收信号量*/
		rt_sem_release(usart2_tx_sem);
	}
}





static void OTA_vTransmitHandler_entry(void* parameter)
{
	/*创建一个发送信号量*/
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
	/*创建一个接收信号量*/
	usart2_rx_sem = rt_sem_create("usart2_rx_sem", 0, RT_IPC_FLAG_PRIO);
	if (usart2_rx_sem == RT_NULL)
	{
		rt_kprintf("create usart2_rx_sem failed.\n");

	}
	else
	{
		rt_kprintf("create done.usart2_rx_sem value = 0.\n");
	}
	Usart2RxIrqCallback(); //上电执行一下，因为第一次接收数据为0
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
	/* 创建OTA_vTransmitHandler_entry线程 */
	tid1 = rt_thread_create("OTA_vTransmitHandler_entry",
		OTA_vTransmitHandler_entry,
		RT_NULL,
		1024,
		7,
		20);
	/* 创建OTA_vTaskHandler_entry线程 */
	tid2 = rt_thread_create("OTA_vReceptionHandler_entry",
		OTA_vReceptionHandler_entry,
		RT_NULL,
		1024,
		7,
		20);

	/* 创建成功则启动线程 */
	if (tid1 != RT_NULL)rt_thread_startup(tid1);
	/* 创建成功则启动线程 */
	if (tid2 != RT_NULL)rt_thread_startup(tid2);
	return 0;
}
INIT_APP_EXPORT(OTA_iTaskHandler);
