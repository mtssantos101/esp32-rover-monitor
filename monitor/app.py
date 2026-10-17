from flask import Flask, jsonify, render_template, request
from flask_socketio import SocketIO
import csv
import os
from collections import deque
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
    try:
        limit = int(request.args.get("limit", 100))
        offset = int(request.args.get("offset", 0))
    except ValueError:
        return jsonify({"error": "limit e offset devem ser números inteiros"}), 400

    if not 1 <= limit <= 1000:
        return jsonify({"error": "limit deve estar entre 1 e 1000"}), 400
    if not 0 <= offset <= 10000:
        return jsonify({"error": "offset deve estar entre 0 e 10000"}), 400

    if not os.path.exists(CSV_FILE):
        return jsonify([])

    with open(CSV_FILE, newline="") as file:
        rows = deque(csv.DictReader(file), maxlen=limit + offset)

    rows = list(rows)
    selected_rows = rows[-limit:] if offset == 0 else rows[-(limit + offset):-offset]
    data = [{
        "timestamp": row["timestamp"],
        "temperature": float(row["temperature"]) if row["temperature"] else None,
        "humidity": float(row["humidity"]) if row["humidity"] else None,
        "distance1": float(row["distance1"]) if row["distance1"] else None,
        "distance2": float(row["distance2"]) if row["distance2"] else None,
        "obstacle": row["obstacle"].lower() == "true"
    } for row in selected_rows]

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
        allow_unsafe_werkzeug=True
    )
