
import asyncio
from bleak import BleakScanner

SERVICE_UUID = "4fafc201-1fb5-459e-8fcc-c5c9c331914b"

async def main():
    print("Procurando dispositivos BLE por 20 segundos...")

    devices = await BleakScanner.discover(
        timeout=20,
        return_adv=True
    )

    found = False

    for address, (device, adv) in devices.items():
        name = adv.local_name or device.name or "Sem nome"
        services = adv.service_uuids or []

        if (
            name == "ESP32-SENSOR"
            or SERVICE_UUID.lower() in [s.lower() for s in services]
        ):
            found = True
            print("\n*** POSSIVEL ESP32 ENCONTRADO ***")
            print("Nome:", name)
            print("Endereco:", address)
            print("Servicos:", services)
            print("RSSI:", adv.rssi)

    if not found:
        print("ESP32-SENSOR e seu servico nao foram encontrados.")

asyncio.run(main())