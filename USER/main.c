/**********************************************************************************
 * 工程名  :http通信服务器模式
 * 描述    :通过STM32开发板控制模块进入透传模式，连接上服务器后将收到服务器的数据返回给服务器
 * 实验平台:STM32F10X
 * 库版本  :
 * 作者    : WS CJ

 * 硬件连接说明
	 使用单片串口2与GPRS模块通信  注：使用串口2可以避免下载和通信不会冲突
	 STM32      GPRS模块
	 PA3 (RXD2)->RXD
	 PA2 (TXD2)->TXD
	 GND	      ->GND

**********************************************************************************/

#include "stm32f10x.h"
#include "usart.h"
#include "Led.h"
#include "SysTick.h"
#include "timer.h"
#include "string.h"
#include "stdio.h"
#include "iwdg.h"

#define uchar unsigned char
#define uint unsigned int
	
#define Buf2_Max 200 					  //串口2缓存长度
#define Buf3_Max 200            //串口3缓存长度

#define send_success 1
#define send_no_success 0

/*************  本地变量声明	**************/

char Uart2_Buf[Buf2_Max]; //串口2接收缓存
char Uart3_Buf[Buf3_Max]; //串口3接收缓存
char status[150] = "AT+HTTPPARA=\"URL\",\"http://api.jreader.net:9980/status.php?id=5201000001o\"";            //存储状态信息前部
char report[100] = "AT+HTTPPARA=\"URL\",\"http://api.jreader.net:9980/report.php?id=";            //存储上传信息前部
//char id_status[13] = "5201000001i\"";                 //存储ID
char id[12] = "5201000001o";                 //存储ID
char op[8] = "&op=-1\"";                  //存储状态
char str_status[150];               //存储状态信息
char str_report[150];               //存储上报信息

char *p1,*p2;             //串口2使用
char *p3,*p4;             //串口3使用

u8 Times=0,shijian=0;
vu8 Timer0_start;	//定时器0延时启动计数器

/*************	本地函数声明	**************/
void System_Initialization(void); //STM32系统初始化
void Ram_Initialization(void);    //内存变量初始化
void NVIC_Configuration(void);    //中断向量组初始化

void CLR_Buf2(void);              //清串口2接收缓存
void CLR_Buf3(void);              //请串口3接收缓存

u8 Find(char *a);                 //查找字符串
void Second_AT_Command(char *b,char *a,u8 wait_time);  //发送AT指令
void Second_AT_Command_1(char *b,char *a,u8 wait_time); //发送AT指令，并判断返回值是否正确
void Second_AT_Command_2(char *b,char *a,u8 wait_time); //发送AT指令，发送完不清除UART2缓存
u8 Wait_CREG(void);                          //查询等待模块注册成功
u8 Wait_CREG_mode(void);                     //查询检测模式确定阈值 默认是2000cm
void Set_ATE0(void);                           //取消回显
void Connect_Server(void);                     //配置GPRS连接参数，并进入透传模式
void Rec_Server_Data(void);                    //接收服务器数据并返回
void Rec_Server_Data_3(void);                  //串口3，感觉不用加
void Connect_Server_http(void);                //http协议向服务器发送数据
void Connect_Server_http_simple(void);         //http协议向服务器发送数据
void Connect_status(void);                     //发送状态信息

void Connect_Inquire_Number(void);              //查询电话号
void Connect_Server_http_open(void);           //open GPRS port and enable http
void Connect_Server_http_close(void); 

