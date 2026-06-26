#include <WiFi.h>
#include <ESP32Servo.h>
#include <ESPAsyncWebServer.h>
#include <LittleFS.h>

// WiFi 
const char* ssid     = "RedRoboticaUAA";
const char* password = "";

// Sensor ultrasónico 
#define PIN_DISPARO 12
#define PIN_ECO     14

// Motores 
#define PIN_MOTOR_1_1 19
#define PIN_MOTOR_1_2 18
#define PIN_MOTOR_2_1 22
#define PIN_MOTOR_2_2 21
#define PIN_PWM1      5
#define PIN_PWM2      23

// PWM 
#define CANAL_PWM1     4
#define CANAL_PWM2     1
#define PWM_FREQ       500
#define PWM_RESOLUTION 8
#define MAX_DUTY_CYCLE 255

// Servo 
#define PIN_SERVO 13
Servo miServo;

// IP estática
IPAddress localIP(192, 168, 0, 100);
IPAddress gateway(192, 168, 0, 1);
IPAddress subnet(255, 255, 255, 0);
IPAddress dns(192, 168, 0, 1);

// Estado global 
float distancia      = 0;
int   velocidadMotor = 0;
int   anguloServo    = 90;
bool  modoPDI        = false;   // false = Normal, true = PDI (visión computacional)

AsyncWebServer server(80);

// Prototipos de funciones 
void moverAdelante();
void moverAtras();
void girarIzquierda();
void girarDerecha();
void detenerMotores();
float leerDistancia();


void setup() {
  Serial.begin(115200);

  // PWM para motores
  ledcSetup(CANAL_PWM1, PWM_FREQ, PWM_RESOLUTION);
  ledcSetup(CANAL_PWM2, PWM_FREQ, PWM_RESOLUTION);
  ledcAttachPin(PIN_PWM1, CANAL_PWM1);
  ledcAttachPin(PIN_PWM2, CANAL_PWM2);
  ledcWrite(CANAL_PWM1, velocidadMotor);
  ledcWrite(CANAL_PWM2, velocidadMotor);

  // Pines sensor y motores
  pinMode(PIN_DISPARO,   OUTPUT);
  pinMode(PIN_ECO,       INPUT);
  pinMode(PIN_MOTOR_1_1, OUTPUT);
  pinMode(PIN_MOTOR_1_2, OUTPUT);
  pinMode(PIN_MOTOR_2_1, OUTPUT);
  pinMode(PIN_MOTOR_2_2, OUTPUT);

  // Servo
  miServo.attach(PIN_SERVO);
  miServo.write(anguloServo);

  // LittleFS
  if (!LittleFS.begin(true)) {
    Serial.println("Error al montar LittleFS");
    return;
  }
  Serial.println("LittleFS montado correctamente");

  // WiFi con IP estática
  if (!WiFi.config(localIP, gateway, subnet, dns)) {
    Serial.println("Error al configurar IP estática");
  }
  WiFi.begin(ssid, password);
  Serial.print("Conectando a WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());

  // Rutas HTTP

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *req) {
    req->send(LittleFS, "/index.html", "text/html");
  });

  server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *req) {
    req->send(LittleFS, "/style.css", "text/css");
  });

  // Movimiento
  server.on("/forward", HTTP_GET, [](AsyncWebServerRequest *req) {
    moverAdelante();
    req->send(200, "text/plain", "Adelante");
  });

  server.on("/backward", HTTP_GET, [](AsyncWebServerRequest *req) {
    moverAtras();
    req->send(200, "text/plain", "Atrás");
  });

  server.on("/left", HTTP_GET, [](AsyncWebServerRequest *req) {
    girarIzquierda();
    req->send(200, "text/plain", "Izquierda");
  });

  server.on("/right", HTTP_GET, [](AsyncWebServerRequest *req) {
    girarDerecha();
    req->send(200, "text/plain", "Derecha");
  });

  server.on("/stop", HTTP_GET, [](AsyncWebServerRequest *req) {
    detenerMotores();
    req->send(200, "text/plain", "Detenido");
  });

  // Distancia (consultada periódicamente por el frontend)
  server.on("/distancia", HTTP_GET, [](AsyncWebServerRequest *req) {
    req->send(200, "text/plain", String(distancia));
  });

  // Velocidad: /velocidad?valor=0-255
  server.on("/velocidad", HTTP_GET, [](AsyncWebServerRequest *req) {
    if (req->hasParam("valor")) {
      int v = req->getParam("valor")->value().toInt();
      v = constrain(v, 0, MAX_DUTY_CYCLE);
      velocidadMotor = v;
      ledcWrite(CANAL_PWM1, velocidadMotor);
      ledcWrite(CANAL_PWM2, velocidadMotor);
      req->send(200, "text/plain", "Velocidad: " + String(velocidadMotor));
    } else {
      req->send(400, "text/plain", "Parámetro 'valor' requerido");
    }
  });

  // Servo cámara — paso de 15° a la izquierda
  server.on("/cam_izq", HTTP_GET, [](AsyncWebServerRequest *req) {
    anguloServo = max(0, anguloServo - 15);
    miServo.write(anguloServo);
    req->send(200, "text/plain", "Ángulo: " + String(anguloServo));
  });

  // Servo cámara — paso de 15° a la derecha
  server.on("/cam_der", HTTP_GET, [](AsyncWebServerRequest *req) {
    anguloServo = min(180, anguloServo + 15);
    miServo.write(anguloServo);
    req->send(200, "text/plain", "Ángulo: " + String(anguloServo));
  });

  // Modo de operación: /modo?val=true|false
  server.on("/modo", HTTP_GET, [](AsyncWebServerRequest *req) {
    if (req->hasParam("val")) {
      String val = req->getParam("val")->value();
      modoPDI = (val == "true");
      if (!modoPDI) detenerMotores();
    }
    req->send(200, "text/plain", modoPDI ? "PDI" : "NORMAL");
  });

  server.onNotFound([](AsyncWebServerRequest *req) {
    req->send(404, "text/plain", "No encontrado");
  });

  server.begin();
  Serial.println("Servidor iniciado");
}


