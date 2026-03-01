#include <IRremote.hpp>
#include <U8g2lib.h>
#include <DHT.h>
#include <Wire.h>
#include <avr/wdt.h>

#define DECODE_NEC  
#define trigPin 7
#define echoPin 4
#define DHTPIN 2  // Pin del sensor dht de hum y temp
#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE);
// Usamos el constructor _1_ (Page Buffer)
// U8G2_SH1106_128X64_NONAME_1_HW_I2C u8g2(U8G2_R0, /* reset=*/U8X8_PIN_NONE);
// Nota: Usamos este constructor que suele ir mejor para actualizaciones rapidas
U8G2_SH1106_128X64_NONAME_1_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);

// --- ESTADOS ---
enum RobotState { IDLE,
                  MOVING_FWD,
                  MOVING_BCK,
                  TURNING_L,
                  TURNING_R,
                  OBSTACLE };
RobotState currentState = IDLE;

unsigned long previousMillis = 0;
unsigned long timerDHT = 0;
unsigned long timerScreen = 0;
float t = 0;
float h = 0;

int pinA1 = 10;
int pinA2 = 5;
int pinB1 = 6;
int pinB2 = 9;
// ------------------------

int globalSpeed = 255;  //Velocidad max == 255
bool autoMode = false;

// --- ESTADOS DE ÁNIMO ---
enum Mood { HAPPY, ANGRY, SUSPICIOUS, TIRED };
Mood currentMood = HAPPY;
int lookX = 0; // -10 (Izq) a 10 (Der)
int lookY = 0; // -10 (Arriba) a 10 (Abajo)

// Variables de animación de Ojos
unsigned long nextBlink = 0;
bool isBlinking = false;
unsigned long blinkStart = 0;
unsigned long nextGazeMove = 0;

String receivedString = "";

// Prototipos de funciones nuevas
void drawFluxEyes();
void updateEyeAnimation();
void forceDraw(); // Para dibujar inmediatamente en emergencias

void setup() {
  wdt_disable();

  Serial.begin(9600);
  randomSeed(analogRead(A0));

  //Inicializar Oled y dht
  u8g2.begin();
  u8g2.setContrast(255);  // Contraste maximo para estilo Flux
  dht.begin();

  // Lectura inicial
  t = dht.readTemperature();
  h = dht.readHumidity();

  //Ultrasonido
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  //Sensores InfraRojos
  pinMode(8, INPUT);
  pinMode(12, INPUT);

  //A
  pinMode(pinA1, OUTPUT);
  pinMode(pinA2, OUTPUT);

  //B
  pinMode(pinB1, OUTPUT);
  pinMode(pinB2, OUTPUT);

  stop();

  wdt_enable(WDTO_4S);  // Watchdog activado (4s por seguridad)
}

void loop() {
  wdt_reset();

  unsigned long currentMillis = millis();

  // --- TAREA 1: LEER SENSOR (LENTO: Cada 2 seg) ---
  if (currentMillis - timerDHT >= 2000) {
    timerDHT = currentMillis;
    float newT = dht.readTemperature();
    float newH = dht.readHumidity();
    if (!isnan(newT)) t = newT;
    if (!isnan(newH)) h = newH;
    
    //mando al esp para q la app lo pueda leer
    sendDataToESP32();
  }

  // --- NUEVO: ACTUALIZAR ANIMACIÓN ---
  updateEyeAnimation();

  // --- TAREA 2: PANTALLA (AHORA DIBUJA OJOS ESTILO FLUX) ---
  if (currentMillis - timerScreen >= 30) { // 30ms para fluidez
    timerScreen = currentMillis;

    u8g2.firstPage();
    do {
       drawFluxEyes();
    } while (u8g2.nextPage());
  }

  // 2. DETECTAR OBSTÁCULOS (LÓGICA PRIORITARIA)
  // Reemplaza al bloque anterior para evitar conflictos con delays
  checkSensorsAndEvade();

  // 3. RECIBIR DATOS (SI NO HAY OBSTACULO)
  if (currentState != OBSTACLE) {
     receiveUIData();
  }
}

