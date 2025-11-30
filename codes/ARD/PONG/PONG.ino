#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "pitches.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include <freertos/semphr.h>
#include <freertos/event_groups.h>

// --- DEFINICIONES ---
#define SCREEN_WIDTH 128 // Estandar es 128, no 127
#define SCREEN_HEIGHT 64 // Estandar es 64, no 63
#define OLED_RESET   -1
#define SCREEN_ADDRESS 0x3C

#define BUTTON_PIN 13 // CAMBIADO AL 13 (PullUp interno seguro)
#define POT1_LEFT_PIN 34
#define POT2_RIGHT_PIN 35
#define BUZZER_PIN 25

// --- OBJETOS ---
Adafruit_SSD1306 Oled(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// --- BITS DE EVENTOS ---
#define BIT_SOUND_START       (1 << 0)
#define BIT_SOUND_PADDLE_HIT  (1 << 1)
#define BIT_SOUND_HIT_WALL    (1 << 2)
#define BIT_SOUND_SCORE       (1 << 3)
#define BIT_SOUND_MENU        (1 << 4)
// Eliminamos BIT_SOUND_IDLE, no es necesario gastar CPU enviando silencio.

const int velocity = 2; // Aumenté un poco la velocidad para que sea divertido

// --- HANDLERS RTOS ---
EventGroupHandle_t eventGroupSound;
SemaphoreHandle_t mutexGame;
SemaphoreHandle_t semaphoreButton;

// --- ESTRUCTURAS ---
struct VarGame {
  float ballX, ballY; // Float para movimiento fluido
  int ballRadius;
  float ballSpeedX, ballSpeedY;
  int paddle1Y, paddle2Y;
  int paddle_width, paddle_height;
  int score1, score2;
  bool resetBall, isRunning; // Quitamos isGoal, lo manejamos localmente
} Game;

struct InputData {
  int pot1Value, pot2Value;
} input;

// --- PROTOTIPOS ---
void IRAM_ATTR ISR_Button();
void ResetBall();
void DrawGameScreen(VarGame localGameCopy);
void DrawMenuScreen(int score1, int score2);

void Task_InputData(void *pvParameters);
void Task_GameLogic(void *pvParameters);
void Task_Display(void *pvParameters);
void Task_SoundEffects(void *pvParameters);

// ================= SETUP =================
void setup() 
{
  Serial.begin(115200);
  
  // I2C y Pantalla
  if(!Oled.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) 
  {
    Serial.println(F("Fallo OLED"));
  }
  Oled.clearDisplay();
  Oled.display();

  // Pines
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(POT1_LEFT_PIN, INPUT);
  pinMode(POT2_RIGHT_PIN, INPUT);

  // RTOS Tools
  mutexGame = xSemaphoreCreateMutex();
  semaphoreButton = xSemaphoreCreateBinary();
  eventGroupSound = xEventGroupCreate();
  
  // Interrupción
  attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), ISR_Button, FALLING);

  // Inicializar Variables
  Game.isRunning = false;
  Game.resetBall = true;
  Game.ballRadius = 2;
  Game.paddle_height = 12; // Un poco más grande para jugabilidad
  Game.paddle_width = 3;
  Game.score1 = 0;
  Game.score2 = 0;
  
  // Crear Tareas
  xTaskCreate(Task_InputData, "Input", 4096, NULL, 3, NULL);
  xTaskCreate(Task_GameLogic, "Logic", 6000, NULL, 4, NULL);
  xTaskCreate(Task_Display, "Display", 8000, NULL, 2, NULL);
  xTaskCreate(Task_SoundEffects, "Sound", 4096, NULL, 1, NULL);
}

void loop() { vTaskDelete(NULL); }

// ================= ISR =================
void IRAM_ATTR ISR_Button() 
{ 
  static unsigned long last_interrupt_time = 0;
  unsigned long interrupt_time = millis();
  
  if (interrupt_time - last_interrupt_time > 250) 
  {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    xSemaphoreGiveFromISR(semaphoreButton, &xHigherPriorityTaskWoken);
    if (xHigherPriorityTaskWoken) 
    {
      portYIELD_FROM_ISR();
    }
    last_interrupt_time = interrupt_time;
  }
}

// ================= FUNCIONES AUXILIARES =================
// NOTA: Esta función ahora asume que YA TIENES EL MUTEX tomado
void ResetBall() 
{
  Game.ballX = SCREEN_WIDTH / 2;
  Game.ballY = SCREEN_HEIGHT / 2;
  // Random direction
  Game.ballSpeedX = (random(0,2) == 0) ? velocity : -velocity;
  Game.ballSpeedY = (random(0,2) == 0) ? velocity : -velocity;
  Game.resetBall = false;
}

