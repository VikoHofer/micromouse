/*************************************************************
Project		: Micromouse V1.0
File		: Uart.c
Description	: Configurate UART for debugging
Date		: 09.09.2024
*************************************************************/


#include "Uart.h"

/**************************
- Using UART6
- PC6 -> Uart6 TX
- PC7 -> Uart6 RX
- GPIOD AF8
- Baud: 115200
- Parity: No
- Stop Bits: 1
**************************/

// UART handle declaration
UART_HandleTypeDef huart6;


// UART BUFFER
static volatile uint8_t UART_RxBuf[RX_BUFFER_SIZE];
static volatile uint32_t UART_RxHead = 0;
static volatile uint32_t UART_RxTail = 0;

static volatile uint8_t UART_TxBuf[TX_BUFFER_SIZE];
static volatile uint32_t UART_TxHead = 0;
static volatile uint32_t UART_TxTail = 0;


// UART GPIO configuration
static void GPIO_Init_Uart(){

    GPIO_InitTypeDef GPIO_InitStruct;
	
    // Enable GPIO Port A Clock
    __HAL_RCC_GPIOC_CLK_ENABLE();
	
    // Configure UART TX and RX Pin
    GPIO_InitStruct.Pin = GPIO_PIN_6 | GPIO_PIN_7;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF8_USART6;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
	
}

void Uart6_Init(void)
{
	// GPIO Init
	GPIO_Init_Uart();
	
	// CLK enable
	__HAL_RCC_USART6_CLK_ENABLE();
	
	// UART configuration structure
    huart6.Instance = USART6;
    huart6.Init.BaudRate = 115200;
    huart6.Init.WordLength = UART_WORDLENGTH_8B;
    huart6.Init.StopBits = UART_STOPBITS_1;
    huart6.Init.Parity = UART_PARITY_NONE;
    huart6.Init.Mode = UART_MODE_TX_RX;
    huart6.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart6.Init.OverSampling = UART_OVERSAMPLING_16;

    // Initialize UART peripheral
    HAL_UART_Init(&huart6);
	
	UART_RxHead = 0;
    UART_RxTail = 0;
    UART_TxHead = 0;
    UART_TxTail = 0;

    // Enable UART interrupt
    HAL_NVIC_SetPriority(USART6_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(USART6_IRQn);

    // Enable the RX and TX interrupts
    __HAL_UART_ENABLE_IT(&huart6, UART_IT_RXNE);
    __HAL_UART_ENABLE_IT(&huart6, UART_IT_TXE);
}
	
	
// Data: pointer of data array
// Size: Data Size
void Uart6_SendData(uint8_t *Data, uint16_t size) {
    for (uint16_t i = 0; i < size; i++) {
        Uart6_PutChar(Data[i]);  // Sende jedes Zeichen einzeln über den Ringpuffer
    }
}


// send a single byte
void Uart6_PutChar(uint8_t byte) {
    uint32_t tmphead = (UART_TxHead + 1) & UART_TX_BUFFER_MASK;

    while (tmphead == UART_TxTail) {
        // Wait until there is space in the buffer
    }

    UART_TxBuf[tmphead] = byte;
    UART_TxHead = tmphead;

    __HAL_UART_ENABLE_IT(&huart6, UART_IT_TXE);  // Enable TX interrupt
}


// 
void Uart6_PutStr(char *str) {
    while (*str) {
        Uart6_PutChar((uint8_t)*str++);
    }
}

int32_t Uart6_GetChar(void) {
    if (UART_RxHead == UART_RxTail) {
        return NO_DATA_IN_RX_BUFFER;  // No data available
    }

    uint32_t tmptail = (UART_RxTail + 1) & UART_RX_BUFFER_MASK;
    UART_RxTail = tmptail;

    return UART_RxBuf[tmptail];
}

void USART6_IRQHandler(void) {
	
    uint32_t isrflags = USART6->SR;

    // Handle received data
    if (isrflags & USART_SR_RXNE) {
        uint8_t received_byte = (uint8_t)(USART6->DR & 0xFF);
        uint32_t tmphead = (UART_RxHead + 1) & UART_RX_BUFFER_MASK;

        if (tmphead != UART_RxTail) {
            UART_RxHead = tmphead;
            UART_RxBuf[tmphead] = received_byte;
        }
    }

    // Handle data transmission
    if (isrflags & USART_SR_TXE) {
        if (UART_TxHead != UART_TxTail) {
            uint32_t tmptail = (UART_TxTail + 1) & UART_TX_BUFFER_MASK;
            UART_TxTail = tmptail;

            USART6->DR = UART_TxBuf[tmptail];
        } else {
            __HAL_UART_DISABLE_IT(&huart6, UART_IT_TXE);  // Disable TX interrupt
        }
    }

}