#include "app_IOT2Board.h"
#include "usart.h"
#include "board.h"
#include "app_key.h"
#include "app_display.h"
#include "app_error.h"
#include "main.h"


/*私有变量-----------------------------------------------------------------------*/
static volatile uint32_t IOT_ulMsgToTransmitBits = 0;		//最大32个消息能被发送


static uint32_t Number_of_Transmit = 0;	//串口发送的次数(每次发送+1)

static TDmaBuffDef IOT_ucTxBuffer;	//发送的DMA数据缓存
static TDmaBuffDef IOT_ucRxBuffer;	//接收的DMA数据缓存


static volatile TIOTRxMessageObjDef IOT_tRxMsgQueue[IOT_RX_QUEUE];//IOT接收队列8个(0-7循环队列)

static volatile uint8_t IOT_ucRxMsgHeadIndex = 0;    //下一次新的接收包的索引
static volatile uint8_t IOT_ucRxMsgTailIndex = 0;    //处理中的接收包的索引

static  rt_sem_t usart3_rx_sem = RT_NULL;			//接收信号量
static  rt_sem_t usart3_tx_sem = RT_NULL;			//发送信号量
/*-------------------------------------------------------------------------------*/

/*私有函数-----------------------------------------------------------------------*/
static void IOT_vTxInfo(TIOTMsgObjDef* tObject);		//IOT发送回调函数
static void IOT_vRxInfo(TIOTMsgObjDef* tObject);		//IOT接收回调函数

static uint8_t IOT_ucGetTransmitMessageIndex(void);								 //得到发送列表序号
static uint8_t IOT_ucGetReceptionMessageIndex(uint8_t ucMsgUID);				 //得到接收列表序号
static uint16_t usChecksumCalculate(const uint8_t* pucBuffer, uint16_t usLength);//发送求和校验
static void IOT_vCheckTransmitTask(void);										 //检查发送任务
static void IOT_vCheckReceptionTask(TIOTRxMessageObjDef* ptRxMessage);			 //检查接收任务
static void IOT_vTransmitHandler_entry(void* parameter);						 //发送任务线程
static void IOT_vReceptionHandler_entry(void* parameter);						 //接收任务线程
/*-------------------------------------------------------------------------------*/



/*发送消息列表-------------------------------------------------------------------*/
static const TIOTMsgTbDef IOT_tTransmitTb[] = {
	/*msgid*/					/*call back function*/
	{IOT_INFO_REPORT,				IOT_vTxInfo},

};
/*-------------------------------------------------------------------------------*/

/*接收消息列表-------------------------------------------------------------------*/
static const TIOTMsgTbDef IOT_tReceiveTb[] = {
	/*msgid*/					/*call back function*/
	{IOT_INFO_REPORT,				IOT_vRxInfo},

};
/*-------------------------------------------------------------------------------*/



