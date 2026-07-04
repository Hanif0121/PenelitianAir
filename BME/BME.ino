#include "DFRobot_BME280.h"
#include <SPI.h>

typedef DFRobot_BME280_SPI BME;

#define PIN_CS 5
BME bme(&SPI, PIN_CS);

void setup() {
  Serial.begin(115200);
  SPI.begin(18, 19, 23, 5);

  while (bme.begin() != BME::eStatusOK) {
    Serial.println("BME280 gagal");
    delay(2000);
  }

  Serial.println("BME280 siap");
}

void loop() {
  float temp = bme.getTemperature();
  float pressure = bme.getPressure() / 100.0;
  float hum = bme.getHumidity();

  // format CSV
  Serial.print(temp);
  Serial.print(",");
  Serial.print(pressure);
  Serial.print(",");
  Serial.println(hum);

  delay(2000);
}