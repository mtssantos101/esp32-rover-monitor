import asyncio
import json
import requests
from bleak import BleakClient, BleakScanner

DEVICE_NAME = "ESP32-SENSOR"
CHARACTERISTIC_UUID = "beb5483e-36e1-4688-b7f5-ea07361b26a8"
SERVER_URL = "http://127.0.0.1:5000/api/sensors"

async def find_device():
    devices = await BleakScanner.discover()

    for device in devices:
        if device.name == DEVICE_NAME:
            return device

    return None

def handle_notification(sender, data):
    try:
        payload = json.loads(data.decode("utf-8"))

        response = requests.post(
            SERVER_URL,
            json=payload,
            timeout=5
        )

        print(payload)

        if response.status_code != 201:
            print(f"Erro HTTP: {response.status_code}")

    except Exception as error:
        print(f"Erro ao processar dados: {error}")

async def main():
    print("Procurando ESP32...")

    device = await find_device()

    if not device:
        print("ESP32 nao encontrado")
        return

    print(f"ESP32 encontrado: {device.address}")
    print("Conectando...")

    async with BleakClient(device) as client:
        print("Conectado ao ESP32")

        await client.start_notify(
            CHARACTERISTIC_UUID,
            handle_notification
        )

        print("Aguardando dados...")

        while True:
            await asyncio.sleep(1)

if __name__ == "__main__":
    asyncio.run(main())