void DrawGameScreen(VarGame local) 
{
  Oled.fillCircle((int)local.ballX, (int)local.ballY, local.ballRadius, SSD1306_WHITE);
  Oled.fillRect(2, local.paddle1Y, local.paddle_width, local.paddle_height, SSD1306_WHITE);
  Oled.fillRect(SCREEN_WIDTH - local.paddle_width - 2, local.paddle2Y, local.paddle_width, local.paddle_height, SSD1306_WHITE);
  // Linea punteada
  for(int i=0; i<SCREEN_HEIGHT; i+=4) Oled.drawPixel(SCREEN_WIDTH/2, i, SSD1306_WHITE);
  Oled.setTextSize(1);
  Oled.setCursor((SCREEN_WIDTH / 2) - 20, 2); Oled.print(local.score1);
  Oled.setCursor((SCREEN_WIDTH / 2) + 15, 2); Oled.print(local.score2);
}

void DrawMenuScreen(int s1, int s2) 
{
  Oled.drawRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, SSD1306_WHITE);
  Oled.setCursor(35, 15); 
  Oled.setTextSize(1); 
  Oled.print(F("PONG RTOS"));
  Oled.setCursor(20, 35); 
  Oled.print(F("Score: ")); 
  Oled.print(s1); Oled.print("-"); 
  Oled.print(s2);
  Oled.setCursor(25, 50); 
  Oled.print(F("[PRESS BTN]"));
}

// ================= TAREAS =================

void Task_InputData(void *pvParameters) 
{
  while(1) 
  {
    int raw1 = analogRead(POT1_LEFT_PIN);
    int raw2 = analogRead(POT2_RIGHT_PIN);
    if(xSemaphoreTake(mutexGame, pdMS_TO_TICKS(10)) == pdTRUE) 
    {
      // Mapeo seguro
      Game.paddle1Y = map(raw1, 0, 4095, 0, SCREEN_HEIGHT - Game.paddle_height);
      Game.paddle2Y = map(raw2, 0, 4095, 0, SCREEN_HEIGHT - Game.paddle_height);
      xSemaphoreGive(mutexGame);
    }
    vTaskDelay(pdMS_TO_TICKS(50));
  }
}

void Task_GameLogic(void *pvParameters) 
{
  bool localIsRunning = false; // Importante inicializar
  while(1) 
  {
    if(xSemaphoreTake(semaphoreButton, 0) == pdTRUE) 
    {
      localIsRunning = !localIsRunning;
      // Tomamos Mutex para actualizar estado global de forma segura
      if (xSemaphoreTake(mutexGame, portMAX_DELAY)) 
      {
        Game.isRunning = localIsRunning;
        
        if(localIsRunning) 
        {
          ResetBall(); // Reseteamos al entrar
          Game.score1 = 0; Game.score2 = 0;
          xEventGroupSetBits(eventGroupSound, BIT_SOUND_START);
        } else {
          xEventGroupSetBits(eventGroupSound, BIT_SOUND_MENU);
        }
        xSemaphoreGive(mutexGame);
      }
    }
    if(localIsRunning) 
    {
      if(xSemaphoreTake(mutexGame, portMAX_DELAY) == pdTRUE) 
      {
        // Manejo de Gol (Reset diferido)
        if(Game.resetBall) 
        {
           ResetBall();
           xSemaphoreGive(mutexGame);
           vTaskDelay(pdMS_TO_TICKS(10));
           continue; 
        }
        // Mover pelota
        Game.ballX += Game.ballSpeedX;
        Game.ballY += Game.ballSpeedY;
        // Rebote Paredes Y
        if(Game.ballY - Game.ballRadius <= 0) 
        {
           Game.ballSpeedY = abs(Game.ballSpeedY); // Bajar
           Game.ballY = Game.ballRadius;
           xEventGroupSetBits(eventGroupSound, BIT_SOUND_HIT_WALL);
        } 
        else if(Game.ballY + Game.ballRadius >= SCREEN_HEIGHT) 
        {
           Game.ballSpeedY = -abs(Game.ballSpeedY); // Subir
           Game.ballY = SCREEN_HEIGHT - Game.ballRadius;
           xEventGroupSetBits(eventGroupSound, BIT_SOUND_HIT_WALL);
        }
        // Rebote Raqueta 1 (Izquierda)
        if(Game.ballX - Game.ballRadius <= 5) 
        {
          if((Game.ballY >= Game.paddle1Y) && (Game.ballY <= Game.paddle1Y + Game.paddle_height)) 
          {
            Game.ballSpeedX = abs(Game.ballSpeedX); // Ir derecha
            Game.ballX = 5 + Game.ballRadius + 1;
            xEventGroupSetBits(eventGroupSound, BIT_SOUND_PADDLE_HIT);
          }          
        }
        // Rebote Raqueta 2 (Derecha) - CORREGIDO EL BUG DE COPY-PASTE
        if(Game.ballX + Game.ballRadius >= SCREEN_WIDTH - 5) 
        {
          // AQUI ESTABA EL ERROR: Usabas paddle1Y en vez de paddle2Y
          if((Game.ballY >= Game.paddle2Y) && (Game.ballY <= Game.paddle2Y + Game.paddle_height)) 
          {
            Game.ballSpeedX = -abs(Game.ballSpeedX); // Ir izquierda
            Game.ballX = SCREEN_WIDTH - 5 - Game.ballRadius - 1;
            xEventGroupSetBits(eventGroupSound, BIT_SOUND_PADDLE_HIT);
          }
        }
        // Gol (Salida de pantalla)
        if(Game.ballX < 2 || Game.ballX > SCREEN_WIDTH - 2) 
        {
           Game.resetBall = true;
           if(Game.ballX < 2) 
           {
            Game.score2++;
           }
           else 
           {
            Game.score1++;
           }
           xEventGroupSetBits(eventGroupSound, BIT_SOUND_SCORE);
        }
        xSemaphoreGive(mutexGame);
      }
    }
    vTaskDelay(pdMS_TO_TICKS(30)); // 100Hz Logic Rate
  }
}

