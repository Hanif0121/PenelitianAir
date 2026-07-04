#include <Wire.h>
#include <DFRobot_AirQualitySensor.h>

#define I2C_ADDRESS 0x19
#define SDA_PIN 21
#define SCL_PIN 22
#define WARMUP_TIME 15000  // 15 detik

DFRobot_AirQualitySensor particle(&Wire, I2C_ADDRESS);

void scanI2C() {
  Serial.println("\n=== Scanning I2C Bus ===");
  int found = 0;
  for (byte addr = 1; addr < 127; addr++) {
    Wire.beginTransmission(addr);
    byte error = Wire.endTransmission();
    if (error == 0) {
      Serial.print("Perangkat ditemukan di: 0x");
      if (addr < 16) Serial.print("0");
      Serial.println(addr, HEX);
      found++;
    }
  }
  if (found == 0) {
    Serial.println("Tidak ada perangkat I2C ditemukan!");
    Serial.println("Cek kabel SDA/SCL dan power sensor.");
  }
  Serial.println("=== Scan Selesai ===\n");
}

String getStatus(float pm25) {
  if (pm25 <= 15.5) {
    return "BAIK";
  } else if (pm25 <= 55.4) {
    return "SEDANG";
  } else if (pm25 <= 150.4) {
    return "TIDAK SEHAT";
  } else if (pm25 <= 250.4) {
    return "SANGAT TIDAK SEHAT";
  } else {
    return "BERBAHAYA";
  }
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("\n=============================");
  Serial.println("  DFRobot Air Quality Sensor ");
  Serial.println("  Standar ISPU Indonesia     ");
  Serial.println("=============================");

  Wire.begin(SDA_PIN, SCL_PIN);
  Serial.println("I2C diinisialisasi (SDA=21, SCL=22)");

  scanI2C();

  Serial.println("Menginisialisasi sensor...");
  int retry = 0;
  while (!particle.begin()) {
    retry++;
    Serial.print("Sensor tidak terdeteksi! Percobaan ke-");
    Serial.println(retry);
    if (retry >= 5) {
      Serial.println("GAGAL! Cek koneksi hardware dan restart.");
      while (true) delay(1000);
    }
    delay(2000);
  }
  Serial.println("Sensor berhasil terdeteksi!");

  // Warm-up countdown
  Serial.print("Warm-up sensor selama ");
  Serial.print(WARMUP_TIME / 1000);
  Serial.println(" detik...");
  for (int i = WARMUP_TIME / 1000; i > 0; i--) {
    Serial.print("WARMUP:");
    Serial.println(i);
    delay(1000);
  }

  Serial.println("READY");
  Serial.println("timestamp,pm1_0,pm2_5,pm10,status");
}

void loop() {
  uint16_t pm1_raw  = particle.gainParticleConcentration_ugm3(PARTICLE_PM1_0_STANDARD);
  uint16_t pm25_raw = particle.gainParticleConcentration_ugm3(PARTICLE_PM2_5_STANDARD);
  uint16_t pm10_raw = particle.gainParticleConcentration_ugm3(PARTICLE_PM10_STANDARD);

  // Koreksi nilai raw (tanpa humidity)
  float pm1  = pm1_raw  * 0.52 + 5.71;
  float pm25 = pm25_raw * 0.52 + 5.71;
  float pm10 = pm10_raw * 0.52 + 5.71;

  String status = getStatus(pm25);

  Serial.print("DATA:");
  Serial.print(pm1,  1);
  Serial.print(",");
  Serial.print(pm25, 1);
  Serial.print(",");
  Serial.print(pm10, 1);
  Serial.print(",");
  Serial.println(status);

  delay(1000);
}