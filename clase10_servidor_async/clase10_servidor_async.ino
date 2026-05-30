/*********
  Control LED GPIO4 con página web personalizada
*********/

// Import required libraries
#include "WiFi.h"
#include "ESPAsyncWebServer.h"
#include "LittleFS.h"

// CREDENCIALES:
const char *ssid = "Robotica";
const char *password = "";

// Set LED GPIO
const int ledPin = 4;

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

void setup(){
  // Serial port for debugging purposes
  Serial.begin(115200);
  Serial.println("=== Iniciando el Servidor de Control de LED ===");
  
  // Configure LED pin
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW); // Start with LED OFF
  Serial.println("LED GPIO4 configurado correctamente");

  // Initialize LittleFS
  Serial.println("Inicializando LittleFS...");
  if(!LittleFS.begin(true)){
    Serial.println("❌ ERROR: Fallo al montar LittleFS");
    return;
  }
  Serial.println("LittleFS montado correctamente");
  
  // Verify required files exist
  if(!LittleFS.exists("/index.html")){
    Serial.println("❌ ERROR: Archivo index.html no encontrado en LittleFS");
    Serial.println("Por favor, sube la carpeta data a LittleFS");
    return;
  } else {
    Serial.println("index.html encontrado en LittleFS");
  }

  // Connect to Wi-Fi
  Serial.println("Conectando a Wi-Fi...");
  Serial.print("SSID: ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, password);
  
  int wifi_retry_count = 0;
  const int max_wifi_retries = 30;
  
  while (WiFi.status() != WL_CONNECTED && wifi_retry_count < max_wifi_retries) {
    delay(1000);
    wifi_retry_count++;
    Serial.print(".");
  }
  
  if(WiFi.status() != WL_CONNECTED) {
    Serial.println("\n❌ ERROR: Fallo al conectar al WiFi");
    return;
  }

  Serial.println("\n¡WiFi Conectado Correctamente!");
  Serial.print("📡 Dirección IP: ");
  Serial.println(WiFi.localIP());

  // ==================== ROUTES CONFIGURATION ====================

  // Route for root / web page - SERVES YOUR HTML
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    Serial.println("Solicitud recibida para la página raíz (/)");
    request->send(LittleFS, "/index.html", "text/html");
  });
  
  // Route to load style.css file
  server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request){
    Serial.println("Solicitud recibida para style.css");
    if(LittleFS.exists("/style.css")) {
      request->send(LittleFS, "/style.css", "text/css");
    } else {
      request->send(404, "text/plain", "style.css no encontrado");
    }
  });

  // ==================== MAIN LED CONTROL ENDPOINT ====================
  // This endpoint is called by your JavaScript when button is clicked
  server.on("/led_turnOff", HTTP_GET, [](AsyncWebServerRequest *request){
    Serial.println("Solicitud recibida para /led_turnOff");
    
    if(request->hasArg("led_status")) {
        String led_status = request->arg("led_status");
        
        if(led_status == "1") {
            digitalWrite(ledPin, HIGH);
            Serial.println("LED GPIO4 encendido");
        } else {
            digitalWrite(ledPin, LOW); 
            Serial.println("LED GPIO4 apagado");
        }
        
        // Send JSON response back to your web page
        String jsonResponse = "{\"status\":\"success\", \"led_state\":\"" + led_status + "\"}";
        request->send(200, "application/json", jsonResponse);
        
    } else {
        Serial.println("❌ ERROR: Falta el parámetro led_status");
        request->send(400, "application/json", "{\"status\":\"error\", \"message\":\"Falta parametro\"}");
    }
  });

  // ==================== ADDITIONAL CONTROL ENDPOINTS ====================
  
  // Direct ON endpoint (optional)
  server.on("/on", HTTP_GET, [](AsyncWebServerRequest *request){
    digitalWrite(ledPin, HIGH);
    Serial.println("LED encendido a traves del endpoint /on");
    request->send(200, "application/json", "{\"status\":\"ON\"}");
  });
  
  // Direct OFF endpoint (optional)  
  server.on("/off", HTTP_GET, [](AsyncWebServerRequest *request){
    digitalWrite(ledPin, LOW);
    Serial.println("LED apagado a traves del endpoint /off");
    request->send(200, "application/json", "{\"status\":\"OFF\"}");
  });

  // Status endpoint to check current LED state
  server.on("/status", HTTP_GET, [](AsyncWebServerRequest *request){
    String ledState = digitalRead(ledPin) ? "ON" : "OFF";
    String jsonResponse = "{\"led_state\":\"" + ledState + "\", \"gpio\":4}";
    request->send(200, "application/json", jsonResponse);
    Serial.println("Estado solicitado - El LED esta: " + ledState);
  });

  // Handle 404 errors
  server.onNotFound([](AsyncWebServerRequest *request){
    Serial.println("❌ 404 - Página no encontrada: " + request->url());
    request->send(404, "application/json", "{\"status\":\"error\", \"message\":\"Endpoint no encontrado\"}");
  });

  // Start server
  server.begin();
  Serial.println("Servidor HTTP iniciado correctamente");
  Serial.println("Ahora puedes controlar el GPIO4 a traves de la interfaz web");
  Serial.println("Sirviendo archivos desde LittleFS");
}

void loop(){
}