const int PIN_DISPARO=5;
const int PIN_ECO=4;
const int velocidadSonido=343;
int tiempoEco;
float d;

void setup()
{
  pinMode(PIN_DISPARO, OUTPUT);
  pinMode(PIN_ECO, INPUT);
  Serial.begin(9600);
}

void loop()
{
  disparo();
  tiempoEco = pulseIn(PIN_ECO, HIGH); // TIEMPO DE REBOTE EN US
  d=tiempoEco/58.3;// distancia en cm
  Serial.println(d);
}

void disparo(){
  digitalWrite(PIN_DISPARO, LOW);
  delayMicroseconds(10);
  digitalWrite(PIN_DISPARO, HIGH);
  delayMicroseconds(10); 
  digitalWrite(PIN_DISPARO, LOW);
}