// --- FUNCIONES DE CONTROL ---

void checkSensorsAndEvade() {
  float dist = readDistance();
  bool leftSensor = digitalRead(8);
  bool rightSensor = digitalRead(12);

  // Umbral de 15cm para reaccionar antes
  if ((dist > 1 && dist < 15) || leftSensor == LOW || rightSensor == LOW) {
    
    // 1. CAMBIO DE ÁNIMO INMEDIATO
    currentMood = ANGRY;
    lookX = 0; lookY = 0;

    // 2. FORZAR DIBUJO (Para ver la cara antes de moverse)
    forceDraw();

    String causa = "FRONTAL";
    if (leftSensor == LOW) causa = "IZQUIERDA";
    else if (rightSensor == LOW) causa = "DERECHA";

    Serial.print(F("EMERGENCIA: "));
    Serial.println(causa);

    currentState = OBSTACLE;
    
    // 3. EVASION (Bloqueante con delays, pero ya dibujamos la cara)
    handleEvasion(causa);
    
    currentState = IDLE;
    currentMood = HAPPY; // Vuelve a feliz al terminar
  }
}

void forceDraw() {
  u8g2.firstPage();
  do {
     drawFluxEyes();
  } while (u8g2.nextPage());
}

float readDistance() {
  float duration, distance;
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  duration = pulseIn(echoPin, HIGH, 25000); // Timeout reducido
  distance = ((duration * 0.0343) / 2);
  if (distance <= 0) return 400;
  return distance;
}

void sendDataToESP32(){
  // Formato: "DATA:Temp,Hum,Vel"
  Serial.print("DATA:");
  Serial.print(t);
  Serial.print(",");
  Serial.print(h);
  Serial.print(",");
  Serial.println(globalSpeed); 
}

void handleEvasion(String tipo) {
  wdt_reset();
  stop(); // Parar brevemente
  // delay(100); Eliminado para reaccion rapida
  
  backwards(globalSpeed);
  delay(400); // Retroceso corto
  stop();
  delay(100);
  wdt_reset();

  if (tipo == "IZQUIERDA") {
    right(globalSpeed);
  } else if (tipo == "DERECHA") {
    left(globalSpeed);
  } else {
    // Random direction
    if (random(0, 2) == 0) right(globalSpeed);
    else left(globalSpeed);
  }

  delay(400); // Giro
  stop();
}

void readIRProximitySensors(bool &leftSensor, bool &rightSensor) {
  rightSensor = digitalRead(12);
  leftSensor = digitalRead(8);
}

void receiveUIData() {
  if (Serial.available() > 0) {
    char receivedByte = Serial.read();

    if (receivedByte == '\n') {
      if (receivedString == "F") {
        forwards(globalSpeed); currentMood = HAPPY; lookX = 0; lookY = 0;
      } else if (receivedString == "B") {
        backwards(globalSpeed); currentMood = TIRED; lookX = 0; lookY = 0;
      } else if (receivedString == "L") {
        left(globalSpeed); currentMood = HAPPY; lookX = -12; lookY = 0;
      } else if (receivedString == "R") {
        right(globalSpeed); currentMood = HAPPY; lookX = 12; lookY = 0;
      } else if (receivedString == "S") {
        stop(); currentMood = HAPPY; lookX = 0; lookY = 0;
      }
      
      // RESPUESTA AL ESP32 (ACK)
      // Serial.print("ACK:"); Serial.println(receivedString); // Opcional para debug
      receivedString = "";
    } else {
      receivedString += receivedByte;
    }
  }
}

// --- MOVIMIENTOS ---

void backwards(int speed) {
  currentState = MOVING_BCK;
  analogWrite(pinA1, 0); analogWrite(pinA2, speed);
  analogWrite(pinB1, speed); analogWrite(pinB2, 0);
}

void forwards(int speed) {
  currentState = MOVING_FWD;
  analogWrite(pinA1, speed); analogWrite(pinA2, 0);
  analogWrite(pinB1, 0); analogWrite(pinB2, speed);
}

