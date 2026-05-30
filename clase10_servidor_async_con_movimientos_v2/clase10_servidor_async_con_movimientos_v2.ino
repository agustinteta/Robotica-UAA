#include <WiFi.h>
#include "ESPAsyncWebServer.h"
#include "LittleFS.h"

// Credenciales WiFi
const char* ssid = "RedRobotica24";
const char* password = "";

// Pines Sensor Ultrasonido
const int PIN_DISPARO = 12;
const int PIN_ECO = 14;

// Pines Puente H (Motores)
const int PIN_MOTOR_1_1 = 16;
const int PIN_MOTOR_1_2 = 17;
const int PIN_MOTOR_2_1 = 18;
const int PIN_MOTOR_2_2 = 19;

// Configuracion de IP Estatica
IPAddress ipLocal(192, 168, 0, 100);
IPAddress puertaEnlace(192, 168, 0, 1);
IPAddress mascaraSubred(255, 255, 255, 0);
IPAddress dns(192, 168, 0, 1);

// Variable para la distancia del sensor
float distancia;

// Instancia del servidor web asíncrono en el puerto 80
AsyncWebServer server(80);

// Declaración de funciones de movimiento (prototipos)
void moverAdelante();
void moverAtras();
void girarIzquierda();
void girarDerecha();
void detenerMotores();
float leerDistancia();

void setup() {
  Serial.begin(115200);

  // Configuración de pines del sensor y Puente H
  pinMode(PIN_DISPARO, OUTPUT);
  pinMode(PIN_ECO, INPUT);

  pinMode(PIN_MOTOR_1_1, OUTPUT);
  pinMode(PIN_MOTOR_1_2, OUTPUT);
  pinMode(PIN_MOTOR_2_1, OUTPUT);
  pinMode(PIN_MOTOR_2_2, OUTPUT);

  detenerMotores(); // Aseguramos que empiece quieto

  // Inicializar LittleFS
  Serial.println("Inicializando LittleFS...");
  if (!LittleFS.begin(true)) {
    Serial.println("ERROR: Fallo al montar LittleFS");
    return;
  }
  Serial.println("LittleFS montado correctamente.");

  Serial.println("\n--- Iniciando Configuración de Red ---");
  if (!WiFi.config(ipLocal, puertaEnlace, mascaraSubred, dns)) {
    Serial.println("ERROR: ¡Error al aplicar la configuración de IP fija!");
  } else {
    Serial.println("Configuración de IP fija aplicada correctamente.");
  }

  Serial.print("Conectando a la red: ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  int retries = 0;
  while (WiFi.status() != WL_CONNECTED && retries < 20) {
    delay(500);
    Serial.print(".");
    retries++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n¡WiFi conectado exitosamente!");
    Serial.print("Dirección IP asignada: ");
    Serial.println(WiFi.localIP());

    // --- RUTAS ASÍNCRONAS ---
    // Ruta raíz - index.html desde Little fs
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
      request->send(LittleFS, "/index.html", "text/html");
    });

    // Ruta para los estilos CSS
    server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request) {
      if (LittleFS.exists("/style.css")) {
        request->send(LittleFS, "/style.css", "text/css");
      } else {
        request->send(404, "text/plain", "style.css no encontrado");
      }
    });

    // Rutas de Control de Movimiento
    server.on("/forward", HTTP_GET, [](AsyncWebServerRequest *request) {
      Serial.println("Robot Web: Adelante");
      moverAdelante();
      request->send(200, "text/plain", "Avanzando...");
    });

    server.on("/backward", HTTP_GET, [](AsyncWebServerRequest *request) {
      Serial.println("Robot Web: Atrás");
      moverAtras();
      request->send(200, "text/plain", "Retrocediendo...");
    });

    server.on("/left", HTTP_GET, [](AsyncWebServerRequest *request) {
      Serial.println("Robot Web: Izquierda");
      girarIzquierda();
      request->send(200, "text/plain", "Girando a la Izquierda");
    });

    server.on("/right", HTTP_GET, [](AsyncWebServerRequest *request) {
      Serial.println("Robot Web: Derecha");
      girarDerecha();
      request->send(200, "text/plain", "Girando a la Derecha");
    });

    server.on("/stop", HTTP_GET, [](AsyncWebServerRequest *request) {
      Serial.println("Robot Web: STOP");
      detenerMotores();
      request->send(200, "text/plain", "Detenido");
    });

    // Ruta para devolver el valor numérico de la distancia
    server.on("/distancia", HTTP_GET, [](AsyncWebServerRequest *request) {
      request->send(200, "text/plain", String(distancia, 2));
    });

    // Manejo de error 404
    server.onNotFound([](AsyncWebServerRequest *request) {
      request->send(404, "text/plain", "No encontrado");
    });

    server.begin();
    Serial.println("Servidor HTTP Async iniciado.");
  } else {
    Serial.println("\n¡No se pudo conectar a WiFi!");
  }
}

void loop() {

  // Sistema de seguridad autónoma con el sensor ultrasónico
  distancia = leerDistancia();

  // Si detecta un obstáculo a menos de 50cm, retrocede y desvía
  if (distancia > 0 && distancia <= 50) {
    Serial.print("¡Obstáculo detectado a ");
    Serial.print(distancia);
    Serial.println(" cm!");

    moverAtras();
    delay(1000);
    girarIzquierda();
    delay(250);
    detenerMotores();
  }

  delay(50);
}

// --- FUNCIONES DE MOVIMIENTO ---

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

// --- FUNCIÓN SENSOR ---
float leerDistancia() {
  digitalWrite(PIN_DISPARO, LOW);
  delayMicroseconds(2);
  digitalWrite(PIN_DISPARO, HIGH);
  delayMicroseconds(10);
  digitalWrite(PIN_DISPARO, LOW);

  long tiempo = pulseIn(PIN_ECO, HIGH, 30000); // Timeout de 30ms
  float d = tiempo / 58.3;
  return d;
}