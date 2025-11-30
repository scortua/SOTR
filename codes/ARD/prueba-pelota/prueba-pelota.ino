#include <Arduino.h>
// librerias
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "pitches.h" // notas musicales
// pines de pantalla oles
#define SCREEN_WIDTH 127
#define SCREEN_HEIGHT 63
#define OLED_RESET    -1
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
// pines del boton
#define BUTTON_PIN 12 // Pin del boton de inicio
// pines potenciometro para players
#define POT1_LEFT_PIN 34 // Pin del potenciometro jugador 1
#define POT2_RIGHT_PIN 35 // Pin del potenciometro jugador 2
// pin del buzzer
#define BUZZER_PIN 0 // Pin del buzzer

// display es el objeto de la clase y se puede llamar como sea, display, oled etc.
Adafruit_SSD1306 Oled(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET); 

void InputDataRead();
void ResetBall();
void GameLogic();
void DrawMenuScreen();
void DrawGameScreen();
void Display();
void SoundEffect();

int vel = 2;

struct VarGame
{
    int ballX, ballY;             // Posicion de la pelota
    int ballRadius;               // Tamaño de la pelota
    // Con numeros negativos se mueve a la izquierda/arriba y si es mas grande se mueve mas rapido
    int ballSpeedX, ballSpeedY;   // Velocidad de la pelota
    int paddle_height, paddle_width;// raquetas del pong
    int paddle1Y, paddle2Y;
    int score1, score2;           // Puntuacion de los jugadores
    bool isRunning;               // Estado del juego
    bool resetBall;               // Para reiniciar la pelota
}game;  // instancia de la estructura

struct InputData
{
  int pot1Value, pot2Value;
  bool buttonPressed;
}inputData; // instancia de la estructura

enum GameSounds
{
  SOUND_IDLE = 0,
  SOUND_PADDLE_HIT,
  SOUND_SCORE,
  SOUND_TOUCH_WALL,
  SOUND_START_GAME
}event;

void setup() 
{
  Serial.begin(115200);
  // boton
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  // potenciometros
  pinMode(POT1_LEFT_PIN, INPUT);
  pinMode(POT2_RIGHT_PIN, INPUT);
  // Inicializar Pantalla
  if(!Oled.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) 
  {
    Serial.println(F("Fail OLED"));
  }
  Oled.clearDisplay();
  Oled.display();
  // --- Inicializa el juego
  game.isRunning = true; // Empezar en menu
  game.ballRadius = 3;
  game.paddle_height = 9;
  game.paddle_width = 2;
  game.resetBall = true;
  game.score1 = 0;
  game.score2 = 0;
  // inicializar musica
  event = SOUND_IDLE;
  SoundEffect();
}

void loop() 
{
  InputDataRead();
  GameLogic();
  Display();
  delay(16); // ~60 FPS
}

void InputDataRead()
{
  inputData.buttonPressed = digitalRead(BUTTON_PIN);
  if(inputData.buttonPressed == LOW)
  {
    game.isRunning = !game.isRunning;
    game.resetBall = true; // Reiniciar la pelota al iniciar el juego
    delay(100); // Anti rebote y evitar multiples cambios de estado
  }
  inputData.pot1Value = analogRead(POT1_LEFT_PIN);
  game.paddle1Y = map(inputData.pot1Value, 0, 4095, 0, SCREEN_HEIGHT - game.paddle_height); // Mapear valor a posicion de raqueta
  inputData.pot2Value = analogRead(POT2_RIGHT_PIN);
  game.paddle2Y = map(inputData.pot2Value, 0, 4095, 0, SCREEN_HEIGHT - game.paddle_height);
  // Mapear valores de potenciometros a posiciones de raquetas
  Serial.print("Player 1: ");
  Serial.println(game.paddle1Y);
  Serial.print("Player 2: ");
  Serial.println(game.paddle2Y);
}

void ResetBall() {
  game.ballX = SCREEN_WIDTH / 2;
  game.ballY = SCREEN_HEIGHT / 2;
  // Dirección aleatoria
  game.ballSpeedX = (random(0, 2) == 0) ? vel : -vel; 
  game.ballSpeedY = (random(0, 2) == 0) ? vel : -vel;
  game.resetBall = false; // Ya terminamos el reset
}

void GameLogic()
{
  if(game.resetBall)
  {
    ResetBall();
    return; // Importante: si reseteamos, no calcules colisiones en este frame
  }
  if(game.isRunning)
  {
    // --- Mover pelota
    game.ballX += game.ballSpeedX;
    game.ballY += game.ballSpeedY;
    // --- REBOTE TECHO (Arriba)
    if(game.ballY - game.ballRadius <= 0) 
    {
      game.ballSpeedY = abs(game.ballSpeedY); // Forzamos velocidad positiva (bajar)
      game.ballY = game.ballRadius;           // Corregimos posición justo al borde
      event = SOUND_TOUCH_WALL;
      SoundEffect();
    }
    // --- REBOTE PISO (Abajo)
    else if(game.ballY + game.ballRadius >= SCREEN_HEIGHT) 
    {
      game.ballSpeedY = -abs(game.ballSpeedY); // Forzamos velocidad negativa (subir)
      game.ballY = SCREEN_HEIGHT - game.ballRadius; // Corregimos posición al borde de abajo
      event = SOUND_TOUCH_WALL;
      SoundEffect();
    }
    // --- REBOTE RAQUETAS player 1 izquierda
    if((game.ballX - game.ballRadius <= 5))
    {
      // Verificamos si la bola está a la altura de la raqueta
      if((game.ballY >= game.paddle1Y) && (game.ballY <= game.paddle1Y + game.paddle_height)) 
      {
          game.ballSpeedX = abs(game.ballSpeedX); // Rebota hacia la derecha
          game.ballX = 5 + game.ballRadius + 1;   // Sacamos la bola de la raqueta
          event = SOUND_PADDLE_HIT;
          SoundEffect();
      }
    }
    // --- REBOTE RAQUETAS player 2 derecha
    if((game.ballX + game.ballRadius >= SCREEN_WIDTH - 5))
    {
      // Verificamos altura
      if((game.ballY >= game.paddle2Y) && (game.ballY <= game.paddle2Y + game.paddle_height)) 
      {
          game.ballSpeedX = -abs(game.ballSpeedX); // Rebota hacia la izquierda
          game.ballX = SCREEN_WIDTH - 5 - game.ballRadius - 1; // Sacamos la bola
          event = SOUND_PADDLE_HIT;
          SoundEffect();
      }
    }
    // --- GOAL (Fuera de la pantalla)
    if(game.ballX < 0 || game.ballX > SCREEN_WIDTH)
    {
        game.resetBall = true;
        if(game.ballX < 0)
        {
          game.score2++; // Player 2 anota
          event = SOUND_SCORE;
          SoundEffect();
        }
        else if(game.ballX > SCREEN_WIDTH)
        {
          game.score1++; // Player 1 anota
          event = SOUND_SCORE;
          SoundEffect();
        }
    }
  }
}

