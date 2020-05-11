#include <ETH.h>
#include <WiFi.h>
#include <WiFiAP.h>
#include <WiFiClient.h>
#include <WiFiGeneric.h>
#include <WiFiMulti.h>
#include <WiFiScan.h>
#include <WiFiServer.h>
#include <WiFiSTA.h>
#include <WiFiType.h>
#include <WiFiUdp.h>

#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"
#include "Adafruit_Sensor.h"


//DHT VARIABLES
float  humidityValue = 0;
float temperatureValue = 0;
float gasValue = 0;
const int DHTPin = 23;



#define AIO_SERVER      "io.adafruit.com"
#define AIO_SERVERPORT  1883
#define AIO_USERNAME  "sjmunozb"
#define AIO_KEY  "aio_HByN27yAc4LZHSzdMsk1KShQ5wIO"
#define DHTTYPE DHT11   // DHT 11

WiFiClient client;

Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);
Adafruit_MQTT_Publish   temperature = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/temperature");
Adafruit_MQTT_Publish   humidity = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/humidity");
Adafruit_MQTT_Subscribe gasvalvemanualstatus = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/gasvalvemanualstatus");

  
// the setup function runs once when you press reset or power the board
void setup() {
  

  Serial2.begin(115200);
  Serial.begin(115200);

  // Now set up two tasks to run independently.

  xTaskCreate(
    Task_Uart,
    "UART",
    5000,            /* Stack size in bytes. */
    NULL,             /* Parameter passed as input of the task */
    6,                /* Priority of the task. */
    NULL);            /* Task handle. */

  xTaskCreate(
    Task_humedadYtemp,
    "humedadYtemp",
    2000,            /* Stack size in bytes. */
    NULL,             /* Parameter passed as input of the task */
    10,                /* Priority of the task. */
    NULL);            /* Task handle. */


  xTaskCreate(
    Task_Enviar,
    "Enviar",
    5000,            /* Stack size in bytes. */
    NULL,             /* Parameter passed as input of the task */
    1,                /* Priority of the task. */
    NULL);            /* Task handle. */


  xTaskCreate(
    Task_conectar,
    "Conectar",
    4000,            /* Stack size in bytes. */
    NULL,             /* Parameter passed as input of the task */
    4,                /* Priority of the task. */
    NULL);            /* Task handle. */

  xTaskCreate(
    Task_mqtt,
    "mqtt",
    2000,            /* Stack size in bytes. */
    NULL,             /* Parameter passed as input of the task */
    5,                /* Priority of the task. */
    NULL);            /* Task handle. */

//  xTaskCreate(
//    Task_estado,
//    "estado",
//    5000,            /* Stack size in bytes. */
//    NULL,             /* Parameter passed as input of the task */
//    2,                /* Priority of the task. */
//    NULL);            /* Task handle. */
//
//  xTaskCreate(
//    Task_reset,
//    "reset",
//    5000,            /* Stack size in bytes. */
//    NULL,             /* Parameter passed as input of the task */
//    3,                /* Priority of the task. */
//    NULL);            /* Task handle. */


  // Now the task scheduler, which takes over control of scheduling individual tasks, is automatically started.
}

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
    
  if(Serial.read() == '0')
  {
    
    if (temperature.publish(temperatureValue)) {
      Serial.println("Temperature data sent: ");
      Serial.println(temperatureValue);
    } else {
      Serial.println("T");
      Serial.println(temperatureValue);
    }
    if (humidity.publish(humidityValue)) {
      Serial.println("Humidity data sent: ");
      Serial.println(humidityValue);
    } else {
      Serial.println("H");
      Serial.println(humidityValue);
    }
  }
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}

int ret;
int retries = 3;

void Task_mqtt(void * pvParameters)
{
  while (1) {

    if (mqtt.connected()) {
      Serial.println("MQTT conected");
      vTaskDelay(10000 / portTICK_PERIOD_MS);
    }
    while ((ret = mqtt.connect()) != 0) {
      mqtt.disconnect();
      delay(2000);
      Serial.println("CONECTING");
      retries--;

      if (retries == 0) {
        retries=3;
        Serial.println("MQTT Error");
        vTaskDelay(8000 / portTICK_PERIOD_MS);
      }
    }
    vTaskDelay(1000 / portTICK_PERIOD_MS);
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
      WiFi.begin("Embebidos-Hogar_Inteligente", "bonnechance");
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



void Task_humedadYtemp(void * pvParameters)
{
  while (1) {
    
    vTaskDelay(10000 / portTICK_PERIOD_MS);
  }
}

int estado=0;

void Task_Enviar(void * pvParameters)
{
  while (1) {
    Adafruit_MQTT_Subscribe *subscription_gasValve;
    while ((subscription_gasValve = mqtt.readSubscription(100))) {
      if (subscription_gasValve == &gasvalvemanualstatus) {
        Serial.println(((char *)gasvalvemanualstatus.lastread));
        String inString = (char *)gasvalvemanualstatus.lastread;
        if (inString == "ON") {
          Serial.print("ON");
          Serial2.print("1");
          digitalWrite(2, LOW);

          vTaskDelay(1000 / portTICK_PERIOD_MS);
        } else {
          Serial.print("OFF");
          Serial2.print("0");
          digitalWrite(2, HIGH);
          vTaskDelay(1000 / portTICK_PERIOD_MS);
        }
      }
    }
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}