void right(int speed) {
  currentState = TURNING_R;
  analogWrite(pinA1, speed); analogWrite(pinA2, 0);
  analogWrite(pinB1, speed); analogWrite(pinB2, 0);
}

void left(int speed) {
  currentState = TURNING_L;
  analogWrite(pinA1, 0); analogWrite(pinA2, speed);
  analogWrite(pinB1, 0); analogWrite(pinB2, speed);
}

void stop() {
  currentState = IDLE;
  analogWrite(pinA1, LOW); analogWrite(pinA2, LOW);
  analogWrite(pinB1, LOW); analogWrite(pinB2, LOW);
}

// --- FUNCIONES DE DIBUJO ESTILO FLUX ---

void updateEyeAnimation() {
  // Lógica de Parpadeo
  if (millis() > nextBlink && !isBlinking) {
    isBlinking = true;
    blinkStart = millis();
  }
  if (isBlinking && millis() - blinkStart > 150) { 
    isBlinking = false;
    nextBlink = millis() + random(1500, 5000);
  }

  // Lógica de Mirada Aleatoria
  if (currentState == IDLE && currentMood == HAPPY && millis() > nextGazeMove) {
    lookX = random(-8, 9);
    lookY = random(-5, 6);
    nextGazeMove = millis() + random(800, 2500);
  }
}

void drawFluxEyes() {
  // Configuración Estilo Flux (Cápsulas)
  int eyeW = 42;    // Ancho del ojo
  int eyeH = 55;    // Alto del ojo
  int radius = 12;  // Redondeo fuerte
  int gap = 8;      // Espacio
  int yPos = 5;     // Posición Y
  
  int leftX = 64 - (gap/2) - eyeW;
  int rightX = 64 + (gap/2);

  if (isBlinking) {
    // Parpadeo estilo cápsula cerrada
    u8g2.drawRBox(leftX, yPos + eyeH/2 - 2, eyeW, 4, 2);
    u8g2.drawRBox(rightX, yPos + eyeH/2 - 2, eyeW, 4, 2);
    return;
  }

  // 1. DIBUJAR OJOS (CÁPSULAS BLANCAS)
  u8g2.drawRBox(leftX, yPos, eyeW, eyeH, radius);
  u8g2.drawRBox(rightX, yPos, eyeW, eyeH, radius);

  // 2. DIBUJAR PUPILAS (RECTÁNGULOS NEGROS REDONDEADOS)
  u8g2.setColorIndex(0); // Negro
  
  int pupW = 18;
  int pupH = 22;
  int pupRad = 6;
  
  // Limitar movimiento
  int pX = constrain(lookX, -10, 10);
  int pY = constrain(lookY, -15, 15);

  int lCenterX = leftX + (eyeW - pupW)/2;
  int rCenterX = rightX + (eyeW - pupW)/2;
  int centerY = yPos + (eyeH - pupH)/2;
  
  u8g2.drawRBox(lCenterX + pX, centerY + pY, pupW, pupH, pupRad);
  u8g2.drawRBox(rCenterX + pX, centerY + pY, pupW, pupH, pupRad);

  // 3. EXPRESIONES (PÁRPADOS)
  u8g2.setColorIndex(1); // Blanco para tapar

  if (currentMood == ANGRY) {
    // Ceño fruncido (Triangulos negros que cortan la capsula)
    u8g2.setColorIndex(0); // Borrar
    u8g2.drawTriangle(leftX, yPos, leftX + eyeW, yPos, leftX + eyeW, yPos + 25);
    u8g2.drawTriangle(rightX, yPos, rightX + eyeW, yPos, rightX, yPos + 25);
    u8g2.setColorIndex(1);
  } else if (currentMood == TIRED) {
    // Parpado plano
    u8g2.setColorIndex(0);
    u8g2.drawBox(leftX, yPos, eyeW, 20);
    u8g2.drawBox(rightX, yPos, eyeW, 20);
    u8g2.setColorIndex(1);
  }
}