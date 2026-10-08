# ESP32 Sensor Monitor

Projeto de monitoramento de sensores utilizando **ESP32 + Bluetooth Low Energy (BLE) + Python + WebSocket + CSV + Dashboard Web**.

O projeto foi desenvolvido de forma incremental. Atualmente, a parte de monitoramento pode ser executada **sem o ESP32**, utilizando um simulador que gera dados de temperatura e umidade.

Quando o ESP32 estiver disponível, o simulador poderá ser substituído pelo cliente BLE (`esp-client`).

---

## Arquitetura

Atualmente, o fluxo de dados é:

```text
┌──────────────────┐
│    Simulator     │
│                  │
│ Temperature      │
│ Humidity         │
└────────┬─────────┘
         │
         ▼
┌──────────────────────────┐
│       ESP32 Monitor      │
│                          │
│ Flask                    │
│ Flask-SocketIO           │
│                          │
│ ┌────────────┐           │
│ │    CSV     │           │
│ └────────────┘           │
│                          │
│ ┌────────────┐           │
│ │ WebSocket  │───────────┼──────┐
│ └────────────┘           │      │
└──────────────────────────┘      │
                                  ▼
                         ┌─────────────────┐
                         │     Browser     │
                         │                 │
                         │    Chart.js     │
                         │                 │
                         │  Dashboard      │
                         └─────────────────┘
```

A arquitetura planejada para a versão com ESP32 será:

```text
┌──────────────┐
│    ESP32     │
│              │
│   Sensors    │
└──────┬───────┘
       │
       │ BLE
       ▼
┌─────────────────┐
│  Python Client  │
│     Bleak       │
└────────┬────────┘
         │
         ▼
┌─────────────────┐
│ ESP32 Monitor   │
│                 │
│ CSV + WebSocket │
└────────┬────────┘
         │
         ▼
      Browser
```

---

# Estrutura do projeto

```text
sis-emb/
│
├── esp32-monitor/
│   │
│   ├── app.py
│   ├── simulator.py
│   ├── requirements.txt
│   ├── sensors.csv
│   │
│   └── templates/
│       └── index.html
│
└── esp-client/
    │
    ├── client.py
    ├── esp.c
    └── requirements.txt
```

## `esp32-monitor`

Responsável pelo servidor, armazenamento dos dados e dashboard.

### `app.py`

Servidor Flask responsável por:

* disponibilizar o dashboard;
* disponibilizar a API de sensores;
* executar o loop de aquisição;
* salvar os dados no CSV;
* enviar os dados em tempo real através de WebSocket.

### `simulator.py`

Simula os dados que futuramente serão enviados pelo ESP32.

Atualmente gera:

* temperatura;
* umidade;
* timestamp.

### `sensors.csv`

Arquivo utilizado para persistir as leituras.

Exemplo:

```csv
timestamp,temperature,humidity
2026-10-07T22:30:01,27.4,63.2
2026-10-07T22:30:02,27.5,63.8
2026-10-07T22:30:03,27.3,63.4
```

### `templates/index.html`

Dashboard web responsável pela visualização dos dados.

Utiliza:

* HTML;
* JavaScript;
* Chart.js;
* Socket.IO.

---

# `esp-client`

Responsável pela comunicação com o ESP32 através de Bluetooth Low Energy.

### `client.py`

Cliente Python que futuramente será responsável por:

1. procurar o ESP32;
2. estabelecer conexão BLE;
3. receber notificações;
4. interpretar os dados enviados pelo ESP32;
5. encaminhar os dados para o monitor.

### `esp.c`

Código que será executado no ESP32.

Atualmente o ESP32 deverá funcionar como um servidor BLE, disponibilizando uma característica para os dados dos sensores.

---

# Requisitos

## Computador

É necessário ter:

* Python 3.10 ou superior;
* `pip`;
* Bluetooth habilitado no computador;
* navegador web moderno.

Para verificar o Python:

```bash
python3 --version
```

Exemplo:

```text
Python 3.12.3
```

---

# 1. Clonar o projeto

Caso o projeto esteja em um repositório Git:

```bash
git clone <URL_DO_REPOSITORIO>
```

Entre no diretório:

```bash
cd sis-emb
```

---

# 2. Criar o ambiente virtual

É recomendado utilizar um ambiente virtual Python separado para cada aplicação.

## ESP32 Monitor

Entre no diretório:

```bash
cd esp32-monitor
```

Crie o ambiente virtual:

```bash
python3 -m venv .venv
```

Ative:

```bash
source .venv/bin/activate
```

No terminal deverá aparecer algo semelhante a:

```text
(.venv) user@computer:~/sis-emb/esp32-monitor$
```

---

# 3. Instalar as dependências do monitor

Com o ambiente virtual ativado:

```bash
pip install -r requirements.txt
```

As principais dependências são:

