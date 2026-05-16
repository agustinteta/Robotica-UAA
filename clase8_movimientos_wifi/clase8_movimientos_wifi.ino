#include <WiFi.h>
#include <WebServer.h>

// Reemplaza con tus credenciales WiFi
const char* ssid = "Red24";
const char* password = "Lourdes7944";

// Pin asignado para simular el movimiento
const int SIMULADOR_MOTOR_PIN = 33;

// Configuracion de IP Estatica
IPAddress ipLocal(192, 168, 0, 100);       // La IP estática que querés asignar
IPAddress puertaEnlace(192, 168, 0, 1);    // IP típica de un router en este rango
IPAddress mascaraSubred(255, 255, 255, 0); // Máscara de subred estándar
IPAddress dns(192, 168, 0, 1);             // El DNS (usamos el mismo router)

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
      </script>
    </body>
    </html>
  )rawliteral";
  server.send(200, "text/html", html);
}

// Funciones de control de movimiento
void handleForward() {
  server.send(200, "text/plain", "Avanzando...");
  Serial.println("Robot: Adelante");
  simularMovimiento();
}

void handleBackward() {
  server.send(200, "text/plain", "Retrocediendo...");
  Serial.println("Robot: Atrás");
  simularMovimiento();
}

void handleLeft() {
  server.send(200, "text/plain", "Girando a la Izquierda");
  Serial.println("Robot: Izquierda");
  simularMovimiento();
}

void handleRight() {
  server.send(200, "text/plain", "Girando a la Derecha");
  Serial.println("Robot: Derecha");
  simularMovimiento();
}

void handleStop() {
  digitalWrite(SIMULADOR_MOTOR_PIN, LOW);
  server.send(200, "text/plain", "Detenido");
  Serial.println("Robot: STOP");
}

// Función auxiliar para parpadear el GPIO 33 simulando tracción
void simularMovimiento() {
  for(int i = 0; i < 3; i++) {
    digitalWrite(SIMULADOR_MOTOR_PIN, HIGH);
    delay(100);
    digitalWrite(SIMULADOR_MOTOR_PIN, LOW);
    delay(100);
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(SIMULADOR_MOTOR_PIN, OUTPUT);
  digitalWrite(SIMULADOR_MOTOR_PIN, LOW);

  Serial.println("\n--- Iniciando Configuración de Red ---");
  
  // 1. Configurar la IP fija ANTES de iniciar el WiFi
  if (!WiFi.config(ipLocal, puertaEnlace, mascaraSubred, dns)) {
    Serial.println("¡Error al aplicar la configuración de IP fija!");
  } else {
    Serial.println("Configuración de IP fija aplicada correctamente.");
  }

  // 2. Iniciar la conexión WiFi
  Serial.print("Conectando a la red: ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  // Esperar a que se conecte o agote el tiempo de espera (10 segundos)
  int retries = 0;
  while (WiFi.status() != WL_CONNECTED && retries < 20) {
    delay(500);
    Serial.print(".");
    retries++;
  }

  // 3. Verificar si la conexión fue exitosa
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("");
    Serial.println("¡WiFi conectado exitosamente!");
    Serial.print("Dirección IP asignada: ");
    Serial.println(WiFi.localIP()); // Debería mostrar 192.168.0.100

    // Configuración de las rutas del servidor web del robot
    server.on("/", handleRoot);
    server.on("/forward", handleForward);
    server.on("/backward", handleBackward);
    server.on("/left", handleLeft);
    server.on("/right", handleRight);
    server.on("/stop", handleStop);
    
    server.begin();
    Serial.println("Servidor HTTP iniciado.");
    Serial.print("Potencia de la señal (RSSI): "); 
    Serial.print(WiFi.RSSI()); 
    Serial.println(" dBm");
  } else {
    Serial.println("");
    Serial.println("¡No se pudo conectar a WiFi! Revisá tus credenciales o el rango de red.");
  }
}

void loop() {
  server.handleClient();
}