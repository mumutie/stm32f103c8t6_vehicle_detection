/**********************************************************************************
 * ������  :httpͨ�ŷ�����ģʽ
 * ����    :ͨ��STM32���������ģ�����͸��ģʽ�������Ϸ��������յ������������ݷ��ظ�������
 * ʵ��ƽ̨:STM32F10X
 * ��汾  :
 * ����    : WS CJ

 * Ӳ������˵��
	 ʹ�õ�Ƭ����2��GPRSģ��ͨ��  ע��ʹ�ô���2���Ա������غ�ͨ�Ų����ͻ
	 STM32      GPRSģ��
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
	
#define Buf2_Max 200 					  //����2���泤��
#define Buf3_Max 200            //����3���泤��

#define send_success 1
#define send_no_success 0

/*************  ���ر�������	**************/

char Uart2_Buf[Buf2_Max]; //����2���ջ���
char Uart3_Buf[Buf3_Max]; //����3���ջ���
char status[150] = "AT+HTTPPARA=\"URL\",\"http://api.jreader.net:9980/status.php?id=5201000001o\"";            //�洢״̬��Ϣǰ��
char report[100] = "AT+HTTPPARA=\"URL\",\"http://api.jreader.net:9980/report.php?id=";            //�洢�ϴ���Ϣǰ��
//char id_status[13] = "5201000001i\"";                 //�洢ID
char id[12] = "5201000001o";                 //�洢ID
char op[8] = "&op=-1\"";                  //�洢״̬
char str_status[150];               //�洢״̬��Ϣ
char str_report[150];               //�洢�ϱ���Ϣ

char *p1,*p2;             //����2ʹ��
char *p3,*p4;             //����3ʹ��

u8 Times=0,shijian=0;
vu8 Timer0_start;	//��ʱ��0��ʱ����������

/*************	���غ�������	**************/
void System_Initialization(void); //STM32ϵͳ��ʼ��
void Ram_Initialization(void);    //�ڴ������ʼ��
void NVIC_Configuration(void);    //�ж��������ʼ��

void CLR_Buf2(void);              //�崮��2���ջ���
void CLR_Buf3(void);              //�봮��3���ջ���

u8 Find(char *a);                 //�����ַ���
void Second_AT_Command(char *b,char *a,u8 wait_time);  //����ATָ��
void Second_AT_Command_1(char *b,char *a,u8 wait_time); //����ATָ����жϷ���ֵ�Ƿ���ȷ
void Second_AT_Command_2(char *b,char *a,u8 wait_time); //����ATָ������겻���UART2����
u8 Wait_CREG(void);                          //��ѯ�ȴ�ģ��ע��ɹ�
u8 Wait_CREG_mode(void);                     //��ѯ���ģʽȷ����ֵ Ĭ����2000cm
void Set_ATE0(void);                           //ȡ������
void Connect_Server(void);                     //����GPRS���Ӳ�����������͸��ģʽ
void Rec_Server_Data(void);                    //���շ��������ݲ�����
void Rec_Server_Data_3(void);                  //����3���о����ü�
void Connect_Server_http(void);                //httpЭ�����������������
void Connect_Server_http_simple(void);         //httpЭ�����������������
void Connect_status(void);                     //����״̬��Ϣ

void Connect_Inquire_Number(void);              //��ѯ�绰��
void Connect_Server_http_open(void);           //open GPRS port and enable http
void Connect_Server_http_close(void); 