```text
Flask
Flask-SocketIO
```

Para verificar:

```bash
pip list
```

---

# 4. Executar o monitor

Ainda dentro de:

```text
esp32-monitor/
```

execute:

```bash
python app.py
```

O servidor será iniciado em:

```text
http://localhost:5000
```

Abra esse endereço no navegador.

---

# 5. Funcionamento do simulador

Atualmente o `app.py` gera dados simulados automaticamente.

Os dados são gerados aproximadamente a cada segundo:

```json
{
    "timestamp": "2026-10-07T22:30:01",
    "temperature": 27.4,
    "humidity": 63.2
}
```

Esses dados possuem dois destinos:

### Persistência

São armazenados em:

```text
sensors.csv
```

### Dashboard

São enviados através de WebSocket para o navegador.

Isso permite que o dashboard seja atualizado em tempo real sem precisar recarregar a página.

---

# 6. Dashboard

Após executar:

```bash
python app.py
```

abra:

```text
http://localhost:5000
```

O dashboard apresenta:

* temperatura atual;
* umidade atual;
* gráfico de temperatura;
* gráfico de umidade;
* atualização em tempo real.

O navegador recebe novos dados através do Socket.IO.

Fluxo:

```text
Python
  │
  │ sensor_data
  ▼
Socket.IO
  │
  ▼
Browser
  │
  ├── Temperatura
  ├── Umidade
  └── Gráficos
```

---

# 7. Cliente BLE

A pasta `esp-client` será utilizada quando o ESP32 estiver disponível.

Entre no diretório:

```bash
cd ../esp-client
```

Crie um ambiente virtual:

```bash
python3 -m venv .venv
```

Ative:

```bash
source .venv/bin/activate
```

Instale as dependências:

```bash
pip install -r requirements.txt
```

O cliente utiliza a biblioteca `bleak` para comunicação Bluetooth Low Energy.

---

# 8. Verificar dispositivos BLE

No Linux, o Bluetooth pode ser testado através do:

```bash
bluetoothctl
```

Dentro do programa:

```text
power on
scan on
```

Quando o ESP32 estiver ligado, deverá aparecer algo semelhante a:

```text
Device AA:BB:CC:DD:EE:FF ESP32-SENSOR
```

Para parar a busca:

```text
scan off
```

E sair:

```text
exit
```

---

# 9. Fluxo planejado com o ESP32

Quando o ESP32 estiver disponível, o fluxo será:

```text
ESP32
  │
  │ BLE Notification
  ▼
Python Client
  │
  │ JSON
  ▼
ESP32 Monitor
  │
  ├──────────────► sensors.csv
  │
  └──────────────► WebSocket
                         │
                         ▼
                      Browser
```

O ESP32 deverá enviar dados em um formato semelhante a:

```json
{
    "temperature": 27.4,
    "humidity": 63.2
}
```

O cliente Python será responsável por receber esses dados e encaminhá-los para o monitor.

---

# 10. Desenvolvimento

Para trabalhar no projeto:

## Terminal 1 — Monitor

```bash
cd esp32-monitor
source .venv/bin/activate
python app.py
```

## Terminal 2 — Cliente BLE

Quando o ESP32 estiver disponível:

```bash
cd esp-client
source .venv/bin/activate
python client.py
```

## Navegador

```text
http://localhost:5000
```

---

# 11. Tecnologias

### ESP32

* ESP32-WROOM-32
* C/C++
* Bluetooth Low Energy

### Backend

* Python
* Flask
* Flask-SocketIO
* Bleak

### Frontend

* HTML
* JavaScript
* Chart.js
* Socket.IO

### Persistência

* CSV

---

# Próximos passos

O projeto está sendo desenvolvido incrementalmente.

### Concluído

* [x] Estrutura inicial do projeto
* [x] Simulador de sensores
* [x] Persistência em CSV
* [x] Servidor Flask
* [x] Dashboard web
* [x] Gráficos com Chart.js
* [x] Atualização em tempo real via WebSocket

### Próximas etapas

* [ ] Implementar GATT Server no ESP32
* [ ] Implementar comunicação BLE
* [ ] Conectar `esp-client` ao ESP32
* [ ] Receber notificações BLE
* [ ] Substituir o simulador pelos dados reais
* [ ] Adicionar status da conexão BLE
* [ ] Adicionar timestamp da última leitura
* [ ] Melhorar o dashboard
* [ ] Avaliar substituição do CSV por banco de dados
* [ ] Avaliar integração com Grafana

---

# Execução rápida

Depois que tudo estiver configurado, para iniciar o dashboard:

```bash
cd esp32-monitor

source .venv/bin/activate

python app.py
```

Depois acesse:

```text
http://localhost:5000
```

O sistema começará a gerar dados simulados e o dashboard será atualizado automaticamente em tempo real.
