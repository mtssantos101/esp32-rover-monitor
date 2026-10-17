import random
import time
import requests

SERVER_URL = "http://127.0.0.1:5000/api/sensors"

while True:
    distance1 = round(random.uniform(5, 100), 2)
    distance2 = round(random.uniform(5, 100), 2)

    obstacle = distance1 <= 20 or distance2 <= 20

    data = {
        "temperature": round(random.uniform(22, 32), 2),
        "humidity": round(random.uniform(45, 75), 2),
        "distance1": distance1,
        "distance2": distance2,
        "obstacle": obstacle
    }

    try:
        response = requests.post(
            SERVER_URL,
            json=data,
            timeout=5
        )

        print(data)

        if response.status_code != 201:
            print(f"Erro HTTP: {response.status_code}")

    except requests.RequestException as error:
        print(f"Erro de conexao: {error}")

    time.sleep(1)