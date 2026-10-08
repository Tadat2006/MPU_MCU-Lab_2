/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

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
TIM_HandleTypeDef htim2;

/* USER CODE BEGIN PV */
/*
 * LAB 2 ALL-IN-ONE DEMO
 * Change DEMO_MODE to 1..10, then Build again.
 * Teacher demo modes: 4, 5, 9, 10.
 */
#define DEMO_MODE           10
#define TIMER_CYCLE_MS      10
#define MATRIX_SCAN_MS      2

/*
 * Matrix polarity for the Proteus MATRIX-8X8-RED + ULN2803 circuit.
 * Default: ROW is active HIGH and ENM is active HIGH (ULN2803 sinks selected COL).
 * If the matrix is inverted on your exact Proteus component, change 1 -> 0.
 */
#define MATRIX_ROW_ACTIVE_HIGH   1
#define MATRIX_ENM_ACTIVE_HIGH   1

const int MAX_LED = 4;
int index_led = 0;
int current_led = 0;
int led_buffer[4] = {1, 2, 3, 4};

volatile int counter_7seg = 0;
volatile int counter_dot = 0;

int hour = 15;
int minute = 8;
int second = 50;

/* Software timers used by Exercises 6-10 */
volatile int timer0_counter = 0;
volatile int timer0_flag = 0;
volatile int timer1_counter = 0;
volatile int timer1_flag = 0;
volatile int timer2_counter = 0;
volatile int timer2_flag = 0;

/* LED Matrix */
const int MAX_LED_MATRIX = 8;
int index_led_matrix = 0;
int matrix_shift_count = 0;

/* Each byte represents one matrix column; each bit represents one row. */
const uint8_t letter_A[8] = {
    0x00,
    0x7E,
    0x11,
    0x11,
    0x11,
    0x7E,
    0x00,
    0x00
};

uint8_t matrix_buffer[8] = {
    0x00,
    0x7E,
    0x11,
    0x11,
    0x11,
    0x7E,
    0x00,
    0x00
};
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_TIM2_Init(void);

/* USER CODE BEGIN PFP */
void display7SEG(int num);
void update7SEG(int index);
void updateClockBuffer(void);

void setTimer0(int duration);
void setTimer1(int duration);
void setTimer2(int duration);
void timer_run(void);

void turnOff7SEG(void);
void turnOffMatrix(void);

void writeMatrixRows(uint8_t data);
void updateLEDMatrix(int index);
void matrixScanTask(void);
void loadLetterA(void);
void shiftMatrixLeft(void);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void setTimer0(int duration)
{
    timer0_counter = duration / TIMER_CYCLE_MS;
    timer0_flag = 0;
}

void setTimer1(int duration)
{
    timer1_counter = duration / TIMER_CYCLE_MS;
    timer1_flag = 0;
}

void setTimer2(int duration)
{
    timer2_counter = duration / TIMER_CYCLE_MS;
    timer2_flag = 0;
}