char * left(char *dst,char *src, int n);
//超声波-CJ，新加2018.0104
void DATA(void); 
int distance;
uchar  cc,bb;
u8 sum = 0;
u8 time = 0;
u8 num =0; //记录错误车数量
u8 jishu = 0;   //容错循环处理部分
int threshold = 2000;   //设置阈检测值 
int status_time = 0;
/*******************************************************************************
* 函数名 : main 
* 描述   : 主函数
* 输入   : 
* 输出   : 
* 返回   : 
* 注意   : 串口2负责与MG323模块通信，串口1用于串口调试，可以避免在下载程序时数据
					 还发送到模块
*******************************************************************************/
int main(void)
{
	System_Initialization(); //系统初始化
	UART1_SendString("系统初始化.............\r\n");
	Ram_Initialization();    //变量初始化
	UART1_SendString("变量初始化.............\r\n");

	strcpy(str_report,report);
	strcat(str_report,id);
	strcat(str_report,op);
	UART1_SendString(id);
	printf("\r\n");
	
//	strcpy(str_status,status);
//	strcat(str_status,id_status);
//	UART1_SendString(str_status);
//	printf("\r\n");
	
	Delay_nS(5);
//	Connect_Server_http_close();
//	UART1_SendString("关闭HTTP............\r\n");
	Set_ATE0();    //取消回显
	
  Connect_Server_http_open();
	UART1_SendString("打开HTTP...........\r\n");

	Connect_status();
	printf("%d",threshold);
	printf("\r\n");
	printf("open the dog ..................\r\n");
	IWDG_Init(6,4094);
	/*******超声波程序********/
	while(1)
		
	{	 
	   DATA();
		 cc=Uart3_Buf[1];   //接收距离的高8位
		 bb=Uart3_Buf[2];   //接收距离的低8位
		 IWDG_Feed();
		 distance = (int) ((cc<<8)|bb);
		
		if(distance < threshold)
		{		
			UART1_SendString("Yes! Strst!");	
		  UART1_SendString("\r\n");
			time++;
			Delay_nMs(20);	//yuan 200
		}
		else 
		{			
			if((distance>threshold)&(time>3))
				{
					sum++;
					UART1_SendString("Yes! there is one car");
					UART1_SendString("\r\n");
					printf("the sum of the car:%d\r\n",sum);
					
					/*************fasong shuju *****************/

//					Delay_nS(1); 
//					UART2_SendString("+++");//退出透传模式，避免模块还处于透传模式中
//					//Delay_nS(1);
					UART1_SendString("GPRS模块开始连接服务器\r\n");
					//Set_ATE0();    //取消回显
					if(sum>1)
					{
						Connect_Server_http_simple();
					}
					else
					{
						Connect_Server_http();
					}
					UART1_SendString("数据发送成功\r\n");
					time = 0;
				}
			UART1_SendString("No_car");
			UART1_SendString("\r\n");
			time = 0;	
		 }
		if(num > 2)
		 {
			 printf("进行容错处理..................");
			 for(jishu=num;jishu>0;jishu--)
			 {
				 /************************容错处理*************************************/
				 DATA();
				 cc=Uart3_Buf[1];   //接收距离的高8位
				 bb=Uart3_Buf[2];   //接收距离的低8位
				 IWDG_Feed();
				 distance = (int) ((cc<<8)|bb);
				 
				 if(distance < threshold)
					{		
						UART1_SendString("Yes! Strst!");	
						UART1_SendString("\r\n");
						time++;
						Delay_nMs(20);	//yuan 200
					}
					else 
					{			
						if((distance>threshold)&(time>3))
							{
								sum++;
								UART1_SendString("Yes! there is one car");
								UART1_SendString("\r\n");
								printf("the sum of the car:%d\r\n",sum);
					
								/*************fasong shuju *****************/

//								Delay_nS(1); 
//								UART2_SendString("+++");//退出透传模式，避免模块还处于透传模式中
//								Delay_nS(1);
								UART1_SendString("GPRS模块开始连接服务器\r\n");
//								Set_ATE0();    //取消回显
								Connect_Server_http();
								UART1_SendString("数据发送成功\r\n");
								time = 0;
							}
								UART1_SendString("No_car");
								UART1_SendString("\r\n");
								time = 0;	
							
							/*************发送错误补偿**********************/
							Delay_nS(1); 
							UART2_SendString("+++");//退出透传模式，避免模块还处于透传模式中
							Delay_nS(1);
							UART1_SendString("GPRS模块开始连接服务器\r\n");
							Set_ATE0();    //取消回显
							Connect_Server_http();
							UART1_SendString("数据发送成功\r\n");
							num--;						
					}
				 
				 /************************end******************************************/
			 }
		 }
		 
		 CLR_Buf3();
		 Delay_nMs(200);
			
		 status_time++;
		 if(status_time == 18000)
		 {	
			 Connect_status();
			 status_time = 0;
		 }
	 }
	  
}

