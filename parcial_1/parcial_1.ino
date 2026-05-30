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

// Variables para el filtro de promedio móvil
float lecturas[3] = {0, 0, 0};
int indiceLectura = 0;
float sumaDistancias = 0;
float distancia_prom = 0;

// Tiempos para el control del Sensor (No bloqueante)
unsigned long tiempoUltimoDisparo = 0;
const long intervaloLectura = 60; // Muestreo cada 60ms

// Variables para medir el ancho del pulso de ECO sin pulseIn()
unsigned long tiempoInicioEco = 0;
unsigned long tiempoFinEco = 0;
bool esperandoEco = false;
bool ecoRegistrado = false;

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
  unsigned long tiempoActual = millis();

  // 1. DISPARO ASÍNCRONO: Cada 60ms iniciamos un disparo
  if (tiempoActual - tiempoUltimoDisparo >= intervaloLectura && !esperandoEco) {
    tiempoUltimoDisparo = tiempoActual;
    
    // Generamos el pulso de disparo sin delayMicroseconds
    digitalWrite(PIN_DISPARO, HIGH);
    digitalWrite(PIN_DISPARO, LOW);
    
    esperandoEco = true;
    ecoRegistrado = false;
  }

  // 2. MÁQUINA DE ESTADOS PARA EL ECO (Reemplaza a pulseIn)
  // Estado A: Detectar flanco de subida (cuando arranca el eco)
  if (esperandoEco && !ecoRegistrado && digitalRead(PIN_ECO) == HIGH) {
    tiempoInicioEco = micros();
    ecoRegistrado = true;
  }
  
  // Estado B: Detectar flanco de bajada (cuando termina el eco)
  if (esperandoEco && ecoRegistrado && digitalRead(PIN_ECO) == LOW) {
    tiempoFinEco = micros();
    esperandoEco = false; // Terminamos la medición actual
    
    // Calcular el tiempo transcurrido en microsegundos
    long duracionEco = tiempoFinEco - tiempoInicioEco;
    float distanciaCalculada = duracionEco / 58.3;

    // --- APLICACIÓN DEL FILTRO DE PROMEDIO MÓVIL ---
    sumaDistancias = sumaDistancias - lecturas[indiceLectura];
    lecturas[indiceLectura] = distanciaCalculada;
    sumaDistancias = sumaDistancias + lecturas[indiceLectura];
    indiceLectura = (indiceLectura + 1) % 3;

    distancia_prom = sumaDistancias / 3.0;

    // --- CONTROL DE VELOCIDAD SEGÚN DISTANCIA ---
    if (distancia_prom > 50.0) {
      analogWrite(PIN_ENA, VELOCIDAD_MAX);
      analogWrite(PIN_ENB, VELOCIDAD_MAX);
    } 
    else if (distancia_prom >= 20.0 && distancia_prom <= 50.0) {
      analogWrite(PIN_ENA, VELOCIDAD_MED);
      analogWrite(PIN_ENB, VELOCIDAD_MED);
    } 
    else {
      analogWrite(PIN_ENA, VELOCIDAD_STOP);
      analogWrite(PIN_ENB, VELOCIDAD_STOP);
      detenerMotores();
    }
  }

  // Protección por si el pulso se pierde (Timeout asíncrono de 12ms)
  if (esperandoEco && (micros() - tiempoInicioEco > 12000) && ecoRegistrado) {
    esperandoEco = false;
    // Forzamos un valor alto para no activar el freno por error
    sumaDistancias = sumaDistancias - lecturas[indiceLectura];
    lecturas[indiceLectura] = 100.0;
    sumaDistancias = sumaDistancias + lecturas[indiceLectura];
    indiceLectura = (indiceLectura + 1) % 3;
  }
}

// --- INTERRUPCIONES DE DIRECCIÓN (Se mantienen intactas y ultra veloces) ---
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

// --- FUNCIONES DE MOVIMIENTO ---
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