import serial
import csv
import os
from datetime import datetime

# Ganti COM3 sesuai Device Manager
ser = serial.Serial('COM4', 115200, timeout=1)

# File CSV disimpan di folder yang sama dengan bme.py
CSV_FILE = 'data_sensor.csv'

# Buat header kalau file belum ada
if not os.path.exists(CSV_FILE):
    with open(CSV_FILE, 'w', newline='') as f:
        writer = csv.writer(f)
        writer.writerow(['timestamp', 'temperature', 'pressure', 'humidity'])

print("Membaca data dari ESP32...")
print(f"Data disimpan di: {os.path.abspath(CSV_FILE)}")

while True:
    try:
        line = ser.readline().decode('utf-8').strip()
        if not line:
            continue

        parts = line.split(',')
        if len(parts) != 3:
            continue

        temp     = float(parts[0])
        pressure = float(parts[1])
        humidity = float(parts[2])
        timestamp = datetime.now().strftime('%Y-%m-%d %H:%M:%S')

        print(f"[{timestamp}] Temp: {temp}°C | Pressure: {pressure} hPa | Humidity: {humidity}%")

        # Simpan ke CSV
        with open(CSV_FILE, 'a', newline='') as f:
            writer = csv.writer(f)
            writer.writerow([timestamp, temp, pressure, humidity])

    except KeyboardInterrupt:
        print("\nDihentikan. File CSV tersimpan di:")
        print(os.path.abspath(CSV_FILE))
        ser.close()
        break
    except Exception as e:
        print(f"Error: {e}")