void timer_run(void)
{
    if (timer0_counter > 0)
    {
        timer0_counter--;
        if (timer0_counter == 0)
            timer0_flag = 1;
    }

    if (timer1_counter > 0)
    {
        timer1_counter--;
        if (timer1_counter == 0)
            timer1_flag = 1;
    }

    if (timer2_counter > 0)
    {
        timer2_counter--;
        if (timer2_counter == 0)
            timer2_flag = 1;
    }
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

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
  MX_TIM2_Init();
  /* USER CODE BEGIN 2 */
  /* Safe initial state */
  turnOff7SEG();
  turnOffMatrix();

  /* DOT and LED_RED are connected to +3.3 V in the Proteus circuit:
     SET keeps them OFF. */
  HAL_GPIO_WritePin(DOT_GPIO_Port, DOT_Pin, GPIO_PIN_SET);
  HAL_GPIO_WritePin(LED_RED_GPIO_Port, LED_RED_Pin, GPIO_PIN_SET);

#if DEMO_MODE == 1
  /* Ex1: two 7-segment displays, 1 and 2, switching every 500 ms */
  display7SEG(1);
  HAL_GPIO_WritePin(EN0_GPIO_Port, EN0_Pin, GPIO_PIN_RESET);
  current_led = 0;

#elif DEMO_MODE == 2
  /* Ex2: 1,2,3,0 and DOT every second */
  led_buffer[0] = 1;
  led_buffer[1] = 2;
  led_buffer[2] = 3;
  led_buffer[3] = 0;
  update7SEG(0);
  index_led = 1;

#elif DEMO_MODE == 3
  /* Ex3: update7SEG() + led_buffer[] */
  led_buffer[0] = 1;
  led_buffer[1] = 2;
  led_buffer[2] = 3;
  led_buffer[3] = 4;
  update7SEG(0);
  index_led = 1;

#elif DEMO_MODE == 4
  /* Ex4: complete 4-digit scan frequency = 1 Hz
     => one digit every 250 ms */
  led_buffer[0] = 1;
  led_buffer[1] = 2;
  led_buffer[2] = 3;
  led_buffer[3] = 4;
  update7SEG(0);
  index_led = 1;

#elif DEMO_MODE == 5
  /* Ex5: digital clock HH:MM, initially 15:08:50 */
  hour = 15;
  minute = 8;
  second = 50;
  updateClockBuffer();
  update7SEG(0);
  index_led = 1;

#elif DEMO_MODE == 6
  /* Ex6: software timer demo */
  updateClockBuffer();
  update7SEG(0);
  index_led = 1;
  setTimer0(1000);

#elif DEMO_MODE == 7
  /* Ex7: clock update + DOT in main using software timer */
  updateClockBuffer();
  update7SEG(0);
  index_led = 1;
  setTimer0(1000);

#elif DEMO_MODE == 8
  /* Ex8: both clock and 7SEG scanning are moved to main */
  updateClockBuffer();
  setTimer0(1000);
  setTimer1(250);

#elif DEMO_MODE == 9
  /* Ex9: static letter A on the 8x8 LED matrix */
  loadLetterA();

#elif DEMO_MODE == 10
  /* Ex10: letter A shifts left every 500 ms */
  loadLetterA();
  setTimer2(500);
#endif

  /* Start TIM2 only once. TIM2 interrupt period = 10 ms. */
  HAL_TIM_Base_Start_IT(&htim2);
/* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
#if DEMO_MODE == 1
    /* Ex1 is handled in TIM2 callback. */

#elif DEMO_MODE == 2
    /* Ex2 is handled in TIM2 callback. */

#elif DEMO_MODE == 3
    /* Ex3 is handled in TIM2 callback. */

#elif DEMO_MODE == 4
    /* Ex4 is handled in TIM2 callback. */

#elif DEMO_MODE == 5
    /*
     * Ex5 follows the specification: the clock is updated in main
     * with a blocking HAL_Delay(1000), while 7SEG scanning stays in TIM2.
     */
    HAL_Delay(1000);

    second++;

    if (second >= 60)
    {
        second = 0;
        minute++;
    }

    if (minute >= 60)
    {
        minute = 0;
        hour++;
    }

    if (hour >= 24)
    {
        hour = 0;
    }

    updateClockBuffer();

#elif DEMO_MODE == 6
    /* Ex6 software timer test: first toggle after 1 s, then every 2 s. */
    if (timer0_flag == 1)
    {
        HAL_GPIO_TogglePin(LED_RED_GPIO_Port, LED_RED_Pin);
        setTimer0(2000);
    }

#elif DEMO_MODE == 7
    /* Ex7: remove HAL_Delay; update clock and DOT using software timer. */
    if (timer0_flag == 1)
    {
        setTimer0(1000);

        second++;

        if (second >= 60)
        {
            second = 0;
            minute++;
        }

        if (minute >= 60)
        {
            minute = 0;
            hour++;
        }

        if (hour >= 24)
        {
            hour = 0;
        }

        updateClockBuffer();
        HAL_GPIO_TogglePin(DOT_GPIO_Port, DOT_Pin);
    }

#elif DEMO_MODE == 8
    /* Ex8: interrupt only handles software timers. */
    if (timer0_flag == 1)
    {
        setTimer0(1000);

        second++;

        if (second >= 60)
        {
            second = 0;
            minute++;
        }

        if (minute >= 60)
        {
            minute = 0;
            hour++;
        }

        if (hour >= 24)
        {
            hour = 0;
        }

        updateClockBuffer();
        HAL_GPIO_TogglePin(DOT_GPIO_Port, DOT_Pin);
    }

    if (timer1_flag == 1)
    {
        setTimer1(250);

        update7SEG(index_led);
        index_led++;

        if (index_led >= MAX_LED)
            index_led = 0;
    }

#elif DEMO_MODE == 9
    /* Ex9: updateLEDMatrix() is invoked from main as required. */
    matrixScanTask();

#elif DEMO_MODE == 10
    /* Ex10: continuously scan the matrix and update animation every 500 ms. */
    matrixScanTask();

    if (timer2_flag == 1)
    {
        setTimer2(500);
        shiftMatrixLeft();
    }
#endif
  }
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
  * @brief TIM2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM2_Init(void)
{

  /* USER CODE BEGIN TIM2_Init 0 */

  /* USER CODE END TIM2_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 7999;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 9;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, ENM0_Pin|ENM1_Pin|DOT_Pin|LED_RED_Pin
                          |EN0_Pin|EN1_Pin|EN2_Pin|EN3_Pin
                          |ENM2_Pin|ENM3_Pin|ENM4_Pin|ENM5_Pin
                          |ENM6_Pin|ENM7_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, SEG0_Pin|SEG1_Pin|SEG2_Pin|ROW2_Pin
                          |ROW3_Pin|ROW4_Pin|ROW5_Pin|ROW6_Pin
                          |ROW7_Pin|SEG3_Pin|SEG4_Pin|SEG5_Pin
                          |SEG6_Pin|ROW0_Pin|ROW1_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : ENM0_Pin ENM1_Pin DOT_Pin LED_RED_Pin
                           EN0_Pin EN1_Pin EN2_Pin EN3_Pin
                           ENM2_Pin ENM3_Pin ENM4_Pin ENM5_Pin
                           ENM6_Pin ENM7_Pin */
  GPIO_InitStruct.Pin = ENM0_Pin|ENM1_Pin|DOT_Pin|LED_RED_Pin
                          |EN0_Pin|EN1_Pin|EN2_Pin|EN3_Pin
                          |ENM2_Pin|ENM3_Pin|ENM4_Pin|ENM5_Pin
                          |ENM6_Pin|ENM7_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : SEG0_Pin SEG1_Pin SEG2_Pin ROW2_Pin
                           ROW3_Pin ROW4_Pin ROW5_Pin ROW6_Pin
                           ROW7_Pin SEG3_Pin SEG4_Pin SEG5_Pin
                           SEG6_Pin ROW0_Pin ROW1_Pin */
  GPIO_InitStruct.Pin = SEG0_Pin|SEG1_Pin|SEG2_Pin|ROW2_Pin
                          |ROW3_Pin|ROW4_Pin|ROW5_Pin|ROW6_Pin
                          |ROW7_Pin|SEG3_Pin|SEG4_Pin|SEG5_Pin
                          |SEG6_Pin|ROW0_Pin|ROW1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */
void display7SEG(int num)
{
    static const uint8_t digit[10] = {
        0x3F, 0x06, 0x5B, 0x4F, 0x66,
        0x6D, 0x7D, 0x07, 0x7F, 0x6F
    };

    uint16_t segPins[7] = {
        SEG0_Pin, SEG1_Pin, SEG2_Pin, SEG3_Pin,
        SEG4_Pin, SEG5_Pin, SEG6_Pin
    };

    if (num < 0 || num > 9)
        return;

    for (int i = 0; i < 7; i++)
    {
        /* Common-anode 7SEG: LOW = segment ON */
        HAL_GPIO_WritePin(
            GPIOB,
            segPins[i],
            (digit[num] & (1 << i)) ? GPIO_PIN_RESET : GPIO_PIN_SET
        );
    }
}

void turnOff7SEG(void)
{
    /* PNP digit enable is active LOW. */
    HAL_GPIO_WritePin(
        GPIOA,
        EN0_Pin | EN1_Pin | EN2_Pin | EN3_Pin,
        GPIO_PIN_SET
    );
}

void update7SEG(int index)
{
    if (index < 0 || index >= MAX_LED)
        return;

    turnOff7SEG();
    display7SEG(led_buffer[index]);

    switch (index)
    {
        case 0:
            HAL_GPIO_WritePin(EN0_GPIO_Port, EN0_Pin, GPIO_PIN_RESET);
            break;
        case 1:
            HAL_GPIO_WritePin(EN1_GPIO_Port, EN1_Pin, GPIO_PIN_RESET);
            break;
        case 2:
            HAL_GPIO_WritePin(EN2_GPIO_Port, EN2_Pin, GPIO_PIN_RESET);
            break;
        case 3:
            HAL_GPIO_WritePin(EN3_GPIO_Port, EN3_Pin, GPIO_PIN_RESET);
            break;
        default:
            break;
    }
}

void updateClockBuffer(void)
{
    led_buffer[0] = hour / 10;
    led_buffer[1] = hour % 10;
    led_buffer[2] = minute / 10;
    led_buffer[3] = minute % 10;
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance != TIM2)
        return;

#if DEMO_MODE == 1
    /* Ex1: switch between display 1 and 2 every 500 ms. */
    counter_7seg++;

    if (counter_7seg >= 50)
    {
        counter_7seg = 0;
        turnOff7SEG();

        if (current_led == 0)
        {
            display7SEG(2);
            HAL_GPIO_WritePin(EN1_GPIO_Port, EN1_Pin, GPIO_PIN_RESET);
            current_led = 1;
        }
        else
        {
            display7SEG(1);
            HAL_GPIO_WritePin(EN0_GPIO_Port, EN0_Pin, GPIO_PIN_RESET);
            current_led = 0;
        }
    }

#elif DEMO_MODE == 2
    /* Ex2: switch digits every 500 ms; DOT toggles every second. */
    counter_7seg++;
    counter_dot++;

    if (counter_7seg >= 50)
    {
        counter_7seg = 0;

        update7SEG(index_led);
        index_led++;

        if (index_led >= MAX_LED)
            index_led = 0;
    }

    if (counter_dot >= 100)
    {
        counter_dot = 0;
        HAL_GPIO_TogglePin(DOT_GPIO_Port, DOT_Pin);
    }

#elif DEMO_MODE == 3
    /* Ex3: update7SEG() is invoked in the timer interrupt. */
    counter_7seg++;
    counter_dot++;

    if (counter_7seg >= 50)
    {
        counter_7seg = 0;

        update7SEG(index_led);
        index_led++;

        if (index_led >= MAX_LED)
            index_led = 0;
    }

    if (counter_dot >= 100)
    {
        counter_dot = 0;
        HAL_GPIO_TogglePin(DOT_GPIO_Port, DOT_Pin);
    }

#elif DEMO_MODE == 4 || DEMO_MODE == 5
    /*
     * Ex4/Ex5: scan one digit every 250 ms.
     * Full scan period = 4 x 250 ms = 1 s => 1 Hz.
     */
    counter_7seg++;
    counter_dot++;

    if (counter_7seg >= 25)
    {
        counter_7seg = 0;

        update7SEG(index_led);
        index_led++;

        if (index_led >= MAX_LED)
            index_led = 0;
    }

    if (counter_dot >= 100)
    {
        counter_dot = 0;
        HAL_GPIO_TogglePin(DOT_GPIO_Port, DOT_Pin);
    }

#elif DEMO_MODE == 6
    /* Ex6 keeps the previous 7SEG processing and adds software timer. */
    timer_run();

    counter_7seg++;
    counter_dot++;

    if (counter_7seg >= 25)
    {
        counter_7seg = 0;

        update7SEG(index_led);
        index_led++;

        if (index_led >= MAX_LED)
            index_led = 0;
    }

    if (counter_dot >= 100)
    {
        counter_dot = 0;
        HAL_GPIO_TogglePin(DOT_GPIO_Port, DOT_Pin);
    }

#elif DEMO_MODE == 7
    /* Ex7: software clock in main, but update7SEG still in interrupt. */
    timer_run();

    counter_7seg++;

    if (counter_7seg >= 25)
    {
        counter_7seg = 0;

        update7SEG(index_led);
        index_led++;

        if (index_led >= MAX_LED)
            index_led = 0;
    }

#else
    /*
     * Ex8-10: interrupt only handles software timers.
     * All application processing is performed in main.
     */
    timer_run();
#endif
}

