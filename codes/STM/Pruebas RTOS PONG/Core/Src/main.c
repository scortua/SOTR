/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
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
#include "RTOS.h"		// definicion de la libreria del RTOS
#include "AppTypes.h"	// definicion de las variables definidas
#include "ssd1306.h"	// definicion de la pantalla oled
#include "ssd1306_fonts.h" // definicion de fuentes
#include "stdlib.h"		// definicion para variables y absolutos
#include "stdio.h"		// definicion entradas y salidas
#include "string.h"		// definicion de funciones strings
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
typedef struct
{
	float ballX, ballY;					// posicion de la pelota en X y Y
	int ballRadio;						// radio de la pelota, si es circulo, puede ser la mitad del lado del cuadrado
	float ballSpeedX, ballSpeedY;		// velocidad de la pelota en X y Y
	int paddle1Y, paddle2Y;				// posicion de la raqueta izquierda y derecha, en esto se toma en la parte superiror izquierda
	int paddle_width, paddle_height;	// ancho y largo de la raqueta
	int score1, score2;					// marcador de la izquierda y la derecha
	u8 isRunning;						// si esta en ejecucion
	u8 resetBall;						// si se necesita resetear la pelota
}GameState_t;
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define SCREEN_WIDTH 128	// ancho de la pantalla
#define SCREEN_HEIGHT 64	// alto de la pantalla

#define BIT_BTN_PRESSED		( 1 << 0)	// Input del boton

#define BIT_SND_START		(1 << 0)	// bit inicio
#define BIT_SND_PADDLE      (1 << 1)	// bit raqueta
#define BIT_SND_WALL        (1 << 2)	// bit muro
#define BIT_SND_SCORE       (1 << 3)	// bit gol
#define BIT_SND_MENU        (1 << 4)	// bit menu

#define BALL_SPEED		2.0f	// velocidad del juego

#define STACK_SIZE_HIGH		256
#define STACK_SIZE_MEDIUM	128
#define STACK_SIZE_LOW		64

// --- PANTALLA (I2C1) ---
// Definido por hardware en PB6 y PB7, no requiere define manual si usas HAL I2C

// --- ENTRADAS ANALÓGICAS ---
// Canales ADC1
#define POT1_CHANNEL    ADC_CHANNEL_1  // PA1
#define POT2_CHANNEL    ADC_CHANNEL_2  // PA2

// --- BOTÓN (PA0 - Botón KEY integrado) ---
#define BTN_PIN         GPIO_PIN_0
#define BTN_PORT        GPIOA

// --- BUZZER (PB12) ---
#define BUZZER_PIN      GPIO_PIN_12
#define BUZZER_PORT     GPIOB

// --- LED DE LA PLACA (PC13 - Lógica Invertida: LOW=ON) ---
#define LED_PIN         GPIO_PIN_13
#define LED_PORT        GPIOC
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;

I2C_HandleTypeDef hi2c1;

/* USER CODE BEGIN PV */

// Creacion de las tareas
TaskControlBlock_t TcbInput;			// variable de control de tarea input
u32 StackInput[STACK_SIZE_LOW];			// variable de la pila de tarea input
TaskControlBlock_t TcbLogic;			// variable de control de tarea logica
u32 StackLogic[STACK_SIZE_MEDIUM];		// variable de la pila de tarea logica
TaskControlBlock_t TcbDisplay;			// variable de control de tarea display
u32 StackDisplay[STACK_SIZE_HIGH];		// variable de la pila de tarea display
TaskControlBlock_t TcbSound;			// variable de control de tarea sonido
u32 StackSound[STACK_SIZE_LOW];			// variable de la pila de tarea sonido

// Creacion de mutex
MutexHandler_t MutexJuego;				// mutex para manerar variables del juego entre logica input
EventHandler_t EventosSonido;			// eventos de sonido para cada colicion
EventHandler_t EventosInput;			// evento de entrada del boton para cambiar de start a menu

// variable global del juego
GameState_t Game;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_ADC1_Init(void);
static void MX_I2C1_Init(void);
/* USER CODE BEGIN PFP */
// funciones de las tareas
void Tarea_Input(void);					// tarea de entrada
void Tarea_GameLogic(void);				// tarea logica del juego
void Tarea_Display(void);				// tarea del display
void Tarea_Sound(void);					// tarea del sonido

