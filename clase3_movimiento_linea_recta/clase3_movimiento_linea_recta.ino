#include <Servo.h>
#include <math.h>

// Configuración de Hardware
Servo servo1;
Servo servo2;
const int PIN_SERVO1 = 9;
const int PIN_SERVO2 = 10;
const int PIN_LED_ERROR = 13;

// Dimensiones del Robot (mm)
const float L1 = 150.0;
const float L2 = 150.0;
const float ALCANCE_MAX = L1 + L2;

// Parámetros de interpolación
const int PASOS = 20;

void setup() {
  Serial.begin(9600);
  servo1.attach(PIN_SERVO1);
  servo2.attach(PIN_SERVO2);
  pinMode(PIN_LED_ERROR, OUTPUT);
  
  // Ejemplo de ejecución: Movimiento de A(100, 100) a B(200, 50)
  moverEnLineaRecta(100, 100, 200, 50);
}

void loop() {
  // El programa puede esperar comandos por Serial aquí
}

void moverEnLineaRecta(float xa, float ya, float xb, float yb) {
  // 1. Verificar si los puntos son alcanzables (Espacio de trabajo)
  if (!esAlcanzable(xa, ya) || !esAlcanzable(xb, yb)) {
    digitalWrite(PIN_LED_ERROR, HIGH);
    Serial.println("ERROR: Punto fuera del alcance del robot.");
    return;
  }
  
  digitalWrite(PIN_LED_ERROR, LOW);
  Serial.println("Iniciando trayectoria...");

  // 2. Interpolación lineal y Cinemática Inversa
  for (int i = 0; i <= PASOS; i++) {
    // Calcular punto intermedio (xi, yi)
    float xi = xa + (xb - xa) * (float(i) / PASOS);
    float yi = ya + (yb - ya) * (float(i) / PASOS);

    // Calcular ángulos q1 y q2 en radianes
    float q2_rad = acos((pow(xi, 2) + pow(yi, 2) - pow(L1, 2) - pow(L2, 2)) / (2 * L1 * L2));
    float q1_rad = atan2(yi, xi) - atan2(L2 * sin(q2_rad), L1 + L2 * cos(q2_rad));

    // Convertir a grados para los servomotores
    int angulo1 = q1_rad * 180.0 / PI; // PI = Constante con el valor de pi
    int angulo2 = q2_rad * 180.0 / PI;

    // 3. Enviar señales a los actuadores
    servo1.write(angulo1);
    servo2.write(angulo2);

    // Monitor serial para depuración
    Serial.print("Paso "); Serial.print(i);
    Serial.print(" -> X: "); Serial.print(xi);
    Serial.print(" Y: "); Serial.print(yi);
    Serial.print(" | Q1: "); Serial.print(angulo1);
    Serial.print(" Q2: "); Serial.println(angulo2);

    // 4. Retardo entre segmentos de 100ms
    delay(100);
  }
  Serial.println("Trayectoria completada.");
}

bool esAlcanzable(float x, float y) {
  float distanciaAlOrigen = sqrt(pow(x, 2) + pow(y, 2));
  // El punto es alcanzable si no está más lejos que la suma de los eslabones
  // y no está más cerca que la diferencia (zona muerta central)
  return (distanciaAlOrigen <= ALCANCE_MAX && distanciaAlOrigen >= abs(L1 - L2));
}