void DrawMenuScreen()
{
  Oled.clearDisplay();
  // Marco decorativo
  Oled.drawRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, SSD1306_WHITE);
  // Título
  Oled.setTextSize(1);
  Oled.setTextColor(SSD1306_WHITE);
  Oled.setCursor(30, 2);
  Oled.print(F("PONG RTOS"));
  // Marcador anterior
  Oled.setTextSize(1);
  Oled.setCursor(15, 44);
  Oled.print(F("Last Score: "));
  Oled.print(game.score1);
  Oled.print("-");
  Oled.print(game.score2);
  // Instrucción
  Oled.setCursor(20, 54);
  Oled.print(F("[PRESS START]"));
  Oled.display();
}

void DrawGameScreen()
{
  Oled.clearDisplay();
  // Dibujar pelota
  Oled.fillCircle(game.ballX, game.ballY, game.ballRadius, SSD1306_WHITE); // bola rellana, posicion (x,y) del centro, radio y color
  // dibujar raquetas
  Oled.fillRect(2, game.paddle1Y, game.paddle_width, game.paddle_height, SSD1306_WHITE); // raqueta izquierda posicion (x,y) del extremo superior izquierdo, ancho, alto y color
  Oled.fillRect(SCREEN_WIDTH - game.paddle_width - 2, game.paddle2Y, game.paddle_width, game.paddle_height, SSD1306_WHITE); // raqueta derecha
  // Dibujar linea en el centro
  Oled.drawLine(SCREEN_WIDTH / 2, 0, SCREEN_WIDTH / 2, SCREEN_HEIGHT, SSD1306_WHITE);
  // Dibujar puntuacion
  Oled.setTextSize(2);
  Oled.setTextColor(SSD1306_WHITE);
  Oled.setCursor((SCREEN_WIDTH / 2) -20, 2);
  Oled.print(game.score1);
  Oled.setCursor((SCREEN_WIDTH / 2) + 10, 2);
  Oled.print(game.score2);
  Oled.display();
}

void Display()
{
  switch(game.isRunning)
  {
    case true:
      DrawGameScreen();
      event = SOUND_IDLE;
      SoundEffect();
      break;
    case false:
      DrawMenuScreen();
      break;
  }
}

void SoundEffect()
{
  switch (event)
  {
    case SOUND_IDLE:
      noTone(BUZZER_PIN);
      break;
    // "Turu grave"
    // Usamos la octava 3 (baja) descendiendo para dar peso
    case SOUND_PADDLE_HIT:
      tone(BUZZER_PIN, NOTE_F3, 80); // Tu
      //delay(80); // Espera un poco
      tone(BUZZER_PIN, NOTE_C3, 100); // Ru (más grave)
      //delay(100);
      noTone(BUZZER_PIN);
      break;
    // "Tura un poco menos grave"
    // Usamos la octava 4 (media), un poco más rápido y seco
    case SOUND_TOUCH_WALL:
      tone(BUZZER_PIN, NOTE_C4, 60); // Tu
      //delay(60);
      tone(BUZZER_PIN, NOTE_F4, 60); // Ra (un poco más agudo, rebote)
      //delay(60);
      noTone(BUZZER_PIN);
      break;
    // "Turituri medio grave y agudo"
    // Hacemos un arpegio rápido subiendo y bajando
    case SOUND_SCORE:
      tone(BUZZER_PIN, NOTE_C5, 80);  // Tu
      //delay(90);
      tone(BUZZER_PIN, NOTE_E5, 80);  // ri
      //delay(90);
      tone(BUZZER_PIN, NOTE_G5, 80);  // tu
      //delay(90);
      tone(BUZZER_PIN, NOTE_C6, 150); // ri (Agudo final de victoria)
      //delay(150);
      noTone(BUZZER_PIN);
      break;
    // "Tuin tuin"
    // Dos pulsos idénticos con una pequeña separación
    case SOUND_START_GAME:
      tone(BUZZER_PIN, NOTE_B5, 100); // Tuin
      //delay(150); // Pausa entre los dos sonidos
      tone(BUZZER_PIN, NOTE_B5, 200); // Tuin (un poco más largo el segundo)
      //delay(200);
      noTone(BUZZER_PIN);
      break;      
    default:
      noTone(BUZZER_PIN);
      break;
  }
}