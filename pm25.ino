#include <HardwareSerial.h>
#include <PMS.h>

// UART2 ESP32
HardwareSerial PMSerial(2);

// Objek PMS7003
PMS pms(PMSerial);
PMS::DATA data;

void setup() {
  Serial.begin(115200);

  // RX = GPIO16, TX = GPIO17
  PMSerial.begin(9600, SERIAL_8N1, 16, 17);

  Serial.println();
  Serial.println("=================================");
  Serial.println("   ESP32 + Plantower PMS7003");
  Serial.println("=================================");
}

void loop() {

  if (pms.read(data)) {

    Serial.println("--------------------------------");

    Serial.print("PM1.0  : ");
    Serial.print(data.PM_AE_UG_1_0);
    Serial.println(" ug/m3");

    Serial.print("PM2.5  : ");
    Serial.print(data.PM_AE_UG_2_5);
    Serial.println(" ug/m3");

    Serial.print("PM10   : ");
    Serial.print(data.PM_AE_UG_10_0);
    Serial.println(" ug/m3");

    Serial.println("--------------------------------");
    Serial.println();
  }
}