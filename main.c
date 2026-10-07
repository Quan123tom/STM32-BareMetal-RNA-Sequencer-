/* USER CODE BEGIN Header */
/**
******************************************************************************
* @file : main.c
* @brief : Main program body
******************************************************************************
* @attention
*
* Copyright (c) 2026 STMicroelectronics.
* All rights reserved.
*
* This software is licensed under terms that can be found in the LICENSE file
* in the root directory of this software component.
* If no LICENSE file comes with this software, it is provided AS-IS.
*
******************************************************************************
*/
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
/* USER CODE END Includes */
/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
/* USER CODE END PTD */
/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */
/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
/* USER CODE END PM */
/* Private variables ---------------------------------------------------------*/
UART_HandleTypeDef huart2;
/* USER CODE BEGIN PV */
volatile bool button_pressed_flag = false;
const char sars_cov2_sequence[]=
"ATTAAAGGTTTATACCTTCCCAGGTAACAAACCAACCAACTTTCGATCTCTTGTAGATCTGTTCTCT
AAACGAACTTTAAAATCTGTGTGGCTGTCACTCGGCTGCATGCTTAGTGCACTCACGCAGTATAAT
TAATAACTAATTACTGTCGTTGACAGGACAC"; // στον κωδικα το φορτώσαμε όλο
/* USER CODE END PV */
/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
char get_rna_compl(char c);
/* USER CODE BEGIN PFP */
/* USER CODE END PFP */
/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
/* USER CODE END 0 */
/**
* @brief The application entry point.
* @retval int
*/
int main(void)
{
/* USER CODE BEGIN 1 */
int capacity = 400;
uint8_t rx_byte = 0;
char input_buffer[400];
char search_buffer[400];
int pos_buffer[400];
int match_count = 0;
int counter = 0;
bool is_valid;
/* USER CODE END 1 */
/* MCU Configuration--------------------------------------------------------*/
/* Reset of all peripherals, Initializes the Flash interface and the Systick. */
HAL_Init();
/* USER CODE BEGIN Init */
/* USER CODE END Init */
/* Configure the system clock */
SystemClock_Config();
/* USER CODE BEGIN SysInit */
/* USER CODE END SysInit */
/* Initialize all configured peripherals */
MX_GPIO_Init();
MX_USART2_UART_Init();
/* USER CODE BEGIN 2 */
/* USER CODE END 2 */
/* Infinite loop */
/* USER CODE BEGIN WHILE */
while (1)
{
if (button_pressed_flag == true){
HAL_Delay(3000);
HAL_GPIO_WritePin(GPIOA, GPIO_PIN_13, GPIO_PIN_RESET);
button_pressed_flag = false;
counter = 0;
match_count = 0;
continue;
}
counter = 0;
while (HAL_UART_Receive(&huart2, &rx_byte, 1, HAL_MAX_DELAY) ==
HAL_OK){
char curr = (char)rx_byte;
is_valid = (curr == 'A') || (curr == 'G') || (curr == 'U') || (curr == 'C') || (curr ==
'\n') || (curr == '\r'); // flag για χαρακτήρες
if (is_valid){
if (curr == '\n' || curr == '\r'){
break;
}
else{
input_buffer[counter] = curr;
counter++;
HAL_UART_Transmit(&huart2, &rx_byte, 1,
HAL_MAX_DELAY);
}
}
}
if (counter > 0){ // αν εχω χαρακτήρες
for (int i = 0; i < counter; i++){
search_buffer[i] = get_rna_compl(input_buffer[i]);
}
match_count = 0;
for (int i = 0; i < strlen(sars_cov2_sequence) - counter; i++){
int is_match = 1 ;
for (int j = 0; j <counter; j++){
if (sars_cov2_sequence[i+j] != search_buffer[j]){
is_match = 0;
break;
}
}
if (is_match == 1){
pos_buffer[match_count] = i;
match_count++;
}
}
char tx_buffer[100];
snprintf(tx_buffer, sizeof(tx_buffer), "\r\nMatches found: %d\r\n",
match_count);
HAL_UART_Transmit(&huart2, (uint8_t*)tx_buffer, strlen(tx_buffer), 100);
for (int k = 0; k < match_count; k++){
snprintf(tx_buffer, sizeof(tx_buffer), "-> Found at position: %d\r\n",
pos_buffer[k]);
HAL_UART_Transmit(&huart2, (uint8_t*)tx_buffer,
strlen(tx_buffer), 100);
}
if (match_count > 0){
HAL_GPIO_WritePin(GPIOA, GPIO_PIN_12, GPIO_PIN_SET); //
Turn ON PA12
HAL_Delay(2000);
HAL_GPIO_WritePin(GPIOA, GPIO_PIN_12, GPIO_PIN_RESET);
// Turn OFF PA12
}
}
}
/* USER CODE END WHILE */
/* USER CODE BEGIN 3 */
/* USER CODE END 3 */
}
/**
* @brief System Clock Configuration
* @retval None
*/
void SystemClock_Config(void)
{
RCC_OscInitTypeDef RCC_OscInitStruct = {0};
RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
/** Configure the main internal regulator output voltage
*/
__HAL_RCC_PWR_CLK_ENABLE();
__HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
/** Initializes the RCC Oscillators according to the specified parameters
* in the RCC_OscInitTypeDef structure.
*/
RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
RCC_OscInitStruct.HSIState = RCC_HSI_ON;
RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
{
Error_Handler();
}
/** Initializes the CPU, AHB and APB buses clocks
*/
RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
|RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
{
Error_Handler();
}
}
/**
* @brief USART2 Initialization Function
* @param None
* @retval None
*/
static void MX_USART2_UART_Init(void)
{
/* USER CODE BEGIN USART2_Init 0 */
/* USER CODE END USART2_Init 0 */
/* USER CODE BEGIN USART2_Init 1 */
/* USER CODE END USART2_Init 1 */
huart2.Instance = USART2;
huart2.Init.BaudRate = 115200;
huart2.Init.WordLength = UART_WORDLENGTH_8B;
huart2.Init.StopBits = UART_STOPBITS_1;
huart2.Init.Parity = UART_PARITY_NONE;
huart2.Init.Mode = UART_MODE_TX_RX;
huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
huart2.Init.OverSampling = UART_OVERSAMPLING_16;
if (HAL_UART_Init(&huart2) != HAL_OK)
{
Error_Handler();
}
/* USER CODE BEGIN USART2_Init 2 */
/* USER CODE END USART2_Init 2 */
}
/**
* @brief GPIO Initialization Function
* @param None
* @retval None
*/
//static void MX_GPIO_Init(void)
//{
/* USER CODE BEGIN MX_GPIO_Init_1 */
/* USER CODE END MX_GPIO_Init_1 */
/* GPIO Ports Clock Enable */
//__HAL_RCC_GPIOA_CLK_ENABLE();
/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
//}
static void MX_GPIO_Init(void)
{
GPIO_InitTypeDef GPIO_InitStruct = {0};
__HAL_RCC_GPIOA_CLK_ENABLE();
GPIO_InitStruct.Pin = GPIO_PIN_12 | GPIO_PIN_13;
GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
GPIO_InitStruct.Pull = GPIO_NOPULL;
GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
GPIO_InitStruct.Pin = GPIO_PIN_2 | GPIO_PIN_3;
GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
GPIO_InitStruct.Pull = GPIO_PULLUP;
GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
GPIO_InitStruct.Alternate = GPIO_AF7_USART2;
HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}
/* USER CODE BEGIN 4 */
char get_rna_compl(char c) // συνάρτηση για να μπορούμε να ψάξουμε το σωστό κείμενο (τα
μετατρέπουμε στην σωστή μορφή).
{
switch (c)
{
case 'A':
return 'T';
case 'U':
return 'A';
case 'G':
return 'C';
case 'C':
return 'G';
default:
return '\0';
}
}
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
if (GPIO_Pin == GPIO_PIN_0)
{
button_pressed_flag = true;
HAL_GPIO_WritePin(GPIOA, GPIO_PIN_13, GPIO_PIN_SET);
}
}
/* USER CODE END 4 */
/* USER CODE END 4 */
/**
* @brief This function is executed in case of error occurrence.
* @retval None
*/
void Error_Handler(void)
{
/* USER CODE BEGIN Error_Handler_Debug */
/* User can add his own implementation to report the HAL error return state */
__disable_irq();
while (1)
{
}
/* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
* @brief Reports the name of the source file and the source line number
* where the assert_param error has occurred.
* @param file: pointer to the source file name
* @param line: assert_param error line source number
* @retval None
*/
void assert_failed(uint8_t *file, uint32_t line)
{
/* USER CODE BEGIN 6 */
/* User can add his own implementation to report the file name and line number,
ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
/* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
