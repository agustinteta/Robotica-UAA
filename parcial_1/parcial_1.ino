// Pines Sensor Ultrasonido
const int PIN_DISPARO = 13;
const int PIN_ECO = 12;

// Pines Puente H (Dirección de Motores)
const int PIN_MOTOR_1_1 = 11;
const int PIN_MOTOR_1_2 = 10;
const int PIN_MOTOR_2_1 = 9;
const int PIN_MOTOR_2_2 = 8;

// Pines PWM para Control de Velocidad
const int PIN_ENA = 5;
const int PIN_ENB = 6;

// Pines de Interrupción (Sensores de Línea)
const int PIN_SENSOR_IZQ = 2;
const int PIN_SENSOR_DER = 3;

// Constantes de Velocidad
const int VELOCIDAD_MAX = 255;
const int VELOCIDAD_MED = 76;
const int VELOCIDAD_STOP = 0;

// Variables globales compartidas para el filtro de promedio
float lecturas[3] = {0, 0, 0};
int indiceLectura = 0;
float sumaDistancias = 0;
float distanciaProm = 0;

const long intervaloLectura = 60; // Muestreo cada 60ms

void setup() {
  pinMode(PIN_DISPARO, OUTPUT);
  pinMode(PIN_ECO, INPUT);

  pinMode(PIN_MOTOR_1_1, OUTPUT);
  pinMode(PIN_MOTOR_1_2, OUTPUT);
  pinMode(PIN_MOTOR_2_1, OUTPUT);
  pinMode(PIN_MOTOR_2_2, OUTPUT);

  pinMode(PIN_ENA, OUTPUT);
  pinMode(PIN_ENB, OUTPUT);

  pinMode(PIN_SENSOR_IZQ, INPUT);
  pinMode(PIN_SENSOR_DER, INPUT);

  moverAdelante();

  attachInterrupt(digitalPinToInterrupt(PIN_SENSOR_IZQ), moduloIzq, CHANGE);
  attachInterrupt(digitalPinToInterrupt(PIN_SENSOR_DER), moduloDer, CHANGE);
}

void loop() {
  medirDistancia();
}

// Función para medir la distancia utilizando el sensor ultrasónico
void medirDistancia() {
  static unsigned long tiempoUltimoDisparo = 0;
  static unsigned long tiempoInicioEco = 0;
  static unsigned long tiempoFinEco = 0;
  static bool esperandoEco = false;
  static bool ecoRegistrado = false;

  unsigned long tiempoActual = millis();

  // Disparo del sensor cada 60ms
  if (tiempoActual - tiempoUltimoDisparo >= intervaloLectura && !esperandoEco) {
    tiempoUltimoDisparo = tiempoActual;
    digitalWrite(PIN_DISPARO, HIGH);
    digitalWrite(PIN_DISPARO, LOW);
    esperandoEco = true;
    ecoRegistrado = false;
  }

  // Captura flanco de subida (Inicio del Eco)
  if (esperandoEco && !ecoRegistrado && digitalRead(PIN_ECO) == HIGH) {
    tiempoInicioEco = micros();
    ecoRegistrado = true;
  }
  
  // Captura flanco de bajada (Fin del Eco exitoso)
  if (esperandoEco && ecoRegistrado && digitalRead(PIN_ECO) == LOW) {
    tiempoFinEco = micros();
    esperandoEco = false; 
    
    long duracionEco = tiempoFinEco - tiempoInicioEco;
    float distanciaCalculada = duracionEco / 58.3;

    actualizarFiltroYVelocidad(distanciaCalculada);
  }

  // Protección por Timeout (20ms sin respuesta)
  if (esperandoEco && ecoRegistrado && (micros() - tiempoInicioEco > 20000)) {
    esperandoEco = false;
    actualizarFiltroYVelocidad(0); // Detener por falta de respuesta
  }
}

// Función para actualizar el filtro de promedio móvil y controlar la velocidad
void actualizarFiltroYVelocidad(float distanciaCalculada) {
  // Aplicación del filtro de promedio móvil
  sumaDistancias = sumaDistancias - lecturas[indiceLectura];
  lecturas[indiceLectura] = distanciaCalculada;
  sumaDistancias = sumaDistancias + lecturas[indiceLectura];
  indiceLectura = (indiceLectura + 1) % 3;

  distanciaProm = sumaDistancias / 3.0;

  // Control de velocidad y frenado según el promedio
  if (distanciaProm > 50.0) {
    analogWrite(PIN_ENA, VELOCIDAD_MAX);
    analogWrite(PIN_ENB, VELOCIDAD_MAX);
  } 
  else if (distanciaProm >= 20.0 && distanciaProm <= 50.0) {
    analogWrite(PIN_ENA, VELOCIDAD_MED);
    analogWrite(PIN_ENB, VELOCIDAD_MED);
  } 
  else {
    analogWrite(PIN_ENA, VELOCIDAD_STOP);
    analogWrite(PIN_ENB, VELOCIDAD_STOP);
    detenerMotores();
  }
}

// Interrupciones para los sensores de línea
void moduloIzq() {
  if (digitalRead(PIN_SENSOR_IZQ) == HIGH) {
    girarIzquierda();
  } else {
    moverAdelante();
  }
}

void moduloDer() {
  if (digitalRead(PIN_SENSOR_DER) == HIGH) {
    girarDerecha();
  } else {
    moverAdelante();
  }
}

// Funciones de movimiento con los motores y el puente H
void moverAdelante() {
  digitalWrite(PIN_MOTOR_1_1, HIGH);
  digitalWrite(PIN_MOTOR_1_2, LOW);
  digitalWrite(PIN_MOTOR_2_1, HIGH);
  digitalWrite(PIN_MOTOR_2_2, LOW);
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