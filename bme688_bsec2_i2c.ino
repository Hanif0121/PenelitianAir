/**
 * bme688_bsec2_i2c.ino  (v2)
 *
 * ESP32 + BME688 via I2C, library BSEC2.
 *
 * Perubahan dari versi sebelumnya:
 *  1. State BSEC (hasil kalibrasi baseline) disimpan ke flash (NVS/Preferences)
 *     secara berkala, dan di-load lagi saat boot. Jadi kalau board restart
 *     di tengah masa burn-in / eksperimen, kalibrasi TIDAK hilang.
 *  2. Error saat runtime (loop) tidak lagi menghentikan program permanen --
 *     cukup dicatat ke Serial, program tetap lanjut. Ini penting untuk
 *     logging 24 jam tanpa pengawasan. Error saat setup() (wiring/init awal)
 *     tetap dihentikan karena itu memang harus diperbaiki manual dulu.
 *  3. Subscription list dibersihkan -- hanya output yang benar-benar dipakai
 *     (raw temp/humidity/pressure/gas + IAQ untuk accuracy indicator).
 *     STATIC_IAQ, CO2_EQUIVALENT, BREATH_VOC_EQUIVALENT dihapus karena tidak
 *     dipakai dan fokus penelitian ini ke GasResistance mentah, bukan IAQ
 *     bawaan Bosch yang di-tuning untuk VOC umum (bukan SO2 spesifik).
 *
 * Wiring (I2C):
 *   BME688 VCC -> ESP32 3.3V   (JANGAN 5V)
 *   BME688 GND -> ESP32 GND
 *   BME688 SDA -> ESP32 GPIO 21
 *   BME688 SCL -> ESP32 GPIO 22
 *
 * Output Serial: CSV -> Timestamp,Temperature,Humidity,Pressure,GasResistance,IAQ,IAQAccuracy
 */

#include <bsec2.h>
#include <Wire.h>
#include <Preferences.h>

#define BME68X_ADDR       BME68X_I2C_ADDR_LOW   // ganti ke _HIGH kalau modulmu 0x77
#define SDA_PIN            21
#define SCL_PIN            22
#define SAMPLE_RATE        BSEC_SAMPLE_RATE_LP  // ~3 detik per sample

// Simpan state BSEC ke flash tiap 30 menit sekali (cukup sering untuk
// menyelamatkan kalibrasi kalau restart, tapi tidak terlalu sering
// menulis flash / bikin aus)
#define STATE_SAVE_PERIOD_MS   (30UL * 60UL * 1000UL)

Bsec2 envSensor;
Preferences preferences;

uint8_t bsecState[BSEC_MAX_STATE_BLOB_SIZE] = {0};
unsigned long lastStateSaveMillis = 0;

/* ---------- deklarasi fungsi ---------- */
void checkBsecStatus(Bsec2 bsec, bool haltOnError);
void newDataCallback(const bme68xData data, const bsecOutputs outputs, Bsec2 bsec);
void loadBsecState(void);
void saveBsecStateIfDue(void);

void setup(void)
{
    Serial.begin(115200);
    while (!Serial) delay(10);

    Wire.begin(SDA_PIN, SCL_PIN);

    /* Load kalibrasi lama (kalau ada) SEBELUM begin(), supaya BSEC langsung
       pakai baseline yang sudah pernah didapat, bukan mulai dari nol lagi */
    if (!envSensor.begin(BME68X_ADDR, Wire))
    {
        checkBsecStatus(envSensor, true);   // error di setup -> wajib berhenti & diperbaiki manual
    }

    loadBsecState();

    /* Subscription yang benar-benar dipakai saja */
    bsecSensor sensorList[] = {
        BSEC_OUTPUT_RAW_TEMPERATURE,
        BSEC_OUTPUT_RAW_PRESSURE,
        BSEC_OUTPUT_RAW_HUMIDITY,
        BSEC_OUTPUT_RAW_GAS,
        BSEC_OUTPUT_IAQ
    };

    if (!envSensor.updateSubscription(sensorList, ARRAY_LEN(sensorList), SAMPLE_RATE))
    {
        checkBsecStatus(envSensor, true);
    }

    envSensor.attachCallback(newDataCallback);

    Serial.println("BSEC library version " +
        String(envSensor.version.major) + "." +
        String(envSensor.version.minor) + "." +
        String(envSensor.version.major_bugfix) + "." +
        String(envSensor.version.minor_bugfix));

    Serial.println("Timestamp,Temperature,Humidity,Pressure,GasResistance,IAQ,IAQAccuracy");

    lastStateSaveMillis = millis();
}

