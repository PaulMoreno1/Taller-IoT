#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <ESP32Servo.h>

#define MAXPOSITION     90
#define MINPOSITION      1
#define MAXTEMPERATURE  30
#define MINTEMPERATURE  15

#define SERVO   13

WiFiClient ESPClient;
PubSubClient client(ESPClient);
Servo motor;
ESP32PWM servoPWM;

IPAddress espIPAddress (10,107,250,2);
IPAddress subnetMask   (255,255,0,0);
IPAddress gateway      (10,107,255,254);
IPAddress dns          (10,107,255,254);

const char* SSID        = "NET RESIDENTES";
const char* PASSWORD    = "ITLADN01";
const char* MQTTSERVER  = "10.107.100.54"; //dirección IP

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
  ESP32PWM::allocateTimer(0);
  motor.setPeriodHertz(50);
  motor.attach(SERVO);
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
    int position = (MAXPOSITION - MINPOSITION) / (MAXTEMPERATURE - MINTEMPERATURE) * (temperature - MINTEMPERATURE) + MINPOSITION;
    Serial.printf("p: %i\n", position);
    motor.write(position);
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
  Serial.printf("Tópico: %s\n", topic);
  Serial.print("Mensaje: ");
  String temporalMessage;

  for(int i = 0; i < length; i++)
  {
    Serial.print((char)message[i]);
    temporalMessage += (char)message[i];
  }
  Serial.println();

  if(String(topic) == temperatureTopic)
  {
    temperature = atof(temporalMessage.c_str());
    delay(100);
  }

  else if(String(topic) == humidityTopic)
  {
    humidity = atof(temporalMessage.c_str());
    delay(100);
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
      client.subscribe("ESP32/#");
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