#include <Arduino.h>
// librerias
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "pitches.h" // notas musicales
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include <freertos/semphr.h>
#include <freertos/event_groups.h>
// pines pantalla oled
#define SCREEN_WIDTH 127
#define SCREEN_HEIGHT 63
#define OLED_RESET   -1
#define SCREEN_ADDRESS 0x3C
// pines de boton
#define BUTTON_PIN 12
// pines potenciometros
#define POT1_LEFT_PIN 34
#define POT2_RIGHT_PIN 35
// pin del buzzer
#define BUZZER_PIN 25
// ------------- Crear objeto display
Adafruit_SSD1306 Oled(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
// ------------------------------------------- variables 
#define BIT_SOUND_START (1 << 0)      // (00000001) sonido de inicio
#define BIT_SOUND_PADDLE_HIT (1 << 1) // (00000010) sonido de rebote con raqueta
#define BIT_SOUND_HIT_WALL (1 << 2)   // (00000100) sonido de rebote con pared
#define BIT_SOUND_SCORE (1 << 3)      // (00001000) sonido de puntuacion
#define BIT_SOUND_IDLE (1 << 4)       // (00010000) sonido de silencio jaja
#define BIT_SOUND_MENU (1 << 5)       // (00100000) sonido de menu
const int velocity = 1;;
EventGroupHandle_t eventGroupSound;
SemaphoreHandle_t mutexGame;
SemaphoreHandle_t semaphoreButton;
// ------------------------------------------- estructuras
struct VarGame
{
  int ballX, ballY;
  int ballRadius;
  int ballSpeedX, ballSpeedY;
  int paddle1Y, paddle2Y;
  int paddle_width, paddle_height;
  int score1, score2;
  bool resetBall, isRunning, isGoal;
}Game;
struct InputData
{
  int pot1Value, pot2Value;
}input;
// ------------------------------------------- funciones
void IRAM_ATTR ISR_Button();
void ResetBall();
void DrawGameScreen(VarGame localGameCopy);
void DrawMenuScreen(int score1, int score2);

void Task_InputData(void *pvParameters);
void Task_GameLogic(void *pvParameters);
void Task_Display(void *pvParameters);
void Task_SoundEffects(void *pvParameters);

void setup() 
{
  // inicial serial
  Serial.begin(115200);
  Serial.println("Iniciando...");
  // inicializar pantalla
  if(!Oled.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("Fallo al iniciar la pantalla SSD1306"));
  }
  Oled.clearDisplay();
  Oled.display();
  // inicializar pines
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(POT1_LEFT_PIN, INPUT);
  pinMode(POT2_RIGHT_PIN, INPUT);
  // tareas freertos
  xTaskCreate(Task_InputData, "Input Data Hardware", 2048, NULL, 2, NULL);
  xTaskCreate(Task_GameLogic, "Game Logic ball and paddles", 4096, NULL, 3, NULL);
  xTaskCreate(Task_Display, "Display Oled", 2048, NULL, 1, NULL);
  xTaskCreate(Task_SoundEffects, "Sound Effects Buzzer", 2048, NULL, 1, NULL);
  // inicializar semaforos, colas, eventos etc
  mutexGame = xSemaphoreCreateMutex();
  semaphoreButton = xSemaphoreCreateBinary();
  eventGroupSound = xEventGroupCreate();
  attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), ISR_Button, FALLING);
  // inicializar variables del juego
  Game.isRunning = false;
  Game.resetBall = true;
  Game.ballRadius = 3;
  Game.paddle_height = 9;
  Game.paddle_width = 2;
  Game.score1 = 0;
  Game.score2 = 0;
  Game.isGoal = false;
}

void loop() 
{
  vTaskDelete(NULL); // para que muera este loop
}

