#ifndef __USART_H
#define __USART_H

#include "stm32f10x.h"

#define USART1_REC_MAXLEN 200	//最大接收数据长度

void USART1_Init_Config(u32 bound);
void UART1_SendString(char* s);

void USART2_Init_Config(u32 bound);
void UART2_SendString(char* s);
void UART1_SendLR(void);
void UART2_SendLR(void);
/***************串口3相关函数********************/
void USART3_Init_Config(u32 bound);
void UART3_SendString(char* s);

void uat_putchr(char chr);
void uat_putstr(char *str);


extern u8 rx1_num,tx1_num,tx1_nums,usart1_flag;
extern u8 rx1_dat[50],tx1_dat[50];


//串口1发送回车换行
#define UART1_SendLR() UART1_SendString("\r\n")

//											 USART_SendData(USART1,0X0D);\
//											 
//											 USART_SendData(USART1,0X0A)
//串口2发送回车换行
#define UART2_SendLR() UART2_SendString("\r\n")
//											 USART_SendData(USART2,0X0D);\
//											 USART_SendData(USART2,0X0A)
//串口3发送回车换行
#define UART3_SendLR() UART3_SendString("\r\n")
#endif


