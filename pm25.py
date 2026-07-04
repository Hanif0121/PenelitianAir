import serial
import csv
import os
from datetime import datetime

ser = serial.Serial('COM4', 115200, timeout=1)

CSV_FILE = 'data_pm25.csv'

if not os.path.exists(CSV_FILE):
    with open(CSV_FILE, 'w', newline='') as f:
        writer = csv.writer(f)
        writer.writerow(['timestamp', 'pm1_0', 'pm2_5', 'pm10', 'status'])

print("Menunggu sensor siap...")

while True:
    try:
        line = ser.readline().decode('utf-8', errors='ignore').strip()

        if line.startswith('WARMUP:'):
            sisa = line.replace('WARMUP:', '')
            print(f"Warm-up... {sisa} detik lagi")
            continue

        if line == 'READY':
            print("Sensor siap! Mulai membaca data...")
            print(f"Data disimpan di: {os.path.abspath(CSV_FILE)}")
            continue

        if line.startswith('DATA:'):
            data = line.replace('DATA:', '')
            parts = data.split(',', 3)  # split max 3x supaya status tidak terpotong
            if len(parts) != 4:
                continue

            pm1    = float(parts[0])
            pm25   = float(parts[1])
            pm10   = float(parts[2])
            status = parts[3]
            timestamp = datetime.now().strftime('%Y-%m-%d %H:%M:%S')

            print(f"[{timestamp}] PM1.0: {pm1} | PM2.5: {pm25} | PM10: {pm10} | {status}")

            with open(CSV_FILE, 'a', newline='') as f:
                writer = csv.writer(f)
                writer.writerow([timestamp, pm1, pm25, pm10, status])

    except KeyboardInterrupt:
        print("\nDihentikan. File CSV tersimpan di:")
        print(os.path.abspath(CSV_FILE))
        ser.close()
        break
    except Exception as e:
        continue