// ===========================================================funciones
/*
Funcion de interrupcion del boton para iniciar y terminar el juego
*/
void IRAM_ATTR ISR_Button()
{ 
  static unsigned long last_interrupt_time = 0;
  unsigned long interrupt_time = millis();  // Debounce de 250ms: Si han pasado menos de 250ms desde la última vez, ignorar.
  if (interrupt_time - last_interrupt_time > 250) {
    // xHigherPriorityTaskWoken es un tecnicismo de FreeRTOS para avisar si debemos 
    // cambiar de tarea inmediatamente al salir de la interrupción.
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    xSemaphoreGiveFromISR(semaphoreButton, &xHigherPriorityTaskWoken);
    // Si la lógica estaba dormida esperando esto, forzamos un cambio de contexto inmediato
    if (xHigherPriorityTaskWoken) {
      portYIELD_FROM_ISR();
    }
    last_interrupt_time = interrupt_time;
  }
}
/*
Reseteo del juego en posiciones iniciales y de otras variables
para iniciar el juego o entrar en el menu
*/
void ResetBall()
{
  if(Game.isGoal)
  {
    Game.ballX = SCREEN_WIDTH / 2;
    Game.ballY = SCREEN_HEIGHT / 2;
    Game.ballSpeedX = (random(0,2) == 0) ? velocity : -velocity;
    Game.ballSpeedY = (random(0,2) == 0) ? velocity : -velocity;
    Game.isGoal = false;
    Game.resetBall = false;
  }
  else
  {
    Game.ballX = SCREEN_WIDTH / 2;
    Game.ballY = SCREEN_HEIGHT / 2;
    Game.ballSpeedX = (random(0,2) == 0) ? velocity : -velocity;
    Game.ballSpeedY = (random(0,2) == 0) ? velocity : -velocity;
    Game.resetBall = false;
    Game.score1 = 0;
    Game.score2 = 0;
  }
}
/*
Dibuja la pantalla de juego en la pantalla oled 
con la estructura de copia local del juego
*/
void DrawGameScreen(VarGame LocalGameCopy)
{
  Oled.fillCircle(LocalGameCopy.ballX, LocalGameCopy.ballY, LocalGameCopy.ballRadius, SSD1306_WHITE);
  Oled.fillRect(2, LocalGameCopy.paddle1Y, LocalGameCopy.paddle_width, LocalGameCopy.paddle_height, SSD1306_WHITE);
  Oled.fillRect(SCREEN_WIDTH - LocalGameCopy.paddle_width - 2, LocalGameCopy.paddle2Y, LocalGameCopy.paddle_width, LocalGameCopy.paddle_height, SSD1306_WHITE);
  Oled.drawLine(SCREEN_WIDTH / 2, 0, SCREEN_WIDTH / 2, SCREEN_HEIGHT, SSD1306_WHITE);
  Oled.setTextSize(2);
  Oled.setTextColor(SSD1306_WHITE);
  Oled.setCursor((SCREEN_WIDTH / 2) -20, 2);
  Oled.print(LocalGameCopy.score1);
  Oled.setCursor((SCREEN_WIDTH / 2) + 10, 2);
  Oled.print(LocalGameCopy.score2);
}
/*
Dibuja la pantalla del menu con los ultimos scores
*/
void DrawMenuScreen(int score1, int score2)
{
  Oled.setTextSize(2);
  Oled.setTextColor(SSD1306_WHITE);
  Oled.drawRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, SSD1306_WHITE);
  Oled.setCursor(10, 10);
  Oled.print(F("PONG RTOS"));
  Oled.setTextSize(1);
  Oled.setCursor(10, 40);
  Oled.print(F("Last Score: "));
  Oled.print(score1);
  Oled.print(" - ");
  Oled.print(score2);
  Oled.setCursor(20, 52);
  Oled.print(F("[PRESS START]"));
}
/*
Tarea encargada de leer los datos de los potenciometros y el boton
para mantener los datos sincronizados entre las otras tareas al manejar
datos de una variable global compartida con GameLogic y Display
*/
void Task_InputData(void *pvParameters)
{
  while(1)
  {
    input.pot1Value = analogRead(POT1_LEFT_PIN);
    input.pot2Value = analogRead(POT2_RIGHT_PIN);
    if(xSemaphoreTake(mutexGame, pdMS_TO_TICKS(10)) == pdTRUE)
    {
    Game.paddle1Y = map(input.pot1Value, 0, 4095, 0, SCREEN_HEIGHT - Game.paddle_height);
    Game.paddle2Y = map(input.pot2Value, 0, 4095, 0, SCREEN_HEIGHT - Game.paddle_height);
    xSemaphoreGive(mutexGame);
    }
    vTaskDelay(pdMS_TO_TICKS(25));
  }
}