void loop() {
  distancia = leerDistancia();

  if (!modoPDI) {
    if (distancia > 0 && distancia <= 50) {
      moverAtras();
      delay(1000);
      girarIzquierda();
      delay(250);
      detenerMotores();
    }
  }

  delay(50);
}


void moverAdelante() {
  digitalWrite(PIN_MOTOR_1_1, HIGH);
  digitalWrite(PIN_MOTOR_1_2, LOW);
  digitalWrite(PIN_MOTOR_2_1, HIGH);
  digitalWrite(PIN_MOTOR_2_2, LOW);
}

void moverAtras() {
  digitalWrite(PIN_MOTOR_1_1, LOW);
  digitalWrite(PIN_MOTOR_1_2, HIGH);
  digitalWrite(PIN_MOTOR_2_1, LOW);
  digitalWrite(PIN_MOTOR_2_2, HIGH);
}

void girarIzquierda() {
  digitalWrite(PIN_MOTOR_1_1, LOW);
  digitalWrite(PIN_MOTOR_1_2, HIGH);
  digitalWrite(PIN_MOTOR_2_1, HIGH);
  digitalWrite(PIN_MOTOR_2_2, LOW);
}

void girarDerecha() {
  digitalWrite(PIN_MOTOR_1_1, HIGH);
  digitalWrite(PIN_MOTOR_1_2, LOW);
  digitalWrite(PIN_MOTOR_2_1, LOW);
  digitalWrite(PIN_MOTOR_2_2, HIGH);
}

void detenerMotores() {
  digitalWrite(PIN_MOTOR_1_1, LOW);
  digitalWrite(PIN_MOTOR_1_2, LOW);
  digitalWrite(PIN_MOTOR_2_1, LOW);
  digitalWrite(PIN_MOTOR_2_2, LOW);
}


float leerDistancia() {
  digitalWrite(PIN_DISPARO, LOW);
  delayMicroseconds(2);
  digitalWrite(PIN_DISPARO, HIGH);
  delayMicroseconds(10);
  digitalWrite(PIN_DISPARO, LOW);

  long tiempo = pulseIn(PIN_ECO, HIGH, 30000); // timeout 30 ms
  return tiempo / 58.3;
}