/*******************************************************************************
* 函数名  : System_Initialization
* 描述    : STM32系统初始化函数(初始化STM32时钟及外设)
* 输入    : 无
* 输出    : 无
* 返回    : 无 
* 说明    : 无
*******************************************************************************/
void System_Initialization(void)
{
	RCC_Configuration();		//设置系统时钟为72MHZ(这个可以根据需要改)
  SysTick_Init_Config();  //初始化系统滴答时钟SysTick
  NVIC_Configuration();		//STM32中断向量表配配置
	Timer2_Init_Config();		//定时器2初始化配置
	USART1_Init_Config(115200);	//串口1初始化配置
	USART2_Init_Config(115200);	//串口2初始化配置	
	USART3_Init_Config(9600);   //串口3初始化配置
	GPIO_Config();          //初始化GPIO
}
/*******************************************************************************
* 函数名  : Ram_Initialization
* 描述    : 变量初始化函数
* 输入    : 无
* 输出    : 无
* 返回    : 无 
* 说明    : 无
*******************************************************************************/
void Ram_Initialization(void)
{
	Times=0;
	shijian=0;
	Timer0_start=0;
	p1=Uart2_Buf;
	p2=p1;
	p3=Uart3_Buf;
	p4=p3;
}

/*******************************************************************************
* 函数名  : NVIC_Configuration
* 描述    : STM32中断向量表配配置
* 输入    : 无
* 输出    : 无
* 返回值  : 无
* 说明    : 设置KEY1(PC11)的中断优先组
*******************************************************************************/
void NVIC_Configuration(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;						//定义NVIC初始化结构体

  /* Set the Vector Table base location at 0x08000000 */
  NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x0);
	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);				    //设置中断优先级组为2，先占优先级和从优先级各两位(可设0～3)
	
	/*定时器2中断向量配置*/
	NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;				    //设置中断向量号
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 4;	//设置抢先优先级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;			  //设置响应优先级
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;				    //使能NVIC
	NVIC_Init(&NVIC_InitStructure);
	/*串口1中断向量配置*/
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;				  //设置中断向量号
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;	//设置抢先优先级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;			  //设置响应优先级
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;				    //使能NVIC
	NVIC_Init(&NVIC_InitStructure);
  /*串口2中断向量配置*/
  NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;				  //设置中断向量号
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;	//设置抢先优先级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;			  //设置响应优先级
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;				    //使能NVIC
	NVIC_Init(&NVIC_InitStructure);
	
	/*Usart3 NVIC配置*/
	NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=1;	//抢占优先级1
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;		//从优先级0
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);							//根据指定的参数初始化VIC寄存器 
	
}
/*******************************************************************************
* 函数名  : USART2_IRQHandler
* 描述    : 串口2中断服务程序
* 输入    : 无
* 返回    : 无 
* 说明    : 
*******************************************************************************/
void USART2_IRQHandler(void)                	
{
			u8 Res=0;
      if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET)
			{
			Res=USART_ReceiveData(USART2);
			*p1 = Res;  	  //将接收到的字符串存到缓存中
			p1++;                			//缓存指针向后移动
			if(p1>&Uart2_Buf[Buf2_Max])       		//如果缓存满,将缓存指针指向缓存的首地址
			{
				p1=Uart2_Buf;
			} 
		  }			
} 	

/*******************************************************************************
* 函数名  : USART3_IRQHandler
* 描述    : 串口3中断服务程序
* 输入    : 无
* 返回    : 无 
* 说明    : 
*******************************************************************************/
void USART3_IRQHandler(void)                	
{
			u8 Res=0;
      if(USART_GetITStatus(USART3, USART_IT_RXNE) != RESET)
			{
			Res=USART_ReceiveData(USART3);
			*p3 = Res;  	  //将接收到的字符串存到缓存中
			p3++;                			//缓存指针向后移动
			if(p3>&Uart3_Buf[Buf3_Max])       		//如果缓存满,将缓存指针指向缓存的首地址
			{
				p3=Uart3_Buf;
			} 
		  }			
} 