char * left(char *dst,char *src, int n);
//������-CJ���¼�2018.0104
void DATA(void); 
int distance;
uchar  cc,bb;
u8 sum = 0;
u8 time = 0;
u8 num =0; //��¼��������
u8 jishu = 0;   //�ݴ�ѭ��������
int threshold = 2000;   //�����м��ֵ 
int status_time = 0;
/*******************************************************************************
* ������ : main 
* ����   : ������
* ����   : 
* ���   : 
* ����   : 
* ע��   : ����2������MG323ģ��ͨ�ţ�����1���ڴ��ڵ��ԣ����Ա��������س���ʱ����
					 �����͵�ģ��
*******************************************************************************/
int main(void)
{
	System_Initialization(); //ϵͳ��ʼ��
	UART1_SendString("ϵͳ��ʼ��.............\r\n");
	Ram_Initialization();    //������ʼ��
	UART1_SendString("������ʼ��.............\r\n");

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
//	UART1_SendString("�ر�HTTP............\r\n");
	Set_ATE0();    //ȡ������
	
  Connect_Server_http_open();
	UART1_SendString("��HTTP...........\r\n");

	Connect_status();
	printf("%d",threshold);
	printf("\r\n");
	printf("open the dog ..................\r\n");
	IWDG_Init(6,4094);
	/*******����������********/
	while(1)
		
	{	 
	   DATA();
		 cc=Uart3_Buf[1];   //���վ���ĸ�8λ
		 bb=Uart3_Buf[2];   //���վ���ĵ�8λ
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
//					UART2_SendString("+++");//�˳�͸��ģʽ������ģ�黹����͸��ģʽ��
//					//Delay_nS(1);
					UART1_SendString("GPRSģ�鿪ʼ���ӷ�����\r\n");
					//Set_ATE0();    //ȡ������
					if(sum>1)
					{
						Connect_Server_http_simple();
					}
					else
					{
						Connect_Server_http();
					}
					UART1_SendString("���ݷ��ͳɹ�\r\n");
					time = 0;
				}
			UART1_SendString("No_car");
			UART1_SendString("\r\n");
			time = 0;	
		 }
		if(num > 2)
		 {
			 printf("�����ݴ���..................");
			 for(jishu=num;jishu>0;jishu--)
			 {
				 /************************�ݴ���*************************************/
				 DATA();
				 cc=Uart3_Buf[1];   //���վ���ĸ�8λ
				 bb=Uart3_Buf[2];   //���վ���ĵ�8λ
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
//								UART2_SendString("+++");//�˳�͸��ģʽ������ģ�黹����͸��ģʽ��
//								Delay_nS(1);
								UART1_SendString("GPRSģ�鿪ʼ���ӷ�����\r\n");
//								Set_ATE0();    //ȡ������
								Connect_Server_http();
								UART1_SendString("���ݷ��ͳɹ�\r\n");
								time = 0;
							}
								UART1_SendString("No_car");
								UART1_SendString("\r\n");
								time = 0;	
							
							/*************���ʹ��󲹳�**********************/
							Delay_nS(1); 
							UART2_SendString("+++");//�˳�͸��ģʽ������ģ�黹����͸��ģʽ��
							Delay_nS(1);
							UART1_SendString("GPRSģ�鿪ʼ���ӷ�����\r\n");
							Set_ATE0();    //ȡ������
							Connect_Server_http();
							UART1_SendString("���ݷ��ͳɹ�\r\n");
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
* ������  : System_Initialization
* ����    : STM32ϵͳ��ʼ������(��ʼ��STM32ʱ�Ӽ�����)
* ����    : ��
* ���    : ��
* ����    : �� 
* ˵��    : ��
*******************************************************************************/
void System_Initialization(void)
{
	RCC_Configuration();		//����ϵͳʱ��Ϊ72MHZ(������Ը�����Ҫ��)
  SysTick_Init_Config();  //��ʼ��ϵͳ�δ�ʱ��SysTick
  NVIC_Configuration();		//STM32�ж�������������
	Timer2_Init_Config();		//��ʱ��2��ʼ������
	USART1_Init_Config(115200);	//����1��ʼ������
	USART2_Init_Config(115200);	//����2��ʼ������	
	USART3_Init_Config(9600);   //����3��ʼ������
	GPIO_Config();          //��ʼ��GPIO
}
/*******************************************************************************
* ������  : Ram_Initialization
* ����    : ������ʼ������
* ����    : ��
* ���    : ��
* ����    : �� 
* ˵��    : ��
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
* ������  : NVIC_Configuration
* ����    : STM32�ж�������������
* ����    : ��
* ���    : ��
* ����ֵ  : ��
* ˵��    : ����KEY1(PC11)���ж�������
*******************************************************************************/
void NVIC_Configuration(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;						//����NVIC��ʼ���ṹ��

  /* Set the Vector Table base location at 0x08000000 */
  NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x0);
	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);				    //�����ж����ȼ���Ϊ2����ռ���ȼ��ʹ����ȼ�����λ(����0��3)
	
	/*��ʱ��2�ж���������*/
	NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;				    //�����ж�������
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 4;	//�����������ȼ�
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;			  //������Ӧ���ȼ�
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;				    //ʹ��NVIC
	NVIC_Init(&NVIC_InitStructure);
	/*����1�ж���������*/
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;				  //�����ж�������
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;	//�����������ȼ�
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;			  //������Ӧ���ȼ�
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;				    //ʹ��NVIC
	NVIC_Init(&NVIC_InitStructure);
  /*����2�ж���������*/
  NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;				  //�����ж�������
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;	//�����������ȼ�
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;			  //������Ӧ���ȼ�
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;				    //ʹ��NVIC
	NVIC_Init(&NVIC_InitStructure);
	
	/*Usart3 NVIC����*/
	NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=1;	//��ռ���ȼ�1
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;		//�����ȼ�0
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQͨ��ʹ��
	NVIC_Init(&NVIC_InitStructure);							//����ָ���Ĳ�����ʼ��VIC�Ĵ��� 
	
}
/*******************************************************************************
* ������  : USART2_IRQHandler
* ����    : ����2�жϷ������
* ����    : ��
* ����    : �� 
* ˵��    : 
*******************************************************************************/
void USART2_IRQHandler(void)                	
{
			u8 Res=0;
      if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET)
			{
			Res=USART_ReceiveData(USART2);
			*p1 = Res;  	  //�����յ����ַ����浽������
			p1++;                			//����ָ������ƶ�
			if(p1>&Uart2_Buf[Buf2_Max])       		//���������,������ָ��ָ�򻺴���׵�ַ
			{
				p1=Uart2_Buf;
			} 
		  }			
} 	

