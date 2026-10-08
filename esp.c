#include <DHT.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

#define DHT_PIN 5
#define DHT_TYPE DHT22

#define ECHO1 22
#define TRIG1 23

#define ECHO2 18
#define TRIG2 19

#define BUZZER 17

#define IN1 15
#define IN2 2
#define IN3 4
#define IN4 16

#define DISTANCIA_LIMITE 20.0

#define SERVICE_UUID "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

DHT dht(DHT_PIN, DHT_TYPE);

BLECharacteristic *sensorCharacteristic;

float medirDistancia(int trigPin, int echoPin)
{
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);

  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  long duracao = pulseIn(echoPin, HIGH, 30000);

  if (duracao == 0)
  {
    return -1;
  }

  float distancia = duracao / 58.0;

  if (distancia < 2 || distancia > 400)
  {
    return -1;
  }

  return distancia;
}

void setup()
{
  Serial.begin(115200);

  pinMode(TRIG1, OUTPUT);
  pinMode(ECHO1, INPUT);

  pinMode(TRIG2, OUTPUT);
  pinMode(ECHO2, INPUT);

  pinMode(BUZZER, OUTPUT);
  digitalWrite(BUZZER, LOW);

  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);

  dht.begin();

  BLEDevice::init("ESP32-SENSOR");

  BLEServer *server = BLEDevice::createServer();

  BLEService *service = server->createService(SERVICE_UUID);

  sensorCharacteristic = service->createCharacteristic(
    CHARACTERISTIC_UUID,
    BLECharacteristic::PROPERTY_READ |
    BLECharacteristic::PROPERTY_NOTIFY
  );

  sensorCharacteristic->addDescriptor(
    new BLE2902()
  );

  service->start();

  BLEAdvertising *advertising = BLEDevice::getAdvertising();

  advertising->addServiceUUID(SERVICE_UUID);
  advertising->setScanResponse(true);
  advertising->setMinPreferred(0x06);
  advertising->setMinPreferred(0x12);

  BLEDevice::startAdvertising();

  delay(2000);

  Serial.println("=================================");
  Serial.println(" ESP32 SENSOR BLE");
  Serial.println("=================================");
  Serial.println("Nome: ESP32-SENSOR");
  Serial.println("BLE aguardando conexao...");
}

void loop()
{
  float distancia1 = medirDistancia(TRIG1, ECHO1);

  delay(50);

  float distancia2 = medirDistancia(TRIG2, ECHO2);

  float temperatura = dht.readTemperature();
  float umidade = dht.readHumidity();

  bool obstaculo = false;

  if (distancia1 > 0 && distancia1 <= DISTANCIA_LIMITE)
  {
    obstaculo = true;
  }

  if (distancia2 > 0 && distancia2 <= DISTANCIA_LIMITE)
  {
    obstaculo = true;
  }

  if (obstaculo)
  {
    digitalWrite(BUZZER, HIGH);
    delay(150);
    digitalWrite(BUZZER, LOW);
  }
  else
  {
    digitalWrite(BUZZER, LOW);
  }

  String data = "{";

  if (isnan(temperatura))
  {
    data += "\"temperature\":null,";
  }
  else
  {
    data += "\"temperature\":";
    data += String(temperatura, 2);
    data += ",";
  }

  if (isnan(umidade))
  {
    data += "\"humidity\":null,";
  }
  else
  {
    data += "\"humidity\":";
    data += String(umidade, 2);
    data += ",";
  }

  if (distancia1 < 0)
  {
    data += "\"distance1\":null,";
  }
  else
  {
    data += "\"distance1\":";
    data += String(distancia1, 2);
    data += ",";
  }

  if (distancia2 < 0)
  {
    data += "\"distance2\":null,";
  }
  else
  {
    data += "\"distance2\":";
    data += String(distancia2, 2);
    data += ",";
  }

  data += "\"obstacle\":";
  data += obstaculo ? "true" : "false";
  data += "}";

  sensorCharacteristic->setValue(data.c_str());
  sensorCharacteristic->notify();

  Serial.println(data);

  delay(1000);
}