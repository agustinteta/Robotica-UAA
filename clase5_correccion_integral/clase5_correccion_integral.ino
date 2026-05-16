const float factorCalentamiento = 0.02;
//const float factorEnfriamiento = 0.1;
const float factorEnfriamiento = 0.005;

float tempVirtual = 25.0;
float sp = 100.0;     // Valor de referencia

float error;
float pwmActual; // Equivalente a U

const float kp = 2;

float u;

const int PIN_SALIDA=3;

const int ki=15;
float integral=0;
float up, ui, tant, tactual, deltaTiempo;

void setup() {
  pinMode(PIN_SALIDA,OUTPUT);
  Serial.begin(9600);
  tactual=0;
}

void loop() {
  // Cálculo del error
  error = sp - tempVirtual;

  // Control proporcional
  up = error * kp;
  
  tant=tactual;
  tactual=millis();
  deltaTiempo=tactual-tant;
  
  integral = integral + error*0.1;
  
  ui = ki*integral;
  u=up+ui;
  
  //pwmActual=constrain(u,0,255);
  pwmActual=u;
  analogWrite(PIN_SALIDA,pwmActual);

  // Simulación de la planta
  tempVirtual += (pwmActual * factorCalentamiento) - ((tempVirtual - 25.0) * factorEnfriamiento);
  Serial.println(tempVirtual);
  delay(100);
}