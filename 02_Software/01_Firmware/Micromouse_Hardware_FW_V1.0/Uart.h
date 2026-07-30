/*************************************************************
Project		: Micromouse V1.0
File		: Uart.h
Description	: Configurate UART for debugging
Date		: 09.09.2024
*************************************************************/

#ifndef UART_H
#define UART_H

#include <stdint.h>
#include "stm32f4xx.h"
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_gpio.h"
#include "stm32f4xx_hal_uart.h"


/**************************
- Using UART6
- PC6 -> Uart6 TX
- PC7 -> Uart6 RX
- GPIOD AF8
- Baud: 115200
- Parity: No
- Stop Bits: 1
**************************/

#define RX_BUFFER_SIZE      256
#define TX_BUFFER_SIZE      256
#define UART_RX_BUFFER_MASK (RX_BUFFER_SIZE - 1)
#define UART_TX_BUFFER_MASK (TX_BUFFER_SIZE - 1)

#define NO_DATA_IN_RX_BUFFER -1


// Init Uart3 
void Uart6_Init(void);


// Send data via Uart3
void Uart6_SendData(uint8_t *Data, uint16_t size);

// Put a single character into the transmit buffer
void Uart6_PutChar(uint8_t byte);

// Send a string via Uart6
void Uart6_PutStr(char *str);

// Get a single character from the receive buffer
int32_t Uart6_GetChar(void);

// Interrupt handler for Uart6
void USART6_IRQHandler(void);


#endif