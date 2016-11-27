#include "stm32f4_discovery.h"

// PB6 USART1_TX AF
// PB7 USART1_Rx AF

// PD0  DRIVER1 ENABLE
// PD1  DRIVER1 M1
// PD2  DRIVER1 M2
// PD3  DRIVER1 M3
// PD4  DRIVER1 DIRECTION

// PB10 DRIVER2 ENABLE
// PB11 DRIVER2 M1
// PB12 DRIVER2 M2
// PB13 DRIVER2 M3
// PB14 DRIVER2 DIRECTION

// PD13 DRIVER1 STEP
// PD15 DRIVER2 STEP

void Init();
void Delay(volatile long loops);
void SendRequest();
void SetStepsAndDirection(int dx, int dy);

// Previous point
volatile int px;
volatile int py;

// Current point
volatile int x;
volatile int y;

// String ki ga dobi USART
volatile int len;
volatile char* string;

// Steje koliko tock je dobil preko USARTa
volatile int counter;

// Zahtevaj stevilo korakov na motorju (vedno *2 da pošlje 1 0)
volatile int noStepsM1;
volatile int noStepsM2;

int main(void)
{
  Init();
    
  SetStepsAndDirection(2, 5);
  
  while(1);
  
}

// Init functions --------------------------------------------------------------
void Init_USART1(){
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
    
    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
    
    USART_InitTypeDef USART_InitStruct;
    USART_InitStruct.USART_BaudRate = 9600;
    USART_InitStruct.USART_WordLength = USART_WordLength_8b;
    USART_InitStruct.USART_StopBits = USART_StopBits_1;
    USART_InitStruct.USART_Parity = USART_Parity_No;
    USART_InitStruct.USART_HardwareFlowControl=USART_HardwareFlowControl_None;
    USART_InitStruct.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
    USART_Init(USART1, &USART_InitStruct);
  
    USART_Cmd(USART1, ENABLE);
    
    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
}

void Init_GPIO_USART1() {
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
  
  GPIO_InitTypeDef GPIO_InitStruct;
  GPIO_InitStruct.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7; 
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz; 
  GPIO_InitStruct.GPIO_OType = GPIO_OType_PP; 
  GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;
  GPIO_Init(GPIOB, &GPIO_InitStruct);
  
  GPIO_PinAFConfig(GPIOB, GPIO_PinSource6, GPIO_AF_USART1); 
  GPIO_PinAFConfig(GPIOB, GPIO_PinSource7, GPIO_AF_USART1);
}

void Init_GPIO_BUTTON() {
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
    
  EXTI_InitTypeDef EXTI_InitStruct;
  EXTI_InitStruct.EXTI_Line = EXTI_Line0;
  EXTI_InitStruct.EXTI_Mode = EXTI_Mode_Interrupt;
  EXTI_InitStruct.EXTI_Trigger = EXTI_Trigger_Rising;
  EXTI_InitStruct.EXTI_LineCmd = ENABLE;
  EXTI_Init(&EXTI_InitStruct);
  
  NVIC_InitTypeDef NVIC_InitStruct;
  NVIC_InitStruct.NVIC_IRQChannel = EXTI0_IRQn;
  NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStruct.NVIC_IRQChannelSubPriority = 1;
  NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStruct);
  
  GPIO_InitTypeDef GPIO_InitStruct;
  GPIO_InitStruct.GPIO_Pin = GPIO_Pin_0;
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN;
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(GPIOA, &GPIO_InitStruct);
}

void Init_GPIO_LEDS() {
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
  
  GPIO_InitTypeDef GPIO_InitStruct;
  GPIO_InitStruct.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(GPIOD, &GPIO_InitStruct);
  
  GPIO_ResetBits(GPIOD, GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15);
}