void Task_Display(void *pvParameters) 
{
  VarGame localCopy;
  int s1 = 0, s2 = 0;
  while(1)
  {
    // Copia segura
    if(xSemaphoreTake(mutexGame, portMAX_DELAY) == pdTRUE) 
    {
      localCopy = Game;
      // Guardamos scores aparte por si acaso estamos en menú
      s1 = Game.score1; 
      s2 = Game.score2;
      xSemaphoreGive(mutexGame);    
    }
    Oled.clearDisplay();
    if(localCopy.isRunning) 
    {
      DrawGameScreen(localCopy);
    } else {
      DrawMenuScreen(s1, s2);
    }
    Oled.display();
    vTaskDelay(pdMS_TO_TICKS(20)); // ~33 FPS
  }
}

void Task_SoundEffects(void *pvParameters) 
{
  EventBits_t bits;
  while(1) 
  {
    // Esperamos evento
    bits = xEventGroupWaitBits(
          eventGroupSound, 
           BIT_SOUND_START | BIT_SOUND_PADDLE_HIT | BIT_SOUND_HIT_WALL | BIT_SOUND_SCORE | BIT_SOUND_MENU,
           pdTRUE, 
           pdFALSE, 
           portMAX_DELAY);
    if(bits & BIT_SOUND_START) 
    {
      tone(BUZZER_PIN, NOTE_B5, 100); 
      //vTaskDelay(pdMS_TO_TICKS(150));
      tone(BUZZER_PIN, NOTE_B5, 200); 
      //vTaskDelay(pdMS_TO_TICKS(200));
    } 
    if(bits & BIT_SOUND_PADDLE_HIT) 
    {
      tone(BUZZER_PIN, NOTE_F3, 80); 
      //vTaskDelay(pdMS_TO_TICKS(80));
      tone(BUZZER_PIN, NOTE_C3, 100); 
      //vTaskDelay(pdMS_TO_TICKS(100));
    }   
    if(bits & BIT_SOUND_HIT_WALL) 
    {
      tone(BUZZER_PIN, NOTE_C4, 60); 
      //vTaskDelay(pdMS_TO_TICKS(60));
      noTone(BUZZER_PIN); // Importante cortar sonido corto
    }    
    if(bits & BIT_SOUND_SCORE) 
    {
      tone(BUZZER_PIN, NOTE_C5, 100); 
      //vTaskDelay(pdMS_TO_TICKS(100));
      tone(BUZZER_PIN, NOTE_E5, 100); 
      //vTaskDelay(pdMS_TO_TICKS(100));
      tone(BUZZER_PIN, NOTE_G5, 100); 
      //vTaskDelay(pdMS_TO_TICKS(100));
      tone(BUZZER_PIN, NOTE_C6, 200); 
      //vTaskDelay(pdMS_TO_TICKS(200));
    }
    if(bits & BIT_SOUND_MENU) 
    {
       // Melodía corta de Game Over / Pause
       tone(BUZZER_PIN, NOTE_G4, 200); 
       //vTaskDelay(pdMS_TO_TICKS(200));
       tone(BUZZER_PIN, NOTE_C4, 400); 
       //vTaskDelay(pdMS_TO_TICKS(400));
    }
    noTone(BUZZER_PIN); // Asegurar silencio al terminar
    vTaskDelay(pdMS_TO_TICKS(10)); // Pequeño respiro
  }
}