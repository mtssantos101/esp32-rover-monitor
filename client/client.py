
import asyncio
import json
import requests

from bleak import BleakClient, BleakScanner

DEVICE_NAME = "ESP32-SENSOR"
DEVICE_ADDRESS = "08:D1:F9:D3:80:06"

SERVICE_UUID = "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
CHARACTERISTIC_UUID = "beb5483e-36e1-4688-b7f5-ea07361b26a8"

SERVER_URL = "http://127.0.0.1:5000/api/sensors"

RETRY_INTERVAL = 3
MAX_RETRY_INTERVAL = 30


async def find_device():

    print("[BLE] Procurando ESP32...", flush=True)

    devices = await BleakScanner.discover(
        timeout=10,
        return_adv=True
    )

    for address, (device, adv) in devices.items():

        name = adv.local_name or device.name or ""

        services = [
            s.lower()
            for s in (adv.service_uuids or [])
        ]

        if (
            address.upper() == DEVICE_ADDRESS
            or name == DEVICE_NAME
            or SERVICE_UUID.lower() in services
        ):
            print(f"[BLE] Encontrado: {address}", flush=True)
            return device

    return None


async def http_worker(queue):

    while True:

        payload = await queue.get()

        try:

            response = await asyncio.to_thread(
                requests.post,
                SERVER_URL,
                json=payload,
                timeout=5
            )

            print(
                f"[HTTP] Status: {response.status_code}",
                flush=True
            )

        except Exception as error:

            print(
                f"[HTTP] Erro: {type(error).__name__}: {error}",
                flush=True
            )

        finally:
            queue.task_done()


async def connect_and_consume(queue):

    device = await find_device()

    if device is None:
        raise RuntimeError("ESP32 não encontrado")

    disconnected = asyncio.Event()

    def on_disconnect(client):
        print("[BLE] Conexão perdida!", flush=True)
        disconnected.set()

    def handle_notification(sender, data):

        try:

            payload = json.loads(data.decode("utf-8"))

            print(f"[BLE] Payload: {payload}", flush=True)

            queue.put_nowait(payload)

        except asyncio.QueueFull:
            print("[BLE] Fila cheia! Dados descartados.", flush=True)

        except Exception as error:
            print(f"[BLE] Erro: {error}", flush=True)

    print("[BLE] Conectando...", flush=True)

    async with BleakClient(
        device,
        timeout=20,
        disconnected_callback=on_disconnect
    ) as client:

        print(
            f"[BLE] Conectado: {client.is_connected}",
            flush=True
        )

        await client.start_notify(
            CHARACTERISTIC_UUID,
            handle_notification
        )

        print("[BLE] Notificações habilitadas.", flush=True)

        while client.is_connected and not disconnected.is_set():
            await asyncio.sleep(1)

    print("[BLE] Conexão encerrada.", flush=True)


async def main():

    queue = asyncio.Queue(maxsize=100)

    worker = asyncio.create_task(http_worker(queue))

    retry_interval = RETRY_INTERVAL

    try:

        while True:

            try:

                await connect_and_consume(queue)

                print("[BLE] Cliente desconectado.", flush=True)

            except Exception as error:

                print(
                    f"[BLE] Erro: {type(error).__name__}: {error}",
                    flush=True
                )

            print(
                f"[BLE] Nova tentativa em {retry_interval}s...",
                flush=True
            )

            await asyncio.sleep(retry_interval)

            retry_interval = min(
                retry_interval * 2,
                MAX_RETRY_INTERVAL
            )

    except asyncio.CancelledError:
        raise

    finally:
        worker.cancel()
        await asyncio.gather(worker, return_exceptions=True)


if __name__ == "__main__":

    try:
        asyncio.run(main())

    except KeyboardInterrupt:
        print("\n[APP] Encerrado pelo usuário.")
