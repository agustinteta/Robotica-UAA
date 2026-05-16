const int PIN_NTC=A0;

const float BETA = 3950; // should match the Beta Coefficient of the thermistor

int valorLeido;
int analogValue;
float valorAnalogico;
float celsius;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
  valorLeido=analogRead(PIN_NTC);
  valorAnalogico=(5*valorLeido)/1023; // V para la Temp.
  Serial.println(valo));
  analogValue=analogRead(PIN_NTC);
  celsius = 1 / (log(1 / (1023. / analogValue - 1)) / BETA + 1.0 / 298.15) - 273.15; // Conversión V -> C°
  Serial.println(celsius);
}