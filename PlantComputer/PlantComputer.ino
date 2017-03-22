/*

     Connection:
  BH1750
    VCC -> 5V (3V3 on Arduino Due, Zero, MKR1000, etc)
    GND -> GND
    SCL -> SCL (A5 on Arduino Uno, Leonardo, etc or 21 on Mega and Due)
    SDA -> SDA (A4 on Arduino Uno, Leonardo, etc or 20 on Mega and Due)
    ADD -> (not connected) or GND
  4002a
    VCC -> 5V (3V3 on Arduino Due, Zero, MKR1000, etc)
    GND -> GND
    SCL -> SCL (A5 on Arduino Uno, Leonardo, etc or 21 on Mega and Due)
    SDA -> SDA (A4 on Arduino Uno, Leonardo, etc or 20 on Mega and Due)
  DS18B20
    VCC -> 5V (3V3 on Arduino Due, Zero, MKR1000, etc)
    GND -> GND
    ds -> 2
  DHT11
    VCC -> 5V (3V3 on Arduino Due, Zero, MKR1000, etc)
    GND -> GND
    s -> 3
*/
#include <Wire.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <BH1750.h>
#include <LiquidCrystal_I2C.h>
#include <dht11.h>
#define ONE_WIRE_BUS 2

dht11 DHT11;
#define DHT11PIN 3
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
BH1750 lightMeter;
LiquidCrystal_I2C lcd(0x3F, 20, 4); // set the LCD address to 0x27 for a 16 chars and 2 line display

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  sensors.begin();
  lightMeter.begin();
  Serial.println(F("BH1750 Test"));
  lcd.init();                      // initialize the lcd
  lcd.init();
  // Print a message to the LCD.
  lcd.backlight();
  lcd.setCursor(3, 0);
  lcd.print("PlantComputer");
}

void loop() {
  // put your main code here, to run repeatedly:
  int chk = DHT11.read(DHT11PIN);
  sensors.requestTemperatures();
  Serial.println(sensors.getTempCByIndex(0));
  uint16_t lux = lightMeter.readLightLevel();
  Serial.print("Light: ");
  Serial.print(lux);
  Serial.println(" lx");
  lcd.clear();//清屏
  lcd.setCursor(3, 0);
  lcd.print("PlantComputer");
  lcd.setCursor(0, 1);
  lcd.print("Light Value:");
  lcd.print(lux);//打印光照值
  lcd.print(" lx");
  lcd.setCursor(0, 2);
  lcd.print("Water TEMP:");
  lcd.print(sensors.getTempCByIndex(0));//打印光照值
  // lcd.print(" ·C");
  lcd.setCursor(0, 3);
  lcd.print("H:");
  lcd.print((float)DHT11.humidity, 2);//打印光照值
  lcd.setCursor(10, 3);
  lcd.print("T :");
  lcd.print((float)DHT11.temperature, 2);//打印光照值
  delay(1000);
}