#if MATRIX_ROW_ACTIVE_HIGH
#define MATRIX_ROW_ON      GPIO_PIN_SET
#define MATRIX_ROW_OFF     GPIO_PIN_RESET
#else
#define MATRIX_ROW_ON      GPIO_PIN_RESET
#define MATRIX_ROW_OFF     GPIO_PIN_SET
#endif

#if MATRIX_ENM_ACTIVE_HIGH
#define MATRIX_ENM_ON      GPIO_PIN_SET
#define MATRIX_ENM_OFF     GPIO_PIN_RESET
#else
#define MATRIX_ENM_ON      GPIO_PIN_RESET
#define MATRIX_ENM_OFF     GPIO_PIN_SET
#endif

void turnOffMatrix(void)
{
    HAL_GPIO_WritePin(
        GPIOA,
        ENM0_Pin | ENM1_Pin | ENM2_Pin | ENM3_Pin |
        ENM4_Pin | ENM5_Pin | ENM6_Pin | ENM7_Pin,
        MATRIX_ENM_OFF
    );
}

void writeMatrixRows(uint8_t data)
{
    uint16_t rowPins[8] = {
        ROW0_Pin, ROW1_Pin, ROW2_Pin, ROW3_Pin,
        ROW4_Pin, ROW5_Pin, ROW6_Pin, ROW7_Pin
    };

    for (int i = 0; i < 8; i++)
    {
        HAL_GPIO_WritePin(
            GPIOB,
            rowPins[i],
            (data & (1 << i)) ? MATRIX_ROW_ON : MATRIX_ROW_OFF
        );
    }
}