/*******************************************************************************
* ������  : USART3_IRQHandler
* ����    : ����3�жϷ������
* ����    : ��
* ����    : �� 
* ˵��    : 
*******************************************************************************/
void USART3_IRQHandler(void)                	
{
			u8 Res=0;
      if(USART_GetITStatus(USART3, USART_IT_RXNE) != RESET)
			{
			Res=USART_ReceiveData(USART3);
			*p3 = Res;  	  //�����յ����ַ����浽������
			p3++;                			//����ָ������ƶ�
			if(p3>&Uart3_Buf[Buf3_Max])       		//���������,������ָ��ָ�򻺴���׵�ַ
			{
				p3=Uart3_Buf;
			} 
		  }			
} 

/*******************************************************************************
* ������  : TIM2_IRQHandler
* ����    : ��ʱ��2�ж϶Ϸ�����
* ����    : ��
* ���    : ��
* ����    : �� 
* ˵��    : ��
*******************************************************************************/
void TIM2_IRQHandler(void)   //TIM2�ж�
{
	static u8 flag =1;

	if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET)  //���TIM3�����жϷ������
	{
		
		TIM_ClearITPendingBit(TIM2, TIM_IT_Update  );  //���TIMx�����жϱ�־ 
		
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
* ������ : CLR_Buf2
* ����   : �������2��������
* ����   : 
* ���   : 
* ����   : 
* ע��   : 
*******************************************************************************/
void CLR_Buf2(void)
{
	u16 k;
	for(k=0;k<Buf2_Max;k++)      //��������������
	{
		Uart2_Buf[k] = 0x00;
	}
  p1=Uart2_Buf;               //�����ַ�������ʼ�洢λ��
}

/*******************************************************************************
* ������ : CLR_Buf3
* ����   : �������3��������
* ����   : 
* ���   : 
* ����   : 
* ע��   : 
*******************************************************************************/
void CLR_Buf3(void)
{
	u16 k;
	for(k=0;k<Buf3_Max;k++)      //��������������
	{
		Uart3_Buf[k] = 0x00;
	}
  p3=Uart3_Buf;               //�����ַ�������ʼ�洢λ��
}

/*******************************************************************************
* ������ : Find
* ����   : �жϻ������Ƿ���ָ�����ַ���
* ����   : 
* ���   : 
* ����   : unsigned char:1 �ҵ�ָ���ַ���0 δ�ҵ�ָ���ַ� 
* ע��   : 
*******************************************************************************/

u8 Find(char *a)
{ 
  if(strstr(Uart2_Buf,a)!=NULL)
	    return 1;
	else
			return 0;
}

/*******************************************************************************
* ������ : Second_AT_Command
* ����   : ����ATָ���
* ����   : �������ݵ�ָ�롢���͵ȴ�ʱ��(��λ��S)
* ���   : 
* ����   : 
* ע��   : 
*******************************************************************************/

 void Second_AT_Command(char *b,char *a,u8 wait_time)         
{
	u8 i;
	char *c;
	c = b;										//�����ַ�����ַ��c
	CLR_Buf2(); 
  i = 0;

	
	while(i == 0)                    
	{
		if(!Find(a)) 
		{
			if(Timer0_start == 0)
			{
				b = c;							//���ַ�����ַ��b
				for (; *b!='\0';b++)
				{
					while(USART_GetFlagStatus(USART2, USART_FLAG_TC)==RESET);
					USART_SendData(USART2,*b);//UART2_SendData(*b);
				}
				UART2_SendLR();	//���ͻس�
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


/**********************�����ӣ�2018.01.03***********************************/
/*******************************************************************************
* ������ : Second_AT_Command_1
* ����   : ����ATָ���,���жϷ���ֵ�Ƿ���ȷ��
* ����   : �������ݵ�ָ�롢���͵ȴ�ʱ��(��λ��S)
* ���   : 
* ����   : 
* ע��   : 
*******************************************************************************/

void Second_AT_Command_1(char *b,char *a,u8 wait_time)         
{
	u8 i;
	char *c;
	c = b;										//�����ַ�����ַ��c
	CLR_Buf2(); 
  i = 0;

	
	while(i == 0)                    
	{
		if(!Find(a)) 
		{
			if(Timer0_start == 0)
			{
				b = c;							//���ַ�����ַ��b
				for (; *b!='\0';b++)
				{
					while(USART_GetFlagStatus(USART2, USART_FLAG_TC)==RESET);
					USART_SendData(USART2,*b);//UART2_SendData(*b);
				}
				UART2_SendLR();	//���ͻس�
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
* ������ : Second_AT_Command_2
* ����   : ����ATָ���
* ����   : �������ݵ�ָ�롢���͵ȴ�ʱ��(��λ��S) �����겻���UART2����
* ���   : 
* ����   : 
* ע��   : 
*******************************************************************************/

 void Second_AT_Command_2(char *b,char *a,u8 wait_time)         
{
	u8 i;
	char *c;
	c = b;										//�����ַ�����ַ��c
	CLR_Buf2(); 
  i = 0;
	while(i == 0)                    
	{
		if(!Find(a)) 
		{
			if(Timer0_start == 0)
			{
				b = c;							//���ַ�����ַ��b
				for (; *b!='\0';b++)
				{
					while(USART_GetFlagStatus(USART2, USART_FLAG_TC)==RESET);
					USART_SendData(USART2,*b);//UART2_SendData(*b);
				}
				UART2_SendLR();	//���ͻس�
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

/*******************�¼��޸ĳ���2018.0104***********************************************/
/*******************************************************************************
* ������ : Wait_CREG(�жϷ���ֵ)
* ����   : �ȴ�ģ��ע��ɹ�
* ����   : 
* ���   : ����ֵΪ1��ȷ������ֵΪ0����
* ����   : 
* ע��   : 
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
//				if(Uart2_Buf[k+5] == '1')   //˵��ע��ɹ�
			
					UART1_SendString("���ͳɹ�...*********....***.....");
					UART1_SendString("\r\n");
					
					i = 1;
			}
			else
			{
					UART1_SendString("����ʧ��......���·���");
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
* ������ : Wait_CREG_mode(�жϷ���ֵ)
* ����   : �жϼ����ֵ Ĭ�����Ϊ2000cm
* ����   : 
* ���   : ����ֵΪ1Ϊ������ȷ������ֵΪ0�������
* ����   : 
* ע��   : 
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
				if(Uart2_Buf[k+5] == '1')   //˵��ע��ɹ�
				{	
					threshold = 1000;
					printf("ѡ����ֵΪ1000\r\n");
					i = 1;
				}
				else if(Uart2_Buf[k+5] == '2')
				{
					threshold = 2000;
					printf("ѡ����ֵΪ2000\r\n");
					i = 1;
				}
				else if(Uart2_Buf[k+5] == '3')
				{
					threshold = 3000;
					printf("ѡ����ֵΪ3000\r\n");
					i = 1;
				}
				else
				{
					threshold = 2000;
					printf("ѡ����ֵΪ2000\r\n");
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
* ������ : Set_ATE0
* ����   : ȡ������
* ����   : 
* ���   : 
* ����   : 
* ע��   : 
*******************************************************************************/
void Set_ATE0(void)
{
	CLR_Buf2();
	Second_AT_Command("ATE0","OK",3);								//ȡ������		
}
/*******************************************************************************
* ������ : Connect_Server
* ����   : GPRS���ӷ���������
* ����   : 
* ���   : 
* ����   : 
* ע��   : 
*******************************************************************************/
void Connect_Server(void)
{
	UART2_SendString("AT+CIPCLOSE=1");	//�ر�����
  Delay_nMs(100);
	Second_AT_Command("AT+CIPSHUT","SHUT OK",2);		//�ر��ƶ�����
	Second_AT_Command("AT+CGCLASS=\"B\"","OK",2);//����GPRS�ƶ�̨���ΪB,֧�ְ����������ݽ��� 
	Second_AT_Command("AT+CGDCONT=1,\"IP\",\"CMNET\"","OK",2);//����PDP������,��������Э��,��������Ϣ
	Second_AT_Command("AT+CGATT=1","OK",2);//����GPRSҵ��
	Second_AT_Command("AT+CIPCSGP=1,\"CMNET\"","OK",2);//����ΪGPRS����ģʽ
  Second_AT_Command("AT+CIPMUX=0","OK",2);//����Ϊ��·����
	Second_AT_Command("AT+CIPHEAD=1","OK",2);//���ý���������ʾIPͷ(�����ж�������Դ,���ڵ�·������Ч)
	Second_AT_Command("AT+CIPMODE=1","OK",2);//��͸������
	Second_AT_Command("AT+CIPCCFG=4,5,200,1","OK",2);//����͸��ģʽ�������ط�����:2,���1S����һ��,ÿ�η���200���ֽ�
  //Second_AT_Command((char*)string,"OK",5);//��������
  
  Delay_nMs(100);                                //�ȴ��������ݽ������
  CLR_Buf2();                                    //�崮�ڽ��ջ���Ϊ͸��ģʽ׼��
}

/**************************2018.01.04 open GPRS port and enable GPRS*****************/
void Connect_Server_http_open(void)
{
	Second_AT_Command("AT+SAPBR=3,1,\"Contype\",\"GPRS\"","OK",3);
	//Delay_nS(3);
	UART1_SendString("���ó��س���1\r\n");
	
	Second_AT_Command("AT+SAPBR=3,1,\"APN\",\"CMNET\"","OK",3);
	//Delay_nS(3);
	UART1_SendString("���ó��س���2\r\n");
	
	Second_AT_Command("AT+SAPBR=1,1","O",4);
	//Delay_nS(3);
	UART1_SendString("����һ��GPRS������\r\n");
	
//	Second_AT_Command("AT+SAPBR=1,1","O",4);
//	//Delay_nS(3);
//	UART1_SendString("����һ��GPRS������\r\n");
	
	Second_AT_Command("AT+HTTPINIT","O",3);
	//Delay_nS(3);
	UART1_SendString("��ʼ��HTTP����\r\n");
	
}
void Connect_Server_http_close(void)   
{
	Second_AT_Command("AT+HTTPTERM","OK",3);
	//Delay_nS(1);
	UART1_SendString("����HTTP����������\r\n");
	
	Second_AT_Command("AT+SAPBR=0,1","OK",3);
	//Delay_nS(1);
	UART1_SendString("�ر���������\r\n");
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
	UART1_SendString("����״̬���棬ѡ����ֵ\r\n");
	
	Second_AT_Command("AT+HTTPACTION=0","OK",4);
	UART1_SendString("GET�Ự��ʼ\r\n");
	
	Second_AT_Command("AT+HTTPREAD","OK",4);
	
	Second_AT_Command_2("AT+HTTPREAD","OK", 4);  
	UART1_SendString("��ȡ����ֵ,��ɷ��͹���\r\n");

}

/*******************************************************************************
*������ Connect_Inquire_Number
*������ ��ѯ�绰����
*
*
*******************************************************************************/
void Connect_Inquire_Number(void)
{
	Second_AT_Command_2("AT+CCID","OK",3);
	//Delay_nS(2);
	UART1_SendString("��ѯ�豸ID\r\n");
	//strncpy(id,Uart2_Buf,20);
	
	UART1_SendString(Uart2_Buf);
	strcat(str_status,status);
	strcat(str_status,id);
	UART1_SendString(str_status);
}

/*******************************************************************************
*������ Connect_Server_http
*������ ����httpЭ�����ӷ�����
*
*
*******************************************************************************/
void Connect_Server_http(void)
{
	//UART1_SendString(str_report);
	
	Second_AT_Command(str_report,"OK",1);
	//Second_AT_Command("AT+HTTPPARA=\"URL\",\"http://api.jreader.net:9980/report.php?id=a1&op=-1\"","OK",3);
	//Delay_nS(1);
	UART1_SendString("������վ\r\n");
		
	Second_AT_Command_1("AT+HTTPACTION=0","200",1);
	//Delay_nS(2);
	UART1_SendString("GET�Ự��ʼ\r\n");
	
	
	Second_AT_Command("AT+HTTPREAD","OK", 1);
	//Delay_nS(4);   
	UART1_SendString("��ȡ����ֵ,��ɷ��͹���\r\n");

	CLR_Buf2();
}


void Connect_Server_http_simple(void)
{
	
	Second_AT_Command_1("AT+HTTPACTION=0","0",1);
	//Delay_nS(2);
	UART1_SendString("GET�Ự��ʼ\r\n");
	CLR_Buf2();
}
/*******************************************************************************
* ������ : Rec_Server_Data
* ����   : ���շ��������ݺ���,��ԭ������
* ����   : 
* ���   : 
* ����   : 
* ע��   : 
*******************************************************************************/
void Rec_Server_Data(void)
{
	if(p2!=p1)   		//˵����������δ����
	{	
		while(USART_GetFlagStatus(USART2, USART_FLAG_TC)==RESET);
		USART_SendData(USART2,*p2);
		p2++;
    if(p2>&Uart2_Buf[Buf2_Max])
			p2=Uart2_Buf;
	}
}

/*******************************************************************************
* ������ : Rec_Server_Data
* ����   : ���շ��������ݺ���,��ԭ������
* ����   : 
* ���   : 
* ����   : 
* ע��   : 
*******************************************************************************/
void Rec_Server_Data_3(void)
{
	if(p4!=p3)   		//˵����������δ����
	{	
		while(USART_GetFlagStatus(USART3, USART_FLAG_TC)==RESET);
		USART_SendData(USART3,*p4);
		p4++;
    if(p4>&Uart3_Buf[Buf3_Max])
			p4=Uart3_Buf;
	}
}

/*********************************

����16������

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
