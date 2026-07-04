import pandas as pd
import matplotlib.pyplot as plt
import matplotlib.dates as mdates

# Baca CSV - skip baris yang corrupt
df = pd.read_csv('data_pm25.csv', on_bad_lines='skip')
df['timestamp'] = pd.to_datetime(df['timestamp'], errors='coerce')
df = df.dropna(subset=['timestamp'])
df = df.sort_values('timestamp')

print(f"Total data terbaca: {len(df)} baris")

fig, (ax1, ax2, ax3) = plt.subplots(3, 1, figsize=(12, 8), sharex=True)
fig.suptitle('Data Sensor PM2.5 - AirQualityControl', fontsize=14, fontweight='bold')

# Grafik PM1.0
ax1.plot(df['timestamp'], df['pm1_0'], color='tomato', linewidth=1)
ax1.set_ylabel('PM1.0 (µg/m³)', fontsize=10)
ax1.set_title('PM1.0', fontsize=10)
ax1.grid(True, alpha=0.3)
ax1.fill_between(df['timestamp'], df['pm1_0'], alpha=0.1, color='tomato')

# Grafik PM2.5
ax2.plot(df['timestamp'], df['pm2_5'], color='steelblue', linewidth=1)
ax2.set_ylabel('PM2.5 (µg/m³)', fontsize=10)
ax2.set_title('PM2.5', fontsize=10)
ax2.grid(True, alpha=0.3)
ax2.fill_between(df['timestamp'], df['pm2_5'], alpha=0.1, color='steelblue')

# Grafik PM10
ax3.plot(df['timestamp'], df['pm10'], color='mediumseagreen', linewidth=1)
ax3.set_ylabel('PM10 (µg/m³)', fontsize=10)
ax3.set_title('PM10', fontsize=10)
ax3.grid(True, alpha=0.3)
ax3.fill_between(df['timestamp'], df['pm10'], alpha=0.1, color='mediumseagreen')

ax3.xaxis.set_major_formatter(mdates.DateFormatter('%H:%M:%S'))
plt.xticks(rotation=45)

plt.tight_layout()
plt.savefig('grafik_pm25.png', dpi=150, bbox_inches='tight')
plt.show()

print("Grafik disimpan sebagai grafik_pm25.png")