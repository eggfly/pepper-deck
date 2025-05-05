#include "Adafruit_MAX1704X.h"
#include "battery_gauge.h"

battery_gauge_data battery_gauge = {0.0, 0.0};

Adafruit_MAX17048 maxlipo;


void battery_setup()
{
    Serial.println(F("\nAdafruit MAX17048 simple demo"));
    while (!maxlipo.begin())
    {
        Serial.println(F("Couldnt find Adafruit MAX17048?\nMake sure a battery is plugged in!"));
        delay(2000);
    }
    Serial.print(F("Found MAX17048"));
    Serial.print(F(" with Chip ID: 0x"));
    Serial.println(maxlipo.getChipID(), HEX);
}

void battery_loop()
{
    float cellVoltage = maxlipo.cellVoltage();
    if (isnan(cellVoltage))
    {
        Serial.println("Failed to read cell voltage, check battery is connected!");
        delay(2000);
        return;
    }
    auto cellPercent = maxlipo.cellPercent();
    // Serial.print(F("Batt Voltage: "));
    // Serial.print(cellVoltage, 3);
    // Serial.println(" V");
    // Serial.print(F("Batt Percent: "));
    // Serial.print(cellPercent, 1);
    // Serial.println(" %");
    // Serial.println();
    battery_gauge.voltage = cellVoltage;
    battery_gauge.percent = cellPercent;
}
