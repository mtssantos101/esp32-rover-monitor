
import asyncio
from bleak import BleakClient, BleakScanner

DEVICE_NAME = "ESP32-SENSOR"
SERVICE_UUID = "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
CHAR_UUID = "beb5483e-36e1-4688-b7f5-ea07361b26a8"


async def main():
    print("[BLE] Procurando ESP32...", flush=True)

    devices = await BleakScanner.discover(
        timeout=15,
        return_adv=True
    )

    device = None

    for address, (dev, adv) in devices.items():
        name = adv.local_name or dev.name or ""
        services = [s.lower() for s in (adv.service_uuids or [])]

        if name == DEVICE_NAME or SERVICE_UUID.lower() in services:
            device = dev
            break

    if device is None:
        print("[ERRO] ESP32 não encontrado.")
        return

    print(f"[BLE] Encontrado: {device.address}", flush=True)

    try:
        async with BleakClient(device, timeout=15) as client:
            print(f"[BLE] Conectado: {client.is_connected}", flush=True)

            def callback(sender, data):
                print(
                    "[BLE] Dados recebidos:",
                    data.decode("utf-8", errors="replace"),
                    flush=True
                )

            await client.start_notify(CHAR_UUID, callback)
            print("[BLE] Aguardando notificações por 30 segundos...", flush=True)
            await asyncio.sleep(30)

    except Exception as error:
        print(f"[ERRO BLE] {type(error).__name__}: {error}", flush=True)


asyncio.run(main())