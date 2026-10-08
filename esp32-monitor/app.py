from flask import Flask, jsonify, render_template, request
from flask_socketio import SocketIO
import csv
import os
from datetime import datetime

app = Flask(__name__)
socketio = SocketIO(app, cors_allowed_origins="*")

CSV_FILE = "sensors.csv"

def save_to_csv(data):
    file_exists = os.path.exists(CSV_FILE)

    with open(CSV_FILE, "a", newline="") as file:
        fieldnames = [
            "timestamp",
            "temperature",
            "humidity",
            "distance1",
            "distance2",
            "obstacle"
        ]

        writer = csv.DictWriter(file, fieldnames=fieldnames)

        if not file_exists or os.path.getsize(CSV_FILE) == 0:
            writer.writeheader()

        writer.writerow(data)

@app.route("/")
def index():
    return render_template("index.html")

@app.route("/api/sensors", methods=["GET"])
def sensors():
    if not os.path.exists(CSV_FILE):
        return jsonify([])

    with open(CSV_FILE, newline="") as file:
        reader = csv.DictReader(file)

        data = []

        for row in reader:
            data.append({
                "timestamp": row["timestamp"],
                "temperature": float(row["temperature"]) if row["temperature"] else None,
                "humidity": float(row["humidity"]) if row["humidity"] else None,
                "distance1": float(row["distance1"]) if row["distance1"] else None,
                "distance2": float(row["distance2"]) if row["distance2"] else None,
                "obstacle": row["obstacle"].lower() == "true"
            })

    return jsonify(data)

@app.route("/api/sensors", methods=["POST"])
def receive_sensor_data():
    data = request.get_json()

    if not data:
        return jsonify({"error": "Dados invalidos"}), 400

    sensor_data = {
        "timestamp": datetime.now().isoformat(),
        "temperature": data.get("temperature"),
        "humidity": data.get("humidity"),
        "distance1": data.get("distance1"),
        "distance2": data.get("distance2"),
        "obstacle": data.get("obstacle", False)
    }

    save_to_csv(sensor_data)

    socketio.emit("sensor_data", sensor_data)

    print(sensor_data)

    return jsonify({"status": "ok"}), 201

if __name__ == "__main__":
    socketio.run(
        app,
        host="0.0.0.0",
        port=5000,
        debug=True,
        allow_unsafe_werkzeug=True
    )