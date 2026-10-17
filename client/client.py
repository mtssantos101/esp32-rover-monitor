import asyncio
import json
import requests
from bleak import BleakClient, BleakScanner

DEVICE_NAME = "ESP32-SENSOR"
CHARACTERISTIC_UUID = "beb5483e-36e1-4688-b7f5-ea07361b26a8"
SERVER_URL = "http://127.0.0.1:5000/api/sensors"


async def find_device():
    from bleak import BleakScanner

    SERVICE_UUID = "4fafc201-1fb5-459e-8fcc-c5c9c331914b"

    devices = await BleakScanner.discover(timeout=2000000, return_adv=True)

    for address, (device, adv) in devices.items():
        services = [s.lower() for s in (adv.service_uuids or [])]

        if (
            (adv.local_name or device.name) == "ESP32-SENSOR"
            or SERVICE_UUID.lower() in services
        ):
            print(f"Dispositivo encontrado: {device.address}")
            print(f"Nome: {adv.local_name or device.name}")
            return device

    return None


def handle_notification(sender, data):
    print("[BLE] Notificação recebida", flush=True)

    try:
        payload = json.loads(data.decode("utf-8"))
        print(f"[HTTP] Enviando: {payload}", flush=True)

        response = requests.post(
            SERVER_URL,
            json=payload,
            timeout=3
        )

        print(
            f"[HTTP] Resposta: {response.status_code}",
            flush=True
        )

    except Exception as error:
        print(f"[ERRO] {type(error).__name__}: {error}", flush=True)

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