// funciones secundarias
void ResetBall(void);					// funcion de reseteo
void GenerarTono(u32 frecuencia, u32 duracion_ms);
void DrawMenu(int s1, int s2);	// funcion dibujo pantalla menu
void DrawGame(GameState_t *localGame);			// funcion dibujo pantalla jugable
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

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
  ssd1306_Init();
  ssd1306_Fill(Black);
  ssd1306_UpdateScreen();

  Game.isRunning = FALSE;
  Game.resetBall = TRUE;
  Game.ballRadio = 3;
  Game.paddle_height = 12;
  Game.paddle_width = 4;
  Game.score1 = 0;
  Game.score2 = 0;
  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_ADC1_Init();
  MX_I2C1_Init();
  /* USER CODE BEGIN 2 */
  RTOS_Init();
  Mutex_Init(&MutexJuego);
  Events_Init(&EventosSonido);
  Events_Init(&EventosInput);
// tarea()
  Task_CreateTask(&TcbInput,"Tarea entrada de datos", 1, 1, StackInput, STACK_SIZE_LOW, Tarea_Input);
  Task_CreateTask(&TcbLogic,"Tarea logica de juego", 2, 2, StackLogic, STACK_SIZE_MEDIUM, Tarea_GameLogic);
  Task_CreateTask(&TcbDisplay,"Tarea visualizacion display", 3, 3, StackDisplay, STACK_SIZE_HIGH, Tarea_Display);
  Task_CreateTask(&TcbSound,"Tarea de sonido", 4, 4, StackSound, STACK_SIZE_LOW, Tarea_Sound);
  RTOS_Start();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
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

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 64;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */

  /** Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion)
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV2;
  hadc1.Init.Resolution = ADC_RESOLUTION_12B;
  hadc1.Init.ScanConvMode = DISABLE;
  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 1;
  hadc1.Init.DMAContinuousRequests = DISABLE;
  hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
  */
  sConfig.Channel = ADC_CHANNEL_1;
  sConfig.Rank = 1;
  sConfig.SamplingTime = ADC_SAMPLETIME_3CYCLES;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 400000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(BUZZER_GPIO_Port, BUZZER_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : BTN_Pin */
  GPIO_InitStruct.Pin = BTN_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(BTN_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : BUZZER_Pin */
  GPIO_InitStruct.Pin = BUZZER_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(BUZZER_GPIO_Port, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
void Tarea_Input(void)
{
	u32 adc1_val = 0, adc2_val = 0;	// valores de los pots ADC 4096
	GPIO_PinState btnState, lastBtnState = GPIO_PIN_SET;	// variable estado de boton y antirebote debouncing
	u32 lastDebounce = 0;
	while(1)
	{
		// leer los potenciometros
		HAL_ADC_Start(&hadc1);	// abre el primer canal para adc
		if(HAL_ADC_PollForConversion(&hadc1, 10) == HAL_OK)
		{
			adc1_val = HAL_ADC_GetValue(&hadc1);	// primer canal leido
		}
		if(HAL_ADC_PollForConversion(&hadc1, 10) == HAL_OK)
		{
			adc2_val = HAL_ADC_GetValue(&hadc1);	// segundo canal leido
		}
		HAL_ADC_Stop(&hadc1);	// detiene el adc
		// actualizar datos mutex
		if(Mutex_Take(&MutexJuego) == MUTEX_OK)	// espera para acceder al mutex y editar variables globales
		{
			// Mapeo simple: (ValorADC * (Pantalla - AltoPaleta)) / MaxADC
			Game.paddle1Y = (adc1_val * (SCREEN_HEIGHT - Game.paddle_height)) / 4095;
			Game.paddle2Y = (adc2_val * (SCREEN_HEIGHT - Game.paddle_height)) / 4095;
			Mutex_Give(&MutexJuego);	// libera el mutex
		}
		// Lee el estado actual del pin del botón
		btnState = HAL_GPIO_ReadPin(BTN_PORT, BTN_PIN);
		// Verifica si el estado actual es diferente al estado anterior (cambio de flanco)
		if(btnState != lastBtnState)
		{
			// Comprueba si ha pasado suficiente tiempo (50ms) desde el último cambio válido (antirrebote)
			if((Port_GetSystemTick() - lastDebounce) > 50)
			{
				if(btnState == GPIO_PIN_RESET) // Si el estado actual es "presionado" (pull-up)
				{
					Events_Set(&EventosInput, BIT_BTN_PRESSED);	// Señaliza el evento de pulsación de boton a otras Tareas
				}
				lastDebounce = Port_GetSystemTick();	// Actualiza el tick del último cambio valido del antirebote :)
			}
		}
		lastBtnState = btnState;	// Actualiza el estado anterior para la próxima iteracion
		RTOS_Delay(20);
	}
}
void Tarea_GameLogic(void)
{
	u8 localRunning;	// bandera local para evitar el uso de mutex
	EventType_t eventBits;	// almacenar los bits del evento
	while(1)
	{
		Events_Get(&EventosInput, &eventBits);	// revisa el boton para el cambio de juego y menu
		if(eventBits & BIT_BTN_PRESSED)	// ver estado del juego al oprimir el boton
		{
			Events_Clear(&EventosInput, BIT_BTN_PRESSED);	// limpia el bit de evento
			localRunning = !localRunning;	// alterna el estado de la bandera
			if(Mutex_Take(&MutexJuego) == MUTEX_OK)	// espera para tomar el mutex
			{
				Game.isRunning = localRunning;	// sincroniza el estado global
				if(localRunning)	// si se cambia a jugar
				{
					ResetBall();		// acomodar todo desde 0
					Game.score1 = 0;
					Game.score2 = 0;
					Events_Set(&EventosSonido, BIT_SND_START);	// evento de sonido start
				}
				else	// si se cambia a menu
				{
					Events_Set(&EventosSonido, BIT_SND_MENU);	// evento de sonido menu
				}
				Mutex_Give(&MutexJuego);	// suelta el mutex
			}
		}
		// ====================================fisicas=========================================
		if(localRunning)	// si se esta jugando
		{
			if(Mutex_Take(&MutexJuego) == MUTEX_OK)	// tomar el mutex
			{
				if(Game.resetBall)	// si se necesita resetear
				{
					ResetBall();	// resetea juego
					Mutex_Give(&MutexJuego);	// soltar el mutex
					RTOS_Delay(10);
					continue;
				}
				Game.ballX += Game.ballSpeedX;	// movimiento en X y Y
				Game.ballY += Game.ballSpeedY;
				// =================================rebote techo y piso=================================
				if(Game.ballY - Game.ballRadio <= 0 || Game.ballY + Game.ballRadio >= SCREEN_HEIGHT)
				{
					Game.ballSpeedY = (Game.ballSpeedY > 0) ? -BALL_SPEED : BALL_SPEED;	// cambiar la direccion
					Game.ballY = (Game.ballSpeedY > 0) ? SCREEN_HEIGHT-Game.ballRadio : Game.ballRadio;	// cambiar la posicion para no parecer que atravieza la pared
					Events_Set(&EventosSonido, BIT_SND_WALL);
				}
				// =================================rebote izquierda=================================
				if((Game.ballX - Game.ballRadio) <= (2 + Game.paddle_width))
				{
					if((Game.ballY >= Game.paddle1Y) && (Game.ballY <= (Game.paddle1Y + Game.paddle_height)))
					{
						Game.ballSpeedX = BALL_SPEED;	// rebota la pelota
						Game.ballX = (2 + Game.paddle_width) + Game.ballRadio + 1;	// acomoda la posicion
						Events_Set(&EventosSonido, BIT_SND_PADDLE);	// evento de sonido de rebote
					}
				}
				// =================================rebote Derecha=================================
				if((Game.ballX + Game.ballRadio) >= (SCREEN_WIDTH - 2 - Game.paddle_width))
				{
					if((Game.ballY >= Game.paddle2Y) && (Game.ballY <= (Game.paddle2Y + Game.paddle_height)))
					{
						Game.ballSpeedX = -BALL_SPEED;	// rebota la pelota
						Game.ballX = SCREEN_WIDTH - 2 - Game.paddle_width - Game.ballRadio - 1;	// acomoda la posicion
						Events_Set(&EventosSonido, BIT_SND_PADDLE);	// evento de sonido de rebote
					}
				}
				// Goles
				if(Game.ballX < -Game.ballRadio || Game.ballX > SCREEN_WIDTH + Game.ballRadio)	// que se meta la pelota lit
				{
					Game.resetBall = TRUE;	// marca reinicio al centro
					if(Game.ballX < 0)
					{
						Game.score2++;
					}
					else
					{
						Game.score1++;
					}
					Events_Set(&EventosSonido, BIT_SND_SCORE);	// evento de sonido de gol
				}

				Mutex_Give(&MutexJuego);	// libera el mutex
			}
		}
		RTOS_Delay(10);
	}
}
void Tarea_Display(void)
{
	GameState_t localGame;	// libera el mutex usando una copia
	while(1)
	{
		if(Mutex_Take(&MutexJuego) == MUTEX_OK)	// quiere usar el mutex
		{
			memcpy(&localGame, &Game, sizeof(GameState_t));	// destino-funete-bytes para copiar bytes
			Mutex_Give(&MutexJuego);	// libera mutex
		}
		ssd1306_Fill(Black);	// limpiar el buffer - pantalla limpia
		if(localGame.isRunning)	// si esta jugando
		{
			DrawGame(&localGame);	// dibuje el juego
		}
		else
		{
			DrawMenu(localGame.score1, localGame.score2);	// dibuje el menu
		}
		ssd1306_UpdateScreen();	// actualice pantalla
		RTOS_Delay(30);
	}
}
void Tarea_Sound(void)
{
	EventType_t bits;	// guardar los bits de evento activos
	EventType_t mask = BIT_SND_START | BIT_SND_PADDLE | BIT_SND_WALL | BIT_SND_SCORE | BIT_SND_MENU;	// mascara de bits
	while(1)
	{
		// Esperar evento (OR lógico)
		if(Events_WaitAny(&EventosSonido, mask) == EVENT_OK)	// espera que cualquiera de los bits cambie
		{
			Events_Get(&EventosSonido, &bits);	// extrae los bits para saber cual cambio
			if(bits & BIT_SND_START) {
				GenerarTono(1000, 100);
				GenerarTono(1500, 200);
			}
			else if(bits & BIT_SND_SCORE) {
				GenerarTono(523, 100);
				GenerarTono(659, 100);
				GenerarTono(784, 100);
				GenerarTono(1046, 200);
			}
			else if(bits & BIT_SND_PADDLE) {
				GenerarTono(440, 50);
			}
			else if(bits & BIT_SND_WALL) {
				GenerarTono(220, 50);
			}
			else if(bits & BIT_SND_MENU) {
				GenerarTono(300, 150);
				GenerarTono(200, 300);
			}
			Events_Clear(&EventosSonido, mask);	// limpia para otro evento
		}
	}
}

void DrawGame(GameState_t * game)
{
	// Dibujar Bola
	ssd1306_FillCircle((int)game->ballX, (int)game->ballY, game->ballRadio, White);
	// Dibujar raqueta Izquierda
	ssd1306_FillRectangle(2, game->paddle1Y, game->paddle_width, game->paddle_height, White);
	// Dibujar raqueta Derecha
	ssd1306_FillRectangle(SCREEN_WIDTH - 2 - game->paddle_width, game->paddle2Y, game->paddle_width, game->paddle_height, White);
	// Línea Central
	ssd1306_Line(SCREEN_WIDTH / 2, 0, SCREEN_WIDTH / 2, SCREEN_HEIGHT, White);
	// Puntajes (Usando las fuentes de afiskon)
	char buffer[5];
	// Izquierda
	sprintf(buffer, "%d", game->score1);	// String Print Formatted usa buffer para copiar un valor en un formato
	ssd1306_SetCursor(SCREEN_WIDTH/2 - 20, 0);
	ssd1306_WriteString(buffer, Font_7x10, White);
	// Derecha
	sprintf(buffer, "%d", game->score2);
	ssd1306_SetCursor(SCREEN_WIDTH/2 + 10, 0);
	ssd1306_WriteString(buffer, Font_7x10, White);
}

void DrawMenu(int s1, int s2)
{
	ssd1306_SetCursor(15, 10);
	ssd1306_WriteString("PONG RTOS", Font_11x18, White);
	ssd1306_SetCursor(25, 35);
	ssd1306_WriteString("[PRESS BTN]", Font_7x10, White);
	char buf[20];
	sprintf(buf, "LAST: %d - %d", s1, s2);
	ssd1306_SetCursor(25, 50);
	ssd1306_WriteString(buf, Font_7x10, White);
}

void GenerarTono(u32 freq, u32 duration_ms)
{
	if(freq == 0)	// verificar si no hay tono que esperar
	{
		return;
	}
	// ne necesita exponer ondas cuadradas
	u32 period = 1000000 / freq; 		// Periodo en us
	u32 half_period = period / 2;		// ciclo de trabajo onda
	u32 cycles = (duration_ms * 1000) / period;	// numero total de la onda

	// Loop simple para generar onda cuadrada
	// Nota: Esto bloquea la Tarea Sonido (Prio Baja), lo cual está bien.
	// La calibración del loop depende de la velocidad del reloj (100MHz aprox)
	// Ajustar el '6' si suena muy grave o agudo.
	for(u32 i=0; i<cycles; i++)
	{
		HAL_GPIO_WritePin(BUZZER_PORT, BUZZER_PIN, GPIO_PIN_SET);	// pin en alto para iniciar semiperiodo HIGH
		for(volatile int k=0; k<(half_period * 6); k++);	// retraso para dutty

		HAL_GPIO_WritePin(BUZZER_PORT, BUZZER_PIN, GPIO_PIN_RESET);	// pin en bajo para iniciar semiperiodo LOW
		for(volatile int k=0; k<(half_period * 6); k++);	// retraso para dutty
	}
}

void ResetBall(void)
{
	Game.ballX = SCREEN_WIDTH / 2;
	Game.ballY = SCREEN_HEIGHT / 2;
	Game.ballSpeedX = (rand() % 2 == 0) ? BALL_SPEED : -BALL_SPEED;
	Game.ballSpeedY = (rand() % 2 == 0) ? BALL_SPEED : -BALL_SPEED;
	Game.resetBall = FALSE;
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
#ifdef USE_FULL_ASSERT
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
