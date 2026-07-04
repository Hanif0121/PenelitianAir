import pandas as pd
import matplotlib.pyplot as plt
import matplotlib.dates as mdates

# Baca CSV - skip baris yang corrupt
df = pd.read_csv('data_sensor.csv', on_bad_lines='skip')
df['timestamp'] = pd.to_datetime(df['timestamp'], errors='coerce')
df = df.dropna(subset=['timestamp'])
df = df.sort_values('timestamp')

print(f"Total data terbaca: {len(df)} baris")

fig, (ax1, ax2, ax3) = plt.subplots(3, 1, figsize=(12, 8), sharex=True)
fig.suptitle('Data Sensor BME280 - AirQualityControl', fontsize=14, fontweight='bold')

ax1.plot(df['timestamp'], df['temperature'], color='tomato', linewidth=1)
ax1.set_ylabel('Suhu (°C)', fontsize=10)
ax1.set_title('Temperature', fontsize=10)
ax1.grid(True, alpha=0.3)
ax1.fill_between(df['timestamp'], df['temperature'], alpha=0.1, color='tomato')

ax2.plot(df['timestamp'], df['pressure'], color='steelblue', linewidth=1)
ax2.set_ylabel('Tekanan (hPa)', fontsize=10)
ax2.set_title('Pressure', fontsize=10)
ax2.grid(True, alpha=0.3)
ax2.fill_between(df['timestamp'], df['pressure'], alpha=0.1, color='steelblue')

ax3.plot(df['timestamp'], df['humidity'], color='mediumseagreen', linewidth=1)
ax3.set_ylabel('Kelembaban (%)', fontsize=10)
ax3.set_title('Humidity', fontsize=10)
ax3.grid(True, alpha=0.3)
ax3.fill_between(df['timestamp'], df['humidity'], alpha=0.1, color='mediumseagreen')

ax3.xaxis.set_major_formatter(mdates.DateFormatter('%H:%M:%S'))
plt.xticks(rotation=45)

plt.tight_layout()
plt.savefig('grafik_sensor.png', dpi=150, bbox_inches='tight')
plt.show()

print("Grafik disimpan sebagai grafik_sensor.png")