int IOT_iTaskHandler(void)
{
	rt_thread_t tid1,tid2;
	/* 创建IOT_vTransmitHandler_entry线程 */
	tid1 = rt_thread_create("IOT_vTransmitHandler_entry",
		IOT_vTransmitHandler_entry,
		RT_NULL,
		1024,
		7,
		20);
	/* 创建IOT_vTaskHandler_entry线程 */
	tid2 = rt_thread_create("IOT_vReceptionHandler_entry",
		IOT_vReceptionHandler_entry,
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
INIT_APP_EXPORT(IOT_iTaskHandler);

void Usart3RxIrqCallback(void)
{
	/*关闭定时器，清除标志位，关闭DMA通道*/
	TIMER0_Cmd(M4_TMR02, Tim0_ChannelA, Disable);
	USART_ClearStatus(USART_3, UsartRxTimeOut);
	DMA_ChannelCmd(DMA_UNIT_UART3_RX, DMA_CH_UART3_RX, Disable);

	/**获取DMA传输个数*/
	uint8_t ucRxByteCount = UART_DMA_BUFF_SIZE - UART_vGetDMACount(&uart3_handler);//GetDMACount是当前传输剩余次数 每接收一次就-1 所以接收的个数=总数-剩余值  
	/*清除接收消息队列数据*/
	(void)memset((void*)IOT_tRxMsgQueue[IOT_ucRxMsgHeadIndex].ucRxData, 0, UART_DMA_BUFF_SIZE);
	/*将接收的DMA数据缓存放入接收消息队列中*/
	(void)memcpy((void*)IOT_tRxMsgQueue[IOT_ucRxMsgHeadIndex].ucRxData, (const void*)IOT_ucRxBuffer.ucData, ucRxByteCount);
	/*将接收到的个数放入接收消息队列中*/
	IOT_tRxMsgQueue[IOT_ucRxMsgHeadIndex].ulRxByteCount = ucRxByteCount;
	/*处理中的接收包索引+1 (0-7循环)*/
	IOT_ucRxMsgHeadIndex = (IOT_ucRxMsgHeadIndex + 1) % IOT_RX_QUEUE;
	/*清除DMA数据缓存*/
	(void)memset((void*)IOT_ucRxBuffer.ucData, 0x00, UART_DMA_BUFF_SIZE);
	/*打开接收下一次DMA数据*/
	UART_vReceiveDMA(&uart3_handler, &IOT_ucRxBuffer, UART_DMA_BUFF_SIZE);	//！！！将DMA的总传输次数设置为   UART_DMA_BUFF_SIZE
	/*释放一个串口接收信号量*/
	rt_sem_release(usart3_rx_sem);
}
void IOT_vTransmitMessage(uint8_t ucMessageIndex)
{
	if (ucMessageIndex < IOT_MSG_ID_NUM)
	{
		DEF_SET(IOT_ulMsgToTransmitBits, 1 << ucMessageIndex);
		/*释放一个串口接收信号量*/
		rt_sem_release(usart3_tx_sem);
	}
}

static void IOT_vTxInfo(TIOTMsgObjDef* tObject)
{
	/*有效长度、设备编号、帧序号、有效载荷数据*/
	tObject->len = 10;
	tObject->deviceid = 2;
	tObject->seq= Number_of_Transmit;
	
	tObject->payload[0] = Key_ptGetInfo()->Work_Status;
	tObject->payload[1] = (Main_ptGetInfo()->IOT_systick_min & 0xFF);
	tObject->payload[2] = (Main_ptGetInfo()->IOT_systick_min >> 8 & 0xFF);
	tObject->payload[3] = (Main_ptGetInfo()->IOT_systick_min >> 16 & 0xFF);
	tObject->payload[4] = (Main_ptGetInfo()->IOT_systick_min >> 24 & 0xFF);
	tObject->payload[5] = Display_ptGetInfo()->batter_info;
	tObject->payload[6] = Error_GetInfo() & 0xFF;
	tObject->payload[7] = Error_GetInfo() >> 8 & 0xFF;
	tObject->payload[8] = Error_GetInfo() >> 16 & 0xFF;
	tObject->payload[9] = Error_GetInfo() >> 24 & 0xFF;
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

	return 0xFF;	//!< 没有发现 (注：发送列表不得大于这个数值)
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
	判断是否仍然处于发送状态，是就return-------
	code
	code
	code
	---------------------------------------------
	*/

	/*检查发送位是否被置位*/
	if (IOT_ulMsgToTransmitBits)
	{
		/*得到数组的索引值*/
		ucMessageIndex = IOT_ucGetTransmitMessageIndex();
		if (ucMessageIndex < sizeof(IOT_tTransmitTb) / sizeof(IOT_tTransmitTb[0]))
		{
			/*清除发送标志位*/  
			DEF_RES(IOT_ulMsgToTransmitBits, 1 << ucMessageIndex);

			IOT_tMessageToSend.head = IOT_BUFFER_HEAD;			//帧头
			IOT_tMessageToSend.tail = IOT_BUFFER_TAIL;			//帧尾
			IOT_tMessageToSend.len = 0;				//用于下面判断不等于0的情况
			IOT_tMessageToSend.msgid = IOT_tTransmitTb[ucMessageIndex].msgid;		//报文编号
			IOT_tTransmitTb[ucMessageIndex].pvServiceFunc(&IOT_tMessageToSend);		
			IOT_tMessageToSend.checksum = usChecksumCalculate(&IOT_tMessageToSend.head, IOT_tMessageToSend.len+5);//检验和
			/*先把帧头+报文编号+有效载荷长度+设备编号+帧序号*/
			(void)memcpy((void*)IOT_ucTxBuffer.ucData, (const void*)&IOT_tMessageToSend, 5);
			if (0 != IOT_tMessageToSend.len)
			{	/*有效载荷数据*/
				(void)memcpy((void*)(IOT_ucTxBuffer.ucData + 5), (const void*)&(IOT_tMessageToSend.payload), IOT_tMessageToSend.len);
			}
			/*校验和(2byte)+帧尾*/
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

	if (ptRxMessage->ulRxByteCount < IOT_RX_BUFFER_MIN_SIZE)//最小个数=帧头+报文编号+数据长度+设备编号+帧序号+数据+校验和(2)+帧尾
	{
		return;
	}
	/*1.判断帧头+帧尾*/
	if ((ptRxMessage->ucRxData[0] == IOT_BUFFER_HEAD)&&(ptRxMessage->ucRxData[ptRxMessage->ulRxByteCount-1]== IOT_BUFFER_TAIL))
	{
		/*2.判断*接收长度>=数据长度+其他8个 &&  数据长度<DMA最大接收长度-最小个数*/
		if ((ptRxMessage->ulRxByteCount >= (ptRxMessage->ucRxData[2] + IOT_RX_BUFFER_MIN_SIZE)) && (ptRxMessage->ucRxData[2] <= (IOT_RX_BUFFER_SIZE - IOT_RX_BUFFER_MIN_SIZE)))
		{
			/*3.求出Checksum的值*/
			ucCheckSum = usChecksumCalculate(ptRxMessage->ucRxData, ptRxMessage->ucRxData[2] + 5);
			/*4.检查Checksum的值*/
			if (ucCheckSum == ((ptRxMessage->ucRxData[ptRxMessage->ucRxData[2] + 5]) << 8 | (ptRxMessage->ucRxData[ptRxMessage->ucRxData[2] + 6])))
			{
				/*5.取出数据到临时变量*/
				tMessageToReceive.head = ptRxMessage->ucRxData[0];
				tMessageToReceive.msgid = ptRxMessage->ucRxData[1];
				tMessageToReceive.len = ptRxMessage->ucRxData[2];
				tMessageToReceive.deviceid = ptRxMessage->ucRxData[3];
				tMessageToReceive.seq = ptRxMessage->ucRxData[4];
				memcpy((void*)&tMessageToReceive.payload, (const void*)(ptRxMessage->ucRxData+5), ptRxMessage->ucRxData[2]);
				tMessageToReceive.checksum = ucCheckSum;
				tMessageToReceive.tail = ptRxMessage->ucRxData[ptRxMessage->ulRxByteCount - 1];
				/*6.取出接收序列号ID*/
				ucMessageIndex = IOT_ucGetReceptionMessageIndex(tMessageToReceive.msgid);
				if (ucMessageIndex == 0xFF)
				{
					return;	//没有找到
				}
				/*7.找到序列号ID，执行回调函数*/
				IOT_tReceiveTb[ucMessageIndex].pvServiceFunc(&tMessageToReceive);
			}
			
		}
	}
} 


static void IOT_vTransmitHandler_entry(void* parameter)
{
	/*创建一个发送信号量*/
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
	/*创建一个接收信号量*/
	usart3_rx_sem = rt_sem_create("usart3_rx_sem", 0, RT_IPC_FLAG_PRIO);
	if (usart3_rx_sem == RT_NULL)
	{
		rt_kprintf("create usart3_rx_sem failed.\n");

	}
	else
	{
		rt_kprintf("create done.usart3_rx_sem value = 0.\n");
	}
	Usart3RxIrqCallback(); //上电执行一下，因为第一次接收数据为0
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