void Task_GameLogic(void *pvParameters)
{
  bool localIsRunning;
  while(1)
  {
    if(xSemaphoreTake(semaphoreButton, 0) == pdTRUE)
    {
      localIsRunning = !localIsRunning;
      if(localIsRunning)
      {
        // menu a juego
        ResetBall();
        xEventGroupSetBits(eventGroupSound, BIT_SOUND_START);
      }
      else
      {
        // juego a menu
        ResetBall();
        xEventGroupSetBits(eventGroupSound, BIT_SOUND_MENU);
      }
      // ACTUALIZA ESTADO GLOBAL (Para que Display se entere)
      if (xSemaphoreTake(mutexGame, portMAX_DELAY)) {
        Game.isRunning = localIsRunning;
        xSemaphoreGive(mutexGame);
      }
    }
    // juego en marcha
    if(localIsRunning)
    {
      if(xSemaphoreTake(mutexGame, portMAX_DELAY) == pdTRUE)
      {
        if(Game.resetBall)
        {
          ResetBall();
          continue;
        }
        if(Game.isRunning)
        {
          // mover pelota
          Game.ballX += Game.ballSpeedX;
          Game.ballY += Game.ballSpeedY;
          // rebote
          if((Game.ballY - Game.ballRadius <= 0) || (Game.ballY + Game.ballRadius >= SCREEN_HEIGHT))
          {
            Game.ballSpeedY = -abs(Game.ballSpeedY);
            if(Game.ballY - Game.ballRadius <= 0)
            {
              // rebote techo
              Game.ballY = Game.ballRadius;
            }
            else
            {
              // rebote piso
              Game.ballY = SCREEN_HEIGHT - Game.ballRadius;
            }
            xEventGroupSetBits(eventGroupSound, BIT_SOUND_HIT_WALL);
          }
          // rebote raqueta 1 left
          if((Game.ballX - Game.ballRadius) <= 5)
          {
            if((Game.ballY >= Game.paddle1Y) && (Game.ballY <= Game.paddle1Y + Game.paddle_height))
            {
              Game.ballSpeedX = abs(Game.ballSpeedX);
              Game.ballX = 5 + Game.ballRadius + 1;
              xEventGroupSetBits(eventGroupSound, BIT_SOUND_PADDLE_HIT);
            }          
          }
          // rebote raqueta 2 right
          if((Game.ballX + Game.ballRadius) >= SCREEN_WIDTH - 5)
          {
            if((Game.ballY >= Game.paddle2Y) && (Game.ballY <= Game.paddle2Y + Game.paddle_height))
            {
              Game.ballSpeedX = -abs(Game.ballSpeedX);
              Game.ballX = SCREEN_WIDTH - 5 - Game.ballRadius - 1;
              xEventGroupSetBits(eventGroupSound, BIT_SOUND_PADDLE_HIT);
            }
          }
          // goal
          if((Game.ballX <= 2) || (Game.ballX >= SCREEN_WIDTH - 2))
          {
            Game.resetBall = true;
            Game.isGoal = true;
            if(Game.ballX <= 2)
            {
              Game.score2++;
              xEventGroupSetBits(eventGroupSound, BIT_SOUND_SCORE);
            }
            else
            {
              Game.score1++;
              xEventGroupSetBits(eventGroupSound, BIT_SOUND_SCORE);
            }
          }
        }
        xSemaphoreGive(mutexGame);
      }
    }
    vTaskDelay(pdMS_TO_TICKS(30));
  }
}

void Task_Display(void *pvParameters)
{
  VarGame localGameCopy;
  int lastscore1, lastscore2 = 0;
  while(1)
  {
    if(xSemaphoreTake(mutexGame, portMAX_DELAY) == pdTRUE)
    {
      localGameCopy = Game;
      lastscore1 = Game.score1;
      lastscore2 = Game.score2;
      xSemaphoreGive(mutexGame);    
    }
    Oled.clearDisplay();
    if(localGameCopy.isRunning)
    {
      // dibuja juego
      DrawGameScreen(localGameCopy);
      xEventGroupSetBits(eventGroupSound, BIT_SOUND_IDLE);
    }
    else
    {
      // dibuja menu
      DrawMenuScreen(lastscore1, lastscore2);
    }
    Oled.display();
    vTaskDelay(pdMS_TO_TICKS(16));
  }
}

void Task_SoundEffects(void *pvParameters)
{
  EventBits_t bitsReceived;
  while(1)
  {
    bitsReceived = xEventGroupWaitBits(
              eventGroupSound, 
              BIT_SOUND_START | BIT_SOUND_PADDLE_HIT | BIT_SOUND_HIT_WALL | BIT_SOUND_SCORE | BIT_SOUND_IDLE | BIT_SOUND_MENU,
              pdTRUE,
              pdFALSE,
              portMAX_DELAY
            );
    if(bitsReceived & BIT_SOUND_START)
    {
      tone(BUZZER_PIN, NOTE_B5, 100); // Tuin
      //delay(150); // Pausa entre los dos sonidos
      tone(BUZZER_PIN, NOTE_B5, 200); // Tuin (un poco más largo el segundo)
      //delay(200);
      noTone(BUZZER_PIN);
    }
    if(bitsReceived & BIT_SOUND_PADDLE_HIT)
    {
      tone(BUZZER_PIN, NOTE_F3, 80); // Tu
      //delay(80); // Espera un poco
      tone(BUZZER_PIN, NOTE_C3, 100); // Ru (más grave)
      //delay(100);
      noTone(BUZZER_PIN);
    }
    if(bitsReceived & BIT_SOUND_HIT_WALL)
    {
      tone(BUZZER_PIN, NOTE_C4, 60); // Tu
      //delay(60);
      tone(BUZZER_PIN, NOTE_F4, 60); // Ra (un poco más agudo, rebote)
      //delay(60);
      noTone(BUZZER_PIN);
    }
    if(bitsReceived & BIT_SOUND_SCORE)
    {
      tone(BUZZER_PIN, NOTE_C5, 80);  // Tu
      //delay(90);
      tone(BUZZER_PIN, NOTE_E5, 80);  // ri
      //delay(90);
      tone(BUZZER_PIN, NOTE_G5, 80);  // tu
      //delay(90);
      tone(BUZZER_PIN, NOTE_C6, 150); // ri (Agudo final de victoria)
      //delay(150);
      noTone(BUZZER_PIN);
    }
    if(bitsReceived & BIT_SOUND_IDLE)
    {
      noTone(BUZZER_PIN);
    }
    vTaskDelay(pdMS_TO_TICKS(5));
  }
}