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
#define DISTANCIA_DIREITA 30.0

#define INTERVALO_SENSORES 150

#define INTERVALO_BLE 1000

#define INTERVALO_DHT 2500

#define TEMPO_APITO 300
#define TEMPO_RE 200
#define TEMPO_GIRO_DIREITA 700
#define TEMPO_GIRO_ESQUERDA 700

#define SERVICE_UUID "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

DHT dht(DHT_PIN, DHT_TYPE);

BLECharacteristic *sensorCharacteristic;


unsigned long ultimoSensor = 0;
unsigned long ultimoBLE = 0;
unsigned long ultimoDHT = 0;
unsigned long inicioManobra = 0;

float distancia1 = -1;
float distancia2 = -1;

float temperatura = NAN;
float umidade = NAN;

int leiturasDireita = 0;
int validasDireita = 0;
float menorDireita = 999;


enum Estado {
  PARADO,
  ANDANDO,
  VERIFICANDO_DIREITA,
  APITANDO,
  RECUANDO,
  PAUSA_RE,
  VIRANDO_DIREITA,
  VIRANDO_ESQUERDA,
  PAUSA_FINAL
};

Estado estado = PARADO;

bool escolherDireita = false;


void pararMotores()
{
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
}

void frente()
{
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);

  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
}

void re()
{
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);

  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
}


void virarDireita()
{
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);

  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
}

void virarEsquerda()
{
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);

  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
}


float medirDistancia(int trigPin, int echoPin)
{
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);

  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  unsigned long duracao =
    pulseIn(echoPin, HIGH, 30000);

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


void iniciarDesvio()
{
  pararMotores();

  estado = VERIFICANDO_DIREITA;

  leiturasDireita = 0;
  validasDireita = 0;
  menorDireita = 999;

  Serial.println("OBSTACULO NA FRENTE!");
  Serial.println("MOTORES PARADOS");
  Serial.println("VERIFICANDO DIREITA...");
}


void processarSensores()
{
  if (estado == VERIFICANDO_DIREITA)
  {
    leiturasDireita++;

    if (distancia2 >= 0)
    {
      validasDireita++;

      if (distancia2 < menorDireita)
      {
        menorDireita = distancia2;
      }
    }

    if (leiturasDireita >= 3)
    {
      if (validasDireita < 2)
      {
        pararMotores();
        estado = PARADO;

        Serial.println(
          "SENSOR DIREITO INSTAVEL - PARADO"
        );

        return;
      }

      escolherDireita =
        (menorDireita > DISTANCIA_DIREITA);

      if (escolherDireita)
      {
        Serial.println(
          "DIREITA LIVRE - DESVIO DIREITA"
        );
      }
      else
      {
        Serial.println(
          "DIREITA BLOQUEADA - DESVIO ESQUERDA"
        );
      }

      digitalWrite(BUZZER, HIGH);

      estado = APITANDO;
      inicioManobra = millis();
    }

    return;
  }

  if (estado != PARADO && estado != ANDANDO)
  {
    return;
  }

  if (distancia1 < 0)
  {
    pararMotores();
    estado = PARADO;

    Serial.println(
      "FALHA NO SENSOR FRONTAL - PARADO"
    );

    return;
  }

  if (distancia1 <= DISTANCIA_LIMITE)
  {
    iniciarDesvio();
    return;
  }

  if (estado != ANDANDO)
  {
    Serial.println("CAMINHO LIVRE - ANDANDO");
  }

  frente();
  estado = ANDANDO;
}


