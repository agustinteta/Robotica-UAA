/*
  Control robot móvil (avanzar / retroceder) con servidor asincrónico.
  Dos botones en la web; mutuamente excluyentes.
  Página web servida desde LittleFS.
*/

#include "WiFi.h"
#include "ESPAsyncWebServer.h"
#include "LittleFS.h"

// Credenciales WiFi
const char *ssid     = "Robotica";
const char *password = "";

// Pines motor (L298N): motor izquierdo IN1/IN2, motor derecho IN3/IN4
const int IN1 = 25;
const int IN2 = 26;
const int IN3 = 27;
const int IN4 = 14;

// Estado de los pulsadores
bool btn_avanzar    = false;
bool btn_retroceder = false;

AsyncWebServer server(80);

// ---------- funciones de movimiento ----------

void detener() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
}

void avanzar() {
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
}

void retroceder() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
}

// Aplica el estado actual de los pulsadores al robot
void actualizarMovimiento() {
  if (btn_avanzar)         avanzar();
  else if (btn_retroceder) retroceder();
  else                     detener();
}

// ---------- setup ----------

void setup() {
  Serial.begin(115200);
  Serial.println("=== Control Robot Móvil ===");

  // Configurar pines de motor
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  detener();

  // LittleFS
  if (!LittleFS.begin(true)) {
    Serial.println("ERROR: fallo al montar LittleFS");
    return;
  }
  if (!LittleFS.exists("/index.html")) {
    Serial.println("ERROR: index.html no encontrado en LittleFS");
    return;
  }
  Serial.println("LittleFS OK");

  // WiFi
  WiFi.begin(ssid, password);
  Serial.print("Conectando a WiFi");
  int intentos = 0;
  while (WiFi.status() != WL_CONNECTED && intentos < 30) {
    delay(1000);
    Serial.print(".");
    intentos++;
  }
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("\nERROR: no se pudo conectar al WiFi");
    return;
  }
  Serial.print("\nIP: ");
  Serial.println(WiFi.localIP());

  // -------- rutas --------

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(LittleFS, "/index.html", "text/html");
  });

  server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(LittleFS, "/style.css", "text/css");
  });

  // Endpoint pulsador 1 — avanzar
  // GET /btn1?state=1  o  GET /btn1?state=0
  server.on("/btn1", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (!request->hasArg("state")) {
      request->send(400, "application/json", "{\"error\":\"falta parametro state\"}");
      return;
    }
    btn_avanzar = request->arg("state") == "1";
    if (btn_avanzar) btn_retroceder = false; // exclusión mutua
    actualizarMovimiento();

    String resp = "{\"btn1\":" + String(btn_avanzar ? "true" : "false") +
                  ",\"btn2\":" + String(btn_retroceder ? "true" : "false") + "}";
    Serial.println("btn1=" + String(btn_avanzar) + " => " + (btn_avanzar ? "AVANZAR" : "DETENER"));
    request->send(200, "application/json", resp);
  });

  // Endpoint pulsador 2 — retroceder
  server.on("/btn2", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (!request->hasArg("state")) {
      request->send(400, "application/json", "{\"error\":\"falta parametro state\"}");
      return;
    }
    btn_retroceder = request->arg("state") == "1";
    if (btn_retroceder) btn_avanzar = false; // exclusión mutua
    actualizarMovimiento();

    String resp = "{\"btn1\":" + String(btn_avanzar ? "true" : "false") +
                  ",\"btn2\":" + String(btn_retroceder ? "true" : "false") + "}";
    Serial.println("btn2=" + String(btn_retroceder) + " => " + (btn_retroceder ? "RETROCEDER" : "DETENER"));
    request->send(200, "application/json", resp);
  });

  // Estado actual
  server.on("/status", HTTP_GET, [](AsyncWebServerRequest *request) {
    String resp = "{\"btn1\":" + String(btn_avanzar ? "true" : "false") +
                  ",\"btn2\":" + String(btn_retroceder ? "true" : "false") + "}";
    request->send(200, "application/json", resp);
  });

  server.onNotFound([](AsyncWebServerRequest *request) {
    request->send(404, "application/json", "{\"error\":\"not found\"}");
  });

  server.begin();
  Serial.println("Servidor iniciado");
}

void loop() {}
