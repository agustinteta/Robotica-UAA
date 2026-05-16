// Pines Sensor Ultrasónico
const int PIN_DISPARO = 5;
const int PIN_ECO = 4;

// Pines Puente H (Motores)
const int PIN_MOTOR_1_1 = 13;
const int PIN_MOTOR_1_2 = 14;
const int PIN_MOTOR_2_1 = 16;
const int PIN_MOTOR_2_2 = 17;

float distancia;

void setup() {
  Serial.begin(9600);
  
  // Configuración de pines
  pinMode(PIN_DISPARO, OUTPUT);
  pinMode(PIN_ECO, INPUT);
  
  pinMode(PIN_MOTOR_1_1, OUTPUT);
  pinMode(PIN_MOTOR_1_2, OUTPUT);
  pinMode(PIN_MOTOR_2_1, OUTPUT);
  pinMode(PIN_MOTOR_2_2, OUTPUT);
  
  Serial.println("Robot iniciado...");
}

void loop() {
  distancia = leerDistancia();
  Serial.print("Distancia: ");
  Serial.print(distancia);
  Serial.println(" cm");

  if (distancia > 0 && distancia <= 30) {
    // 1. Obstáculo detectado: Retroceder 1 segundo
    Serial.println("Obstaculo detectado! Retrocediendo...");
    moverAtras();
    delay(1000);
    
    // 2. Girar a la izquierda (ángulo < 45°)
    // Para girar a la izquierda: motor derecho adelante, motor izquierdo atras
    Serial.println("Girando a la izquierda...");
    girarIzquierda();
    delay(250); // Ajusta este tiempo para que el giro sea < 45 grados
    
    detenerMotores();
    delay(200); // Pequeña pausa de estabilidad
  } else {
    // Si no hay obstáculos, seguir adelante
    moverAdelante();
  }
  
  delay(50); // Pequeña espera para estabilidad del sensor
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
  
  long tiempo = pulseIn(PIN_ECO, HIGH);
  float d = tiempo / 58.3; // Conversión a cm
  return d;
}