/*******************************************************************************
* 函数名  : TIM2_IRQHandler
* 描述    : 定时器2中断断服务函数
* 输入    : 无
* 输出    : 无
* 返回    : 无 
* 说明    : 无
*******************************************************************************/
void TIM2_IRQHandler(void)   //TIM2中断
{
	static u8 flag =1;

	if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET)  //检查TIM3更新中断发生与否
	{
		
		TIM_ClearITPendingBit(TIM2, TIM_IT_Update  );  //清除TIMx更新中断标志 
		
		if(Timer0_start)
		Times++;
		if(Times > shijian)
		{
			Timer0_start = 0;
			Times = 0;
		}
		
		if(flag)
		{
			LED4_ON(); 
			flag=0;
		}
		else
		{
			LED4_OFF(); 
			flag=1;
		}
	}	
}

/*******************************************************************************
* 函数名 : CLR_Buf2
* 描述   : 清除串口2缓存数据
* 输入   : 
* 输出   : 
* 返回   : 
* 注意   : 
*******************************************************************************/
void CLR_Buf2(void)
{
	u16 k;
	for(k=0;k<Buf2_Max;k++)      //将缓存内容清零
	{
		Uart2_Buf[k] = 0x00;
	}
  p1=Uart2_Buf;               //接收字符串的起始存储位置
}

/*******************************************************************************
* 函数名 : CLR_Buf3
* 描述   : 清除串口3缓存数据
* 输入   : 
* 输出   : 
* 返回   : 
* 注意   : 
*******************************************************************************/
void CLR_Buf3(void)
{
	u16 k;
	for(k=0;k<Buf3_Max;k++)      //将缓存内容清零
	{
		Uart3_Buf[k] = 0x00;
	}
  p3=Uart3_Buf;               //接收字符串的起始存储位置
}

/*******************************************************************************
* 函数名 : Find
* 描述   : 判断缓存中是否含有指定的字符串
* 输入   : 
* 输出   : 
* 返回   : unsigned char:1 找到指定字符，0 未找到指定字符 
* 注意   : 
*******************************************************************************/

u8 Find(char *a)
{ 
  if(strstr(Uart2_Buf,a)!=NULL)
	    return 1;
	else
			return 0;
}

/*******************************************************************************
* 函数名 : Second_AT_Command
* 描述   : 发送AT指令函数
* 输入   : 发送数据的指针、发送等待时间(单位：S)
* 输出   : 
* 返回   : 
* 注意   : 
*******************************************************************************/

 void Second_AT_Command(char *b,char *a,u8 wait_time)         
{
	u8 i;
	char *c;
	c = b;										//保存字符串地址到c
	CLR_Buf2(); 
  i = 0;

	
	while(i == 0)                    
	{
		if(!Find(a)) 
		{
			if(Timer0_start == 0)
			{
				b = c;							//将字符串地址给b
				for (; *b!='\0';b++)
				{
					while(USART_GetFlagStatus(USART2, USART_FLAG_TC)==RESET);
					USART_SendData(USART2,*b);//UART2_SendData(*b);
				}
				UART2_SendLR();	//发送回车
				Times = 0;
				shijian = wait_time;
				Timer0_start = 1;
		   }
    }
 	  else
		{
			i = 1;
			Timer0_start = 0;
			Delay_nS(1);
			uat_putstr(Uart2_Buf);
		}
	}
	Delay_nS(1);
	CLR_Buf2(); 
}


/**********************新增加：2018.01.03***********************************/
/*******************************************************************************
* 函数名 : Second_AT_Command_1
* 描述   : 发送AT指令函数,并判断返回值是否正确。
* 输入   : 发送数据的指针、发送等待时间(单位：S)
* 输出   : 
* 返回   : 
* 注意   : 
*******************************************************************************/