void updateLEDMatrix(int index)
{
    if (index < 0 || index >= MAX_LED_MATRIX)
        return;

    /* Disable all columns before changing row data to reduce ghosting. */
    turnOffMatrix();

    /* Send the row pattern for the selected column. */
    writeMatrixRows(matrix_buffer[index]);

    switch (index)
    {
        case 0:
            HAL_GPIO_WritePin(ENM0_GPIO_Port, ENM0_Pin, MATRIX_ENM_ON);
            break;
        case 1:
            HAL_GPIO_WritePin(ENM1_GPIO_Port, ENM1_Pin, MATRIX_ENM_ON);
            break;
        case 2:
            HAL_GPIO_WritePin(ENM2_GPIO_Port, ENM2_Pin, MATRIX_ENM_ON);
            break;
        case 3:
            HAL_GPIO_WritePin(ENM3_GPIO_Port, ENM3_Pin, MATRIX_ENM_ON);
            break;
        case 4:
            HAL_GPIO_WritePin(ENM4_GPIO_Port, ENM4_Pin, MATRIX_ENM_ON);
            break;
        case 5:
            HAL_GPIO_WritePin(ENM5_GPIO_Port, ENM5_Pin, MATRIX_ENM_ON);
            break;
        case 6:
            HAL_GPIO_WritePin(ENM6_GPIO_Port, ENM6_Pin, MATRIX_ENM_ON);
            break;
        case 7:
            HAL_GPIO_WritePin(ENM7_GPIO_Port, ENM7_Pin, MATRIX_ENM_ON);
            break;
        default:
            break;
    }
}

void matrixScanTask(void)
{
    static uint32_t last_scan = 0;

    if ((HAL_GetTick() - last_scan) >= MATRIX_SCAN_MS)
    {
        last_scan = HAL_GetTick();

        updateLEDMatrix(index_led_matrix);

        index_led_matrix++;

        if (index_led_matrix >= MAX_LED_MATRIX)
            index_led_matrix = 0;
    }
}

void loadLetterA(void)
{
    for (int i = 0; i < MAX_LED_MATRIX; i++)
        matrix_buffer[i] = letter_A[i];

    index_led_matrix = 0;
    matrix_shift_count = 0;
}

void shiftMatrixLeft(void)
{
    for (int i = 0; i < MAX_LED_MATRIX - 1; i++)
        matrix_buffer[i] = matrix_buffer[i + 1];

    matrix_buffer[MAX_LED_MATRIX - 1] = 0x00;
    matrix_shift_count++;

    /* Reload A after it has shifted completely out of the matrix. */
    if (matrix_shift_count >= MAX_LED_MATRIX)
        loadLetterA();
}
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
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

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
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

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
