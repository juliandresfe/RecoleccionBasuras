
#include <FirebaseESP32.h>
#include <FirebaseESP32HTTPClient.h>
#include <FirebaseJson.h>


#include <WiFi.h>
#include <WiFiClient.h>
#include <WiFiServer.h>



// Set these to run example.

#define WIFI_SSID "FAMILIA RODRIGUEZ" // nombre del wifi
#define WIFI_PASSWORD "santiago11" // password del wifi
#define FIREBASE_HOST "monitoreo-de-basuras.firebaseio.com"  //poner direccion de la database de Firebase sin https://  ni el ultimo /
#define FIREBASE_AUTH "cq2e3K8bnvr4F3HtFA4DpABNJtGS9xMUfUbf3E7i"// poner codigo secreo. Configuracion proyecto-cuentas servicio-secretos de la base de datos


// ultrasonico
#define Pin_echo 6
#define Pin_trig 7
int duracion, distancia;
bool full=false; 


WiFiClient client;

FirebaseData firebaseData;

// the setup function runs once when you press reset or power the board
void setup() {
  
  Serial.begin(115200);

  pinMode(Pin_echo, INPUT);     // define el pin 6 como entrada (echo)
  pinMode(Pin_trig, OUTPUT);    // define el pin 7 como salida  (triger)
  
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
 

  // Now set up two tasks to run independently.

//  xTaskCreate(
//    Task_Uart,
//    "UART",
//    5000,            /* Stack size in bytes. */
//    NULL,             /* Parameter passed as input of the task */
//    5,                /* Priority of the task. */
//    NULL);            /* Task handle. */

  xTaskCreate(
    Task_Ultrasonico,
    "Ultrasonico",
    2000,            /* Stack size in bytes. */
    NULL,             /* Parameter passed as input of the task */
    2,                /* Priority of the task. */
    NULL);            /* Task handle. */


  xTaskCreate(
    Task_Firebase_update,
    "Firebase_update",
    100000,            /* Stack size in bytes. */
    NULL,             /* Parameter passed as input of the task */
    3,                /* Priority of the task. */
    NULL);            /* Task handle. */


  xTaskCreate(
    Task_conectar,
    "Conectar",
    4000,            /* Stack size in bytes. */
    NULL,             /* Parameter passed as input of the task */
    1,                /* Priority of the task. */
    NULL);            /* Task handle. */

//  xTaskCreate(
//    Task_Firebase_enviar,
//    "Firebase_enviar",
//    2000,            /* Stack size in bytes. */
//    NULL,             /* Parameter passed as input of the task */
//    4,                /* Priority of the task. */
//    NULL);            /* Task handle. */



  // Now the task scheduler, which takes over control of scheduling individual tasks, is automatically started.
}


int n = 0;






void loop()
{
  delay(5000);
}

/*--------------------------------------------------*/
/*---------------------- Tasks ---------------------*/
/*--------------------------------------------------*/
void Task_Uart(void * pvParameters)
{
  while (1) {
    
      vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}


int ret;
int retries = 3;


void Task_Firebase_enviar(void * pvParameters)
{
  while (1) {
if (Firebase.getFile(firebaseData, StorageType::SPIFFS, "/test/file_data", "/test.txt"))
{
  //SPIFFS.begin(); //not need to begin again due to it has been called in function.
  File file = SPIFFS.open("/test.txt", "r");

  while (file.available())
  {     
    Serial.print(file.read(), HEX);     
  }    
  file.close();
  Serial.println();

} else {
  Serial.println(firebaseData.fileTransferError());
}

Firebase.deleteNode(firebaseData, "/test/append");


   }
}


void Task_conectar(void * pvParameters)
{
  while (1) {
    if (WiFi.status() == WL_CONNECTED)
    {
      Serial.println("Connected");
      vTaskDelay(10000 / portTICK_PERIOD_MS);
    }  else {
      Serial.println("START");
      WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
      while ((!(WiFi.status() == WL_CONNECTED))) {
        delay(300);
        Serial.print(".");
      }
      Serial.println("Connected");
      Serial.println("Your IP is");
      Serial.println((WiFi.localIP()));
      vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
  }
}



void Task_Ultrasonico(void * pvParameters)
{
  while (1) {
    digitalWrite(Pin_trig, LOW);
  vTaskDelay(1 / portTICK_PERIOD_MS);
  digitalWrite(Pin_trig, HIGH);   // genera el pulso de triger por 10ms
  vTaskDelay(10 / portTICK_PERIOD_MS);
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
    full=true;
    }
    else{
      full=false;
    }
  vTaskDelay(10000 / portTICK_PERIOD_MS);
  }
}

int estado=0;

void Task_Firebase_update(void * pvParameters)
{
  while (1) {
FirebaseJson updateData;
updateData.set("Full",full);
updateData.set("Nivel",distancia);

if (Firebase.updateNode(firebaseData, "/Contenedores/000002", updateData)) {

  Serial.println(firebaseData.dataPath());

  Serial.println(firebaseData.dataType());

  Serial.println(firebaseData.jsonString()); 

} else {
  Serial.println(firebaseData.errorReason());
}
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}