void Second_AT_Command_1(char *b,char *a,u8 wait_time)         
{
	u8 i;
	char *c;
	c = b;										//保存字符串地址到c
	CLR_Buf2(); 
  i = 0;

	
	while(i == 0)                    
	{
		if(!Find(a)) 
		{
			if(Timer0_start == 0)
			{
				b = c;							//将字符串地址给b
				for (; *b!='\0';b++)
				{
					while(USART_GetFlagStatus(USART2, USART_FLAG_TC)==RESET);
					USART_SendData(USART2,*b);//UART2_SendData(*b);
				}
				UART2_SendLR();	//发送回车
				Times = 0;
				shijian = wait_time;
				Timer0_start = 1;
		   }
    }
 	  else
		{
			i = 1;
			Timer0_start = 0;
			Delay_nS(1);
			uat_putstr(Uart2_Buf);
			if(!Wait_CREG())
			{
				num++;
			}
		}
	}
	
	CLR_Buf2(); 
}

/*******************************************************************************
* 函数名 : Second_AT_Command_2
* 描述   : 发送AT指令函数
* 输入   : 发送数据的指针、发送等待时间(单位：S) 发送完不清除UART2缓存
* 输出   : 
* 返回   : 
* 注意   : 
*******************************************************************************/

 void Second_AT_Command_2(char *b,char *a,u8 wait_time)         
{
	u8 i;
	char *c;
	c = b;										//保存字符串地址到c
	CLR_Buf2(); 
  i = 0;
	while(i == 0)                    
	{
		if(!Find(a)) 
		{
			if(Timer0_start == 0)
			{
				b = c;							//将字符串地址给b
				for (; *b!='\0';b++)
				{
					while(USART_GetFlagStatus(USART2, USART_FLAG_TC)==RESET);
					USART_SendData(USART2,*b);//UART2_SendData(*b);
				}
				UART2_SendLR();	//发送回车
				Times = 0;
				shijian = wait_time;
				Timer0_start = 1;
		   }
    }
 	  else
		{
			i = 1;
			Timer0_start = 0;
			Delay_nS(1);
			uat_putstr(Uart2_Buf);
			Wait_CREG_mode();
		}
	}
	Delay_nS(1);
	 
}

/*******************新加修改程序2018.0104***********************************************/
/*******************************************************************************
* 函数名 : Wait_CREG(判断返回值)
* 描述   : 等待模块注册成功
* 输入   : 
* 输出   : 返回值为1正确，返回值为0错误
* 返回   : 
* 注意   : 
*******************************************************************************/
u8 Wait_CREG(void)
{
	u8 i = 0;
	u8 k; 
	Delay_nS(1);  						
	for(k=0;k<Buf2_Max;k++)      			
   {
			if((Uart2_Buf[k] == '2')&(Uart2_Buf[k+1] == '0')&(Uart2_Buf[k+2]=='0'))
			{
//				if(Uart2_Buf[k+5] == '1')   //说明注册成功
			
					UART1_SendString("发送成功...*********....***.....");
					UART1_SendString("\r\n");
					
					i = 1;
			}
			else
			{
					UART1_SendString("发送失败......重新发送");
					UART1_SendString("\r\n");
			}
			
		}
		if(i==1)
		{
			return send_success;
		}
		else
		{
			return send_no_success;
		}
	
}

/*******************************************************************************
* 函数名 : Wait_CREG_mode(判断返回值)
* 描述   : 判断检测阈值 默认情况为2000cm
* 输入   : 
* 输出   : 返回值为1为传输正确，返回值为0传输错误
* 返回   : 
* 注意   : 
*******************************************************************************/
u8 Wait_CREG_mode(void)
{
	u8 i = 0;
	u8 k; 
	Delay_nS(1);  						
	for(k=0;k<Buf2_Max;k++)      			
   {
			if((Uart2_Buf[k] == 'd')&(Uart2_Buf[k+1] == 'e'))
			{
				if(Uart2_Buf[k+5] == '1')   //说明注册成功
				{	
					threshold = 1000;
					printf("选择阈值为1000\r\n");
					i = 1;
				}
				else if(Uart2_Buf[k+5] == '2')
				{
					threshold = 2000;
					printf("选择阈值为2000\r\n");
					i = 1;
				}
				else if(Uart2_Buf[k+5] == '3')
				{
					threshold = 3000;
					printf("选择阈值为3000\r\n");
					i = 1;
				}
				else
				{
					threshold = 2000;
					printf("选择阈值为2000\r\n");
				}
			}
		}
		if(i==1)
		{
			return send_success;
		}
		else
		{
			return send_no_success;
		}
	
}