void atualizarManobra()
{
  unsigned long agora = millis();

  if (estado == APITANDO &&
      agora - inicioManobra >= TEMPO_APITO)
  {
    digitalWrite(BUZZER, LOW);

    if (escolherDireita)
    {
      Serial.println("VIRANDO DIREITA");

      virarDireita();

      estado = VIRANDO_DIREITA;
      inicioManobra = agora;
    }
    else
    {
      Serial.println("DANDO RE CURTA");

      re();

      estado = RECUANDO;
      inicioManobra = agora;
    }
  }

  else if (estado == RECUANDO &&
           agora - inicioManobra >= TEMPO_RE)
  {
    pararMotores();

    estado = PAUSA_RE;
    inicioManobra = agora;
  }

  else if (estado == PAUSA_RE &&
           agora - inicioManobra >= 200)
  {
    Serial.println("VIRANDO ESQUERDA");

    virarEsquerda();

    estado = VIRANDO_ESQUERDA;
    inicioManobra = agora;
  }

  else if (estado == VIRANDO_DIREITA &&
           agora - inicioManobra >=
           TEMPO_GIRO_DIREITA)
  {
    pararMotores();

    estado = PAUSA_FINAL;
    inicioManobra = agora;
  }

  else if (estado == VIRANDO_ESQUERDA &&
           agora - inicioManobra >=
           TEMPO_GIRO_ESQUERDA)
  {
    pararMotores();

    estado = PAUSA_FINAL;
    inicioManobra = agora;
  }

  else if (estado == PAUSA_FINAL &&
           agora - inicioManobra >= 250)
  {
    pararMotores();

    estado = PARADO;

    Serial.println("DESVIO FINALIZADO");
    Serial.println("VERIFICANDO NOVO CAMINHO");
  }
}


void enviarDados()
{
  bool obstaculo = false;

  if (distancia1 > 0 &&
      distancia1 <= DISTANCIA_LIMITE)
  {
    obstaculo = true;
  }

  if (distancia2 > 0 &&
      distancia2 <= DISTANCIA_LIMITE)
  {
    obstaculo = true;
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

  sensorCharacteristic->setValue(
    data.c_str()
  );

  sensorCharacteristic->notify();

  Serial.println(data);
}


void setup()
{
  Serial.begin(115200);

  pinMode(TRIG1, OUTPUT);
  pinMode(ECHO1, INPUT);

  pinMode(TRIG2, OUTPUT);
  pinMode(ECHO2, INPUT);

  digitalWrite(TRIG1, LOW);
  digitalWrite(TRIG2, LOW);

  pinMode(BUZZER, OUTPUT);
  digitalWrite(BUZZER, LOW);

  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  pararMotores();

  dht.begin();

  BLEDevice::init("ESP32-SENSOR");

  BLEServer *server = BLEDevice::createServer();

  BLEService *service =
    server->createService(SERVICE_UUID);

  sensorCharacteristic =
    service->createCharacteristic(
      CHARACTERISTIC_UUID,
      BLECharacteristic::PROPERTY_READ |
      BLECharacteristic::PROPERTY_NOTIFY
    );

  sensorCharacteristic->addDescriptor(
    new BLE2902()
  );

  service->start();

  BLEAdvertising *advertising =
    BLEDevice::getAdvertising();

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

  Serial.println("MOTORES AUTOMATICOS ATIVADOS");
  Serial.println("Sensor 1: FRENTE");
  Serial.println("Sensor 2: DIREITA");
  Serial.println("Sensores: 150 ms");
  Serial.println("Bluetooth: 1000 ms");
  Serial.println("DHT22: 2500 ms");

  ultimoSensor =
    millis() - INTERVALO_SENSORES;

  ultimoBLE =
    millis() - INTERVALO_BLE;

  ultimoDHT =
    millis() - INTERVALO_DHT;
}


void loop()
{
  atualizarManobra();

  unsigned long agora = millis();


  if (agora - ultimoSensor >=
      INTERVALO_SENSORES)
  {
    ultimoSensor = agora;

    distancia1 =
      medirDistancia(TRIG1, ECHO1);

    delay(50);

    distancia2 =
      medirDistancia(TRIG2, ECHO2);

    processarSensores();
  }


  agora = millis();

  if (agora - ultimoDHT >= INTERVALO_DHT)
  {
    ultimoDHT = agora;

    float t = dht.readTemperature();
    float h = dht.readHumidity();

    if (!isnan(t))
    {
      temperatura = t;
    }

    if (!isnan(h))
    {
      umidade = h;
    }
  }


  agora = millis();

  if (agora - ultimoBLE >= INTERVALO_BLE)
  {
    ultimoBLE = agora;

    enviarDados();
  }

  atualizarManobra();

  delay(2);
}