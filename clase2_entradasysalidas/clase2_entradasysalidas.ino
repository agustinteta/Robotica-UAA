/*
1) LED PRENDIDO solo cuando el usuario está presionando el pulsador.
LED APAGADO solo cuando el usuario deja de presionar el pulsador.
*/

const int PIN_PULSADOR = 2;
const int PIN_LED = 13;

void setup() {
  pinMode(PIN_PULSADOR, INPUT);
  pinMode(PIN_LED, OUTPUT);
}

void loop() {
  int estadoPulsador = digitalRead(PIN_PULSADOR);
  digitalWrite(PIN_LED, estadoPulsador);
}

/*
2) LED PRENDIDO/APAGADO pero no hace falta mantener presionado el pulsador,
es decir, cada vez que se presiona el pulsador la salida tiene que cambiar de
estado (si el led estaba prendido, tiene que apagarse y viceversa). Tener en
cuenta el tiempo de rebote del pulsador.
*/

const int PIN_PULSADOR = 2;
const int PIN_LED = 13;

int estadoLed = LOW;
int ultimoEstadoBoton = LOW;
unsigned long ultimoTiempoRebote = 0;
unsigned long retrasoRebote = 50; // Tiempo en milisegundos

void setup() {
  pinMode(PIN_PULSADOR, INPUT);
  pinMode(PIN_LED, OUTPUT);
}

void loop() {
  int lectura = digitalRead(PIN_PULSADOR);

  if (lectura != ultimoEstadoBoton) {
    ultimoTiempoRebote = millis();
  }

  if ((millis() - ultimoTiempoRebote) > retrasoRebote) {
    if (lectura == HIGH && estadoLed == LOW) { // Se detecta el flanco de subida
        estadoLed = HIGH;
    } else if (lectura == HIGH && estadoLed == HIGH) {
        estadoLed = LOW;
    }
  }

  digitalWrite(PIN_LED, estadoLed);
  ultimoEstadoBoton = lectura;
}

/*
3) Ídem al punto 2 pero utilizando interrupciones
*/
const int PIN_PULSADOR = 2;
const int PIN_LED = 13;

volatile bool estadoLed = false;
unsigned long tiempoUltimaInterrupcion = 0;

void setup() {
  pinMode(PIN_PULSADOR, INPUT_PULLUP); // INPUT_PULLUP es una constante interna
  pinMode(PIN_LED, OUTPUT);
  
  // Configuramos la interrupción en el pin 2
  // RISING: se activa cuando el botón pasa de LOW a HIGH
  attachInterrupt(digitalPinToInterrupt(PIN_PULSADOR), cambiarEstado, RISING);
}

void loop() {
  // El loop queda libre para otras tareas
  digitalWrite(PIN_LED, estadoLed);
}

// Función que se ejecuta cuando se presiona el botón
void cambiarEstado() {
  unsigned long tiempoInterrupcion = millis();
  
  // Antirrebote por software dentro de la interrupción
  if (tiempoInterrupcion - tiempoUltimaInterrupcion > 200) {
    estadoLed = !estadoLed;
    tiempoUltimaInterrupcion = tiempoInterrupcion;
  }
}