/*******************************************************************************
* 函数名 : Set_ATE0
* 描述   : 取消回显
* 输入   : 
* 输出   : 
* 返回   : 
* 注意   : 
*******************************************************************************/
void Set_ATE0(void)
{
	CLR_Buf2();
	Second_AT_Command("ATE0","OK",3);								//取消回显		
}
/*******************************************************************************
* 函数名 : Connect_Server
* 描述   : GPRS连接服务器函数
* 输入   : 
* 输出   : 
* 返回   : 
* 注意   : 
*******************************************************************************/
void Connect_Server(void)
{
	UART2_SendString("AT+CIPCLOSE=1");	//关闭连接
  Delay_nMs(100);
	Second_AT_Command("AT+CIPSHUT","SHUT OK",2);		//关闭移动场景
	Second_AT_Command("AT+CGCLASS=\"B\"","OK",2);//设置GPRS移动台类别为B,支持包交换和数据交换 
	Second_AT_Command("AT+CGDCONT=1,\"IP\",\"CMNET\"","OK",2);//设置PDP上下文,互联网接协议,接入点等信息
	Second_AT_Command("AT+CGATT=1","OK",2);//附着GPRS业务
	Second_AT_Command("AT+CIPCSGP=1,\"CMNET\"","OK",2);//设置为GPRS连接模式
  Second_AT_Command("AT+CIPMUX=0","OK",2);//设置为单路连接
	Second_AT_Command("AT+CIPHEAD=1","OK",2);//设置接收数据显示IP头(方便判断数据来源,仅在单路连接有效)
	Second_AT_Command("AT+CIPMODE=1","OK",2);//打开透传功能
	Second_AT_Command("AT+CIPCCFG=4,5,200,1","OK",2);//配置透传模式：单包重发次数:2,间隔1S发送一次,每次发送200的字节
  //Second_AT_Command((char*)string,"OK",5);//建立连接
  
  Delay_nMs(100);                                //等待串口数据接收完毕
  CLR_Buf2();                                    //清串口接收缓存为透传模式准备
}

/**************************2018.01.04 open GPRS port and enable GPRS*****************/
void Connect_Server_http_open(void)
{
	Second_AT_Command("AT+SAPBR=3,1,\"Contype\",\"GPRS\"","OK",3);
	//Delay_nS(3);
	UART1_SendString("配置承载场景1\r\n");
	
	Second_AT_Command("AT+SAPBR=3,1,\"APN\",\"CMNET\"","OK",3);
	//Delay_nS(3);
	UART1_SendString("配置承载场景2\r\n");
	
	Second_AT_Command("AT+SAPBR=1,1","O",4);
	//Delay_nS(3);
	UART1_SendString("激活一个GPRS上下文\r\n");
	
//	Second_AT_Command("AT+SAPBR=1,1","O",4);
//	//Delay_nS(3);
//	UART1_SendString("激活一个GPRS上下文\r\n");
	
	Second_AT_Command("AT+HTTPINIT","O",3);
	//Delay_nS(3);
	UART1_SendString("初始化HTTP服务\r\n");
	
}
void Connect_Server_http_close(void)   
{
	Second_AT_Command("AT+HTTPTERM","OK",3);
	//Delay_nS(1);
	UART1_SendString("结束HTTP服务器数据\r\n");
	
	Second_AT_Command("AT+SAPBR=0,1","OK",3);
	//Delay_nS(1);
	UART1_SendString("关闭数据连接\r\n");
}