void loop(void)
{
    if (!envSensor.run())
    {
        checkBsecStatus(envSensor, false);  // error/warning saat runtime -> log saja, jangan halt
    }

    saveBsecStateIfDue();
}

void newDataCallback(const bme68xData data, const bsecOutputs outputs, Bsec2 bsec)
{
    if (!outputs.nOutputs) return;

    float temperature = 0, humidity = 0, pressure = 0, gasResistance = 0, iaq = 0;
    uint8_t iaqAccuracy = 0;

    for (uint8_t i = 0; i < outputs.nOutputs; i++)
    {
        const bsecData output = outputs.output[i];
        switch (output.sensor_id)
        {
            case BSEC_OUTPUT_RAW_TEMPERATURE:
                temperature = output.signal;
                break;
            case BSEC_OUTPUT_RAW_HUMIDITY:
                humidity = output.signal;
                break;
            case BSEC_OUTPUT_RAW_PRESSURE:
                pressure = output.signal;
                break;
            case BSEC_OUTPUT_RAW_GAS:
                gasResistance = output.signal;
                break;
            case BSEC_OUTPUT_IAQ:
                iaq = output.signal;
                iaqAccuracy = output.accuracy;
                break;
            default:
                break;
        }
    }

    Serial.print(millis());
    Serial.print(",");
    Serial.print(temperature);
    Serial.print(",");
    Serial.print(humidity);
    Serial.print(",");
    Serial.print(pressure);
    Serial.print(",");
    Serial.print(gasResistance);
    Serial.print(",");
    Serial.print(iaq);
    Serial.print(",");
    Serial.println(iaqAccuracy);
}

/* Load state BSEC yang tersimpan dari sesi sebelumnya (kalau ada) */
void loadBsecState(void)
{
    preferences.begin("bsec", false);
    if (preferences.isKey("state"))
    {
        size_t len = preferences.getBytes("state", bsecState, BSEC_MAX_STATE_BLOB_SIZE);
        if (len == BSEC_MAX_STATE_BLOB_SIZE)
        {
            if (envSensor.setState(bsecState))
            {
                Serial.println("BSEC state berhasil di-load dari flash.");
            }
            else
            {
                Serial.println("Gagal apply BSEC state yang di-load (mungkin corrupt/versi beda).");
            }
        }
    }
    else
    {
        Serial.println("Belum ada BSEC state tersimpan, mulai kalibrasi dari nol.");
    }
    preferences.end();
}

/* Simpan state BSEC ke flash setiap STATE_SAVE_PERIOD_MS */
void saveBsecStateIfDue(void)
{
    if (millis() - lastStateSaveMillis < STATE_SAVE_PERIOD_MS) return;

    if (envSensor.getState(bsecState))
    {
        preferences.begin("bsec", false);
        preferences.putBytes("state", bsecState, BSEC_MAX_STATE_BLOB_SIZE);
        preferences.end();
        Serial.println("BSEC state disimpan ke flash.");
    }

    lastStateSaveMillis = millis();
}

/**
 * haltOnError = true  -> dipakai saat setup(): kalau gagal, program berhenti
 *                        total karena butuh perbaikan manual (wiring/config).
 * haltOnError = false -> dipakai saat loop(): error/warning dicatat ke Serial
 *                        saja, program tetap lanjut jalan (untuk logging
 *                        24 jam tanpa pengawasan).
 */
void checkBsecStatus(Bsec2 bsec, bool haltOnError)
{
    if (bsec.status < BSEC_OK)
    {
        Serial.println("BSEC error code : " + String(bsec.status));
        if (haltOnError) { while (1) delay(10); }
    }
    else if (bsec.status > BSEC_OK)
    {
        Serial.println("BSEC warning code : " + String(bsec.status));
    }

    if (bsec.sensor.status < BME68X_OK)
    {
        Serial.println("BME68X error code : " + String(bsec.sensor.status));
        if (haltOnError) { while (1) delay(10); }
    }
    else if (bsec.sensor.status > BME68X_OK)
    {
        Serial.println("BME68X warning code : " + String(bsec.sensor.status));
    }
}