void Init_TIM6_DRIVER1() {
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM6, ENABLE);
  
  TIM_TimeBaseInitTypeDef  TIM6_TimeBaseStruct;
  TIM6_TimeBaseStruct.TIM_Period = 10000;
  TIM6_TimeBaseStruct.TIM_Prescaler = 8400-1;   
  TIM6_TimeBaseStruct.TIM_ClockDivision = TIM_CKD_DIV2;
  TIM6_TimeBaseStruct.TIM_CounterMode = TIM_CounterMode_Up;
  TIM_TimeBaseInit(TIM6, &TIM6_TimeBaseStruct);
  
  NVIC_InitTypeDef NVIC_TIM6_InitStruct;
  NVIC_TIM6_InitStruct.NVIC_IRQChannel = TIM6_DAC_IRQn;
  NVIC_TIM6_InitStruct.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_TIM6_InitStruct.NVIC_IRQChannelSubPriority = 1;
  NVIC_TIM6_InitStruct.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_TIM6_InitStruct);

  //TIM_Cmd(TIM6, ENABLE);
  //TIM_ITConfig(TIM6, TIM_IT_Update, ENABLE);
}

void Init_TIM7_DRIVER2() {
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM7, ENABLE);
  
  TIM_TimeBaseInitTypeDef  TIM7_TimeBaseStruct;
  TIM7_TimeBaseStruct.TIM_Period = 10000;
  TIM7_TimeBaseStruct.TIM_Prescaler = 8400-1;   
  TIM7_TimeBaseStruct.TIM_ClockDivision = TIM_CKD_DIV2;
  TIM7_TimeBaseStruct.TIM_CounterMode = TIM_CounterMode_Up;
  TIM_TimeBaseInit(TIM7, &TIM7_TimeBaseStruct);
  
  NVIC_InitTypeDef NVIC_TIM7_InitStruct;
  NVIC_TIM7_InitStruct.NVIC_IRQChannel = TIM7_IRQn;
  NVIC_TIM7_InitStruct.NVIC_IRQChannelPreemptionPriority = 1;
  NVIC_TIM7_InitStruct.NVIC_IRQChannelSubPriority = 2;
  NVIC_TIM7_InitStruct.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_TIM7_InitStruct);

  //TIM_Cmd(TIM7, ENABLE);
  //TIM_ITConfig(TIM7, TIM_IT_Update, ENABLE);
}

void Init_GPIO_DRIVER1() {
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
  
  GPIO_InitTypeDef GPIO_InitStruct;
  GPIO_InitStruct.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4; 
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz; 
  GPIO_InitStruct.GPIO_OType = GPIO_OType_PP; 
  GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(GPIOD, &GPIO_InitStruct);
  
  GPIO_ResetBits(GPIOD, GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4);
}

void Init_GPIO_DRIVER2() {
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
  
  GPIO_InitTypeDef GPIO_InitStruct;
  GPIO_InitStruct.GPIO_Pin = GPIO_Pin_10 | GPIO_Pin_11 | GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14; 
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz; 
  GPIO_InitStruct.GPIO_OType = GPIO_OType_PP; 
  GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(GPIOB, &GPIO_InitStruct);
  
  GPIO_ResetBits(GPIOB, GPIO_Pin_10 | GPIO_Pin_11 | GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14);
}

void Init(){
  noStepsM1 = 0;
  noStepsM2 = 0;
  
  counter = 0;
  px = 0;
  py = 0;
  x = 0;
  y = 0;
  
  len = 1;
  string = (char*)malloc(sizeof(char));
  string[0] = '\0';
  
  Init_GPIO_USART1();
  Init_USART1();
  
  Init_GPIO_LEDS();
  
  Init_GPIO_DRIVER1();
  Init_GPIO_DRIVER2();
  Init_TIM6_DRIVER1();
  Init_TIM7_DRIVER2();
  
  GPIO_ResetBits(GPIOD, GPIO_Pin_1);	// DRIVER1 M1
  GPIO_SetBits(GPIOD, GPIO_Pin_2);		// DRIVER1 M2
  GPIO_ResetBits(GPIOD, GPIO_Pin_3);	// DRIVER1 M3
  GPIO_ResetBits(GPIOD, GPIO_Pin_4);	// DRIVER1 DIRECTION
  GPIO_SetBits(GPIOD, GPIO_Pin_0);		// DRIVER1 ENABLE
  
  GPIO_ResetBits(GPIOB, GPIO_Pin_11);	// DRIVER2 M1
  GPIO_SetBits(GPIOB, GPIO_Pin_12);		// DRIVER2 M2
  GPIO_ResetBits(GPIOB, GPIO_Pin_13);	// DRIVER2 M3
  GPIO_ResetBits(GPIOB, GPIO_Pin_14);	// DRIVER2 DIRECTION
  GPIO_SetBits(GPIOB, GPIO_Pin_10);		// DRIVER2 ENABLE
}

