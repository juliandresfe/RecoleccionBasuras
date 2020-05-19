/*
 HC-SR04 conexiones:
  VCC al arduino 5v 
  GND al arduino GND
  Echo al Arduino pin 6 
  Trig al Arduino pin 7
   */
 
#define Pin_echo 6
#define Pin_trig 7
long duracion, distancia; 
   
void setup() {                
  Serial.begin (9600);          // inicializa el puerto serial a 9600 baudios
  pinMode(Pin_echo, INPUT);     // define el pin 6 como entrada (echo)
  pinMode(Pin_trig, OUTPUT);    // define el pin 7 como salida  (triger)
  }
  
void loop() {
  
  digitalWrite(Pin_trig, LOW);
  delayMicroseconds(2);
  digitalWrite(Pin_trig, HIGH);   // genera el pulso de triger por 10ms
  delayMicroseconds(10);
  digitalWrite(Pin_trig, LOW);
  
  duracion = pulseIn(Pin_echo, HIGH);
  distancia = (duracion/2) / 29;            // calcula la distancia en centimetros
  
  if (distancia >= 500 || distancia <= 0){  // si la distancia es mayor a 500cm o menor a 0cm 
    Serial.println("---");                  // no mide nada
  }
  else {
    Serial.print(distancia);           // envia el valor de la distancia por el puerto serial
    Serial.println("cm");              // le coloca a la distancia los centimetros "cm"
  } 
  
   if (distancia <= 5 && distancia >= 1){
    Serial.println("El contenedor está lleno");         // envia la palabra Alarma por el puerto serial
  }
  delay(400);                                // espera 400ms para que se logre ver la distancia en la consola
}