void Connect_status(void)   
{
	
//	UART1_SendString(status);
//	printf("\r\n");
//	UART1_SendString(str_status);
//	printf("\r\n");
//	UART1_SendString(str_report);
//	printf("\r\n");
	//Second_AT_Command("AT+HTTPPARA=\"URL\",\"http://api.jreader.net:9980/status.php?id=5201000001i\"","OK",3);
	Second_AT_Command(status,"OK", 3);
	UART1_SendString("发送状态报告，选择阈值\r\n");
	
	Second_AT_Command("AT+HTTPACTION=0","OK",4);
	UART1_SendString("GET会话开始\r\n");
	
	Second_AT_Command("AT+HTTPREAD","OK",4);
	
	Second_AT_Command_2("AT+HTTPREAD","OK", 4);  
	UART1_SendString("读取返回值,完成发送过程\r\n");

}

/*******************************************************************************
*函数： Connect_Inquire_Number
*描述： 查询电话卡号
*
*
*******************************************************************************/
void Connect_Inquire_Number(void)
{
	Second_AT_Command_2("AT+CCID","OK",3);
	//Delay_nS(2);
	UART1_SendString("查询设备ID\r\n");
	//strncpy(id,Uart2_Buf,20);
	
	UART1_SendString(Uart2_Buf);
	strcat(str_status,status);
	strcat(str_status,id);
	UART1_SendString(str_status);
}

/*******************************************************************************
*函数： Connect_Server_http
*描述： 根据http协议连接服务器
*
*
*******************************************************************************/
void Connect_Server_http(void)
{
	//UART1_SendString(str_report);
	
	Second_AT_Command(str_report,"OK",1);
	//Second_AT_Command("AT+HTTPPARA=\"URL\",\"http://api.jreader.net:9980/report.php?id=a1&op=-1\"","OK",3);
	//Delay_nS(1);
	UART1_SendString("访问网站\r\n");
		
	Second_AT_Command_1("AT+HTTPACTION=0","200",1);
	//Delay_nS(2);
	UART1_SendString("GET会话开始\r\n");
	
	
	Second_AT_Command("AT+HTTPREAD","OK", 1);
	//Delay_nS(4);   
	UART1_SendString("读取返回值,完成发送过程\r\n");

	CLR_Buf2();
}


void Connect_Server_http_simple(void)
{
	
	Second_AT_Command_1("AT+HTTPACTION=0","0",1);
	//Delay_nS(2);
	UART1_SendString("GET会话开始\r\n");
	CLR_Buf2();
}
/*******************************************************************************
* 函数名 : Rec_Server_Data
* 描述   : 接收服务器数据函数,并原样返回
* 输入   : 
* 输出   : 
* 返回   : 
* 注意   : 
*******************************************************************************/
void Rec_Server_Data(void)
{
	if(p2!=p1)   		//说明还有数据未发出
	{	
		while(USART_GetFlagStatus(USART2, USART_FLAG_TC)==RESET);
		USART_SendData(USART2,*p2);
		p2++;
    if(p2>&Uart2_Buf[Buf2_Max])
			p2=Uart2_Buf;
	}
}

/*******************************************************************************
* 函数名 : Rec_Server_Data
* 描述   : 接收服务器数据函数,并原样返回
* 输入   : 
* 输出   : 
* 返回   : 
* 注意   : 
*******************************************************************************/
void Rec_Server_Data_3(void)
{
	if(p4!=p3)   		//说明还有数据未发出
	{	
		while(USART_GetFlagStatus(USART3, USART_FLAG_TC)==RESET);
		USART_SendData(USART3,*p4);
		p4++;
    if(p4>&Uart3_Buf[Buf3_Max])
			p4=Uart3_Buf;
	}
}

/*********************************

发送16进制数

************************************/

void DATA(void)
{    
  u8 DATA_1=0x55;  
	while((USART3->SR&0x0080)==0);    
  USART3->DR=DATA_1;             
}

/*************************string copy left*************/
char * left(char *dst,char *src, int n)  
{  
    char *p = src;  
    char *q = dst;  
    int len = strlen(src);  
    if(n>len) n = len;  
    while(n--) *(q++) = *(p++);  
    *(q++)='\0';  
    return dst;  
}
