#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>

#define DHTTYPE DHT22
#define DHTPIN  27

DHT sensor(DHTPIN, DHTTYPE);
WiFiClient ESPClient;
PubSubClient client(ESPClient);

IPAddress espIPAddress (10,107,250,1);
IPAddress subnetMask   (255,255,0,0);
IPAddress gateway      (10,107,255,254);
IPAddress dns          (10,107,255,254);

const char* SSID        = "NET RESIDENTES";
const char* PASSWORD    = "ITLADN01";
const char* MQTTSERVER  = ""; //dirección IP

const char* temperatureTopic  = "ESP32/temperature";
const char* humidityTopic     = "ESP32/humidity";

String str_temperature, str_humidity;
float temperature, humidity;
int MQTTPort = 1883;
long last;
int waitTime = 5000;

void beginWiFi();
void reconnect();
void callback(char* topic, byte* message, uint length);

void setup()
{
  Serial.begin(115200);
  sensor.begin();
  beginWiFi();
  client.setServer(MQTTSERVER, MQTTPort);
  client.setCallback(callback);
}

void loop() 
{
  if(!client.connected())
  {
    reconnect();
  }

  client.loop();

  long now = millis();
  if(now - last > waitTime)
  {
    temperature = sensor.readTemperature();
    humidity = sensor.readHumidity();
    str_temperature.concat(temperature);
    str_humidity.concat(humidity);
    last = now;
    client.publish(temperatureTopic, str_temperature.c_str());
    client.publish(humidityTopic, str_humidity.c_str());
    str_temperature = "";
    str_humidity = "";
  }
}

void beginWiFi()
{
  WiFi.config(espIPAddress, gateway, subnetMask, dns);
  WiFi.begin(SSID, PASSWORD);
  Serial.printf("Conectando a la red: %s\n", SSID);
  while(WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(500);
  }

  Serial.printf("\nConexión exitosa\n");
  Serial.print("Dirección IP: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* message, uint length)
{
  Serial.printf("Tópico: %c\n", topic);
  Serial.print("Mensaje: ");
  String temporalMessage;

  for(int i = 0; i < length; i++)
  {
    Serial.print((char)message[i]);
    temporalMessage += (char)message[i];
  }
}

void reconnect()
{
  int count = 0;
  while(!client.connected())
  {
    Serial.print("Conectando a MQTT\n");
    if(client.connect("Cliente"))
    {
      Serial.println("Conectado");
    }
    else
    {
      Serial.print("Conexión fallida, rc = ");
      Serial.print(client.state());
      Serial.println("\nIntentando reconectar en 5 s.\n");
      while(count < 5000)
      {
        delay(100);
        count += 100;
      }
      count = 0;
    }
  }
}