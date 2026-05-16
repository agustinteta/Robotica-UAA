#include <Servo.h>

const int trigPin = 10;
const int echoPin = 11;

long duration;
int distance;

Servo myServo;

void setup() {
  // put your setup code here, to run once:
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  Serial.begin(9600);
  myServo.attach(12);
}

void loop() {
  // put your main code here, to run repeatedly:
  for(int i=15; i<=165; i++){
    myServo.write(i);
    delay(30);
    distance = calculateDistance();

    Serial.print(i); //Esta forma es la más eficiente en memoria/cpu
    Serial.print(",");
    Serial.print(distance);
    Serial.print(".");
    
  }

}

int calculateDistance(){
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);

  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  duration = pulseIn(echoPin, HIGH);

  distance = duration * 0.034 / 2;

  return distance;
}