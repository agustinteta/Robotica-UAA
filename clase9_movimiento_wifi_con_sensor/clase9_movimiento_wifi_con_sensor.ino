#include <WiFi.h>
#include <WebServer.h>
#include "ESPAsyncWebServer.h"
#include "LittleFS.h"

// Reemplaza con tus credenciales WiFi
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

// Crea una instancia del servidor web en el puerto 80
WebServer server(80);

void handleRoot() {
  // HTML y JavaScript para el panel de control del robot
  String html = R"rawliteral(
    <!DOCTYPE html>
    <html>
    <head>
      <title>Control Robot ESP32</title>
      <meta name="viewport" content="width=device-width, initial-scale=1">
      <style>
        body { font-family: Arial, sans-serif; text-align: center; margin-top: 30px; background-color: #f0f0f0; }
        h1 { color: #333; }
        .control-grid {
          display: grid;
          grid-template-columns: repeat(3, 100px);
          grid-template-rows: repeat(3, 100px);
          gap: 15px;
          justify-content: center;
          margin-top: 40px;
        }
        button {
          background-color: #4CAF50; /* Verde */
          color: white;
          font-size: 16px;
          font-weight: bold;
          border-radius: 50%;
          border: none;
          cursor: pointer;
          box-shadow: 0 4px #999;
          transition: all 0.1s ease;
        }
        button:active {
          background-color: #3e8e41;
          box-shadow: 0 2px #666;
          transform: translateY(2px);
        }
        .btn-stop {
          background-color: #f44336; /* Rojo */
          border-radius: 12px;
        }
        .btn-stop:active {
          background-color: #da190b;
        }
        #status { margin-top: 30px; font-size: 20px; color: #555; font-weight: bold; }
        .sensor-container {
          background-color: #fff;
          border: 2px solid #ddd;
          border-radius: 10px;
          padding: 15px;
          width: 200px;
          margin: 20px auto;
          box-shadow: 0 4px 6px rgba(0,0,0,0.1);
        }
        .sensor-title { font-size: 14px; color: #777; text-transform: uppercase; margin: 0; }
        .sensor-value { font-size: 28px; color: #2196F3; font-weight: bold; margin: 5px 0 0 0; }
      </style>
    </head>
    <body>
      <h1>Panel de Control del Robot</h1>
      
      <div class="control-grid">
        <div></div>
        <button onclick="enviarComando('/forward')">&#9650;<br>Adelante</button>
        <div></div>
        
        <button onclick="enviarComando('/left')">&#9664;<br>Izq</button>
        <button class="btn-stop" onclick="enviarComando('/stop')">STOP</button>
        <button onclick="enviarComando('/right')">Der<br>&#9654;</button>
        
        <div></div>
        <button onclick="enviarComando('/backward')">&#9660;<br>Atrás</button>
        <div></div>
      </div>

      <div class="sensor-container">
        <p class="sensor-title">Distancia al Obstáculo</p>
        <p class="sensor-value"><span id="valDistancia">--</span> cm</p>
      </div>

      <p id="status">Estado: Detenido</p>

      <script>
        function enviarComando(ruta) {
          fetch(ruta)
            .then(response => response.text())
            .then(data => {
              document.getElementById('status').innerText = 'Estado: ' + data;
            })
            .catch(err => console.error('Error:', err));
        }

        function actualizarDistancia() {
          fetch('/distancia')
            .then(response => response.text())
            .then(data => {
              if(parseFloat(data) <= 0 || parseFloat(data) > 400) {
                document.getElementById('valDistancia').innerText = 'Fuera de rango';
              } else {
                document.getElementById('valDistancia').innerText = data;
              }
            })
            .catch(err => console.error('Error al leer sensor:', err));
        }

        setInterval(actualizarDistancia, 500);
      </script>
    </body>
    </html>
  )rawliteral";
  server.send(200, "text/html", html);
}

// --- HANDLERS DEL SERVIDOR WEB ---
void handleForward() {
  server.send(200, "text/plain", "Avanzando...");
  Serial.println("Robot Web: Adelante");
  moverAdelante();
}

void handleBackward() {
  server.send(200, "text/plain", "Retrocediendo...");
  Serial.println("Robot Web: Atrás");
  moverAtras();
}

void handleLeft() {
  server.send(200, "text/plain", "Girando a la Izquierda");
  Serial.println("Robot Web: Izquierda");
  girarIzquierda();
}

void handleRight() {
  server.send(200, "text/plain", "Girando a la Derecha");
  Serial.println("Robot Web: Derecha");
  girarDerecha();
}

void handleStop() {
  server.send(200, "text/plain", "Detenido");
  Serial.println("Robot Web: STOP");
  detenerMotores();
}

void handleDistancia() {
  server.send(200, "text/plain", String(distancia, 2));
}

void setup() {
  Serial.begin(115200);

  // Configuración de pines del sensor y Puente H
  pinMode(PIN_DISPARO, OUTPUT);
  pinMode(PIN_ECO, INPUT);

  pinMode(PIN_MOTOR_1_1, OUTPUT);
  pinMode(PIN_MOTOR_1_2, OUTPUT);
  pinMode(PIN_MOTOR_2_1, OUTPUT);
  pinMode(PIN_MOTOR_2_2, OUTPUT);

  detenerMotores();  // Aseguramos que empiece quieto

  Serial.println("\n--- Iniciando Configuración de Red ---");

  if (!WiFi.config(ipLocal, puertaEnlace, mascaraSubred, dns)) {
    Serial.println("¡Error al aplicar la configuración de IP fija!");
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

    // Rutas del servidor web
    server.on("/", handleRoot);
    server.on("/forward", handleForward);
    server.on("/backward", handleBackward);
    server.on("/left", handleLeft);
    server.on("/right", handleRight);
    server.on("/stop", handleStop);
    server.on("/distancia", handleDistancia);

    server.begin();
    Serial.println("Servidor HTTP iniciado.");
  } else {
    Serial.println("\n¡No se pudo conectar a WiFi!");
  }
}

void loop() {
  // Atiende las peticiones web de los botones
  server.handleClient();

  // Sistema de seguridad autónoma con el sensor ultrasónico
  distancia = leerDistancia();

  // Si detecta un obstáculo a menos de 30cm, detenemos el movimiento
  if (distancia > 0 && distancia <= 50) {
    Serial.print("¡Obstáculo detectado a ");
    Serial.print(distancia);
    Serial.println(" cm!.");

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
  // Motor 1 (Izquierdo) hacia atrás, Motor 2 (Derecho) hacia adelante
  digitalWrite(PIN_MOTOR_1_1, LOW);
  digitalWrite(PIN_MOTOR_1_2, HIGH);
  digitalWrite(PIN_MOTOR_2_1, HIGH);
  digitalWrite(PIN_MOTOR_2_2, LOW);
}

void girarDerecha() {
  // Motor 1 (Izquierdo) hacia adelante, Motor 2 (Derecho) hacia atrás
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

  long tiempo = pulseIn(PIN_ECO, HIGH, 30000);  // Timeout de 30ms para evitar bloqueos
  float d = tiempo / 58.3;
  return d;
}