// IRQHandler functions --------------------------------------------------------
void USART1_IRQHandler(void) {
  if(USART_GetITStatus(USART1, USART_IT_RXNE))
  {
    char c = USART_ReceiveData(USART1);
    if(c == '\n' || c == '\r')
    {
      if(counter % 2 == 0)
      {
        px = x;
        x = atoi(string);
      }
      else
      {
        py = y;
        y = atoi(string);
        //SetStepsAndDirection(x-px, y-py);
        SetStepsAndDirection(2, 5);
      }
      len = 1;
      string = (char*)malloc(sizeof(char));
      string[0] = '\0';
      counter++;
    }
    else
    {
      len++;
      string = (char*)realloc(string, len * sizeof(char));
      string[len-2] = c;
      string[len-1] = '\0';
    }
    USART_ClearITPendingBit(USART1, USART_IT_RXNE);
  }
}

void EXTI0_IRQHandler(void) { 
  if(EXTI_GetITStatus(EXTI_Line0)){
    
    EXTI_ClearITPendingBit(EXTI_Line0);
  }
}

void TIM6_DAC_IRQHandler(void) { // DRIVER1
  if(TIM_GetITStatus(TIM6, TIM_IT_Update) && noStepsM1 > 0) {
    noStepsM1--;
    GPIO_ToggleBits(GPIOD, GPIO_Pin_13);
    if(noStepsM1 <= 0){
      TIM_ITConfig(TIM6, TIM_IT_Update, DISABLE);
      TIM_Cmd(TIM6, DISABLE);
    }
    if(noStepsM1 <= 0 && noStepsM2 <= 0){
      SendRequest();
    }
    TIM_ClearITPendingBit(TIM6, TIM_IT_Update);
  }
}

void TIM7_IRQHandler(void) { // DRIVER2
  if(TIM_GetITStatus(TIM7, TIM_IT_Update) && noStepsM2 > 0) {
    noStepsM2--;
    GPIO_ToggleBits(GPIOD, GPIO_Pin_15);
    if(noStepsM2 <= 0){
      TIM_ITConfig(TIM7, TIM_IT_Update, DISABLE);
      TIM_Cmd(TIM7, DISABLE);
    }
    if(noStepsM1 <= 0 && noStepsM2 <= 0){
      SendRequest();
    }
    TIM_ClearITPendingBit(TIM7, TIM_IT_Update);
  }
}

// Helper functions ------------------------------------------------------------
void Delay(volatile long loops) {
  while(loops--);
}

void SendRequest() {
  while(!USART_GetFlagStatus(USART1, USART_FLAG_TXE));
  USART_SendData(USART1, 'R');
  while(!USART_GetFlagStatus(USART1, USART_FLAG_TC));
}

int GetSteps(int pixels){
  return pixels * 2;
}

void SetStepsAndDirection(int dx, int dy){ // TODO: Direction
  if(dx < 0){
    
  }
  if(dy < 0){
    
  }
  dx = abs(dx);
  dy = abs(dy);
  noStepsM1 = GetSteps(dx);
  noStepsM2 = GetSteps(dy);
  TIM_SetCounter(TIM6, 0);
  TIM_SetCounter(TIM7, 0);
  TIM_Cmd(TIM6, ENABLE);
  TIM_Cmd(TIM7, ENABLE);
  TIM_ITConfig(TIM6, TIM_IT_Update, ENABLE);
  TIM_ITConfig(TIM7, TIM_IT_Update, ENABLE);
}