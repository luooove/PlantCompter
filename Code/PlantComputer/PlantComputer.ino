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

/*
   _______________threshold value_________________
*/
#define CO2_Threshold_LOW   1
#define CO2_Threshold_HIGH   1
#define Air_TEMP_Threshold_LOW    1
#define Air_TEMP_Threshold_HIGH   1
#define Light_Threshold_LOW    1
#define Light_Threshold_HIGH   1


/*
   ________________Conductivity_Init__________________
*/

const byte numReadings = 20;     //the number of sample times
byte ECsensorPin = A0;  //EC Meter analog output,pin on analog 1
unsigned int AnalogSampleInterval = 25, printInterval = 700, tempSampleInterval = 850; //analog sample interval;serial print interval;temperature sample interval
unsigned int readings[numReadings];      // the readings from the analog input
byte index = 0;                  // the index of the current reading
unsigned long AnalogValueTotal = 0;                  // the running total
unsigned int AnalogAverage = 0, averageVoltage = 0;             // the average
unsigned long AnalogSampleTime, printTime, tempSampleTime;
float temperature, ECcurrent;
/*
   ________________PH_Init__________________
*/
#define SensorPin A7            //pH meter Analog output to Arduino Analog Input 0
#define Offset 0.00            //deviation compensate
unsigned long int avgValue;     //Store the average value of the sensor feedback

/*
   _______________Setup__________________
*/
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  sensors.begin();
  lightMeter.begin();
  Serial.println(F("BH1750 Test"));
  for (byte thisReading = 0; thisReading < numReadings; thisReading++)
    readings[thisReading] = 0;
  AnalogSampleTime = millis();
  printTime = millis();
  tempSampleTime = millis();
  lcd.init();                      // initialize the lcd
  lcd.init();
  // Print a message to the LCD.
  lcd.backlight();
  lcd.setCursor(3, 0);
  lcd.print("PlantComputer");
  pinMode(46, OUTPUT);//CO2
  pinMode(48, OUTPUT);//生长灯
  pinMode(50, OUTPUT);//冷却风扇
  pinMode(52, OUTPUT);//加热
  digitalWrite(46, HIGH);
  digitalWrite(48, HIGH);
  digitalWrite(50, HIGH);
  digitalWrite(52, HIGH);
  delay(2000);
  digitalWrite(48, LOW);
  delay(2000);
  digitalWrite(48, HIGH);
  delay(2000);
  digitalWrite(48, LOW);
}



/*
   ________________PH__________________
*/
void PH_detect()
{
  int buf[10];                //buffer for read analog
  for (int i = 0; i < 10; i++) //Get 10 sample value from the sensor for smooth the value
  {
    buf[i] = analogRead(SensorPin);
    delay(10);
  }
  for (int i = 0; i < 9; i++) //sort the analog from small to large
  {
    for (int j = i + 1; j < 10; j++)
    {
      if (buf[i] > buf[j])
      {
        int temp = buf[i];
        buf[i] = buf[j];
        buf[j] = temp;
      }
    }
  }
  avgValue = 0;
  for (int i = 2; i < 8; i++)               //take the average value of 6 center sample
    avgValue += buf[i];
  float phValue = (float)avgValue * 5.0 / 1024 / 6; //convert the analog into millivolt
  phValue = 3.5 * phValue + Offset;                //convert the millivolt into pH value
  Serial.print("    pH:");
  Serial.print(phValue, 2);
  Serial.println(" ");

  lcd.setCursor(0, 2);
  lcd.print("PH Value :");
  lcd.print(phValue);//水温
  //  digitalWrite(13, HIGH);
  //  delay(800);
  //  digitalWrite(13, LOW);
}
/*
   _______________Conductivity___________________
*/
void Conductivity()
{
  Serial.println("______________");
  Serial.println(AnalogSampleTime);
  Serial.println(printTime);
  Serial.println(tempSampleTime);
  Serial.println(millis());

  if (millis() - AnalogSampleTime >= AnalogSampleInterval)
  {
    AnalogSampleTime = millis();
    // subtract the last reading:
    AnalogValueTotal = AnalogValueTotal - readings[index];
    // read from the sensor:
    readings[index] = analogRead(ECsensorPin);
    // add the reading to the total:
    AnalogValueTotal = AnalogValueTotal + readings[index];
    // advance to the next position in the array:
    index = index + 1;
    // if we're at the end of the array...
    if (index >= numReadings)
      // ...wrap around to the beginning:
      index = 0;
    // calculate the average:
    AnalogAverage = AnalogValueTotal / numReadings;
  }
  /*
    Every once in a while,MCU read the temperature from the DS18B20 and then let the DS18B20 start the convert.
    Attention:The interval between start the convert and read the temperature should be greater than 750 millisecond,or the temperature is not accurate!
  */
  if (millis() - tempSampleTime >= tempSampleInterval)
  {
    tempSampleTime = millis();
    get_water_Temp();

    // temperature = TempProcess(ReadTemperature);  // read the current temperature from the  DS18B20
    // TempProcess(StartConvert);                   //after the reading,start the convert for next reading


  }
  /*
    Every once in a while,print the information on the serial monitor.
  */
  if (millis() - printTime >= printInterval)
  {
    printTime = millis();
    averageVoltage = AnalogAverage * (float)5000 / 1024;
    Serial.print("Analog value:");
    Serial.print(AnalogAverage);   //analog average,from 0 to 1023
    Serial.print("    Voltage:");
    Serial.print(averageVoltage);  //millivolt average,from 0mv to 4995mV
    Serial.print("mV    ");
    Serial.print("temp:");
    Serial.print(temperature);    //current temperature
    Serial.print("^C     EC:");

    float TempCoefficient = 1.0 + 0.0185 * (temperature - 25.0); //temperature compensation formula: fFinalResult(25^C) = fFinalResult(current)/(1.0+0.0185*(fTP-25.0));
    float CoefficientVolatge = (float)averageVoltage / TempCoefficient;
    if (CoefficientVolatge < 150)
      Serial.println("No solution!"); //25^C 1413us/cm<-->about 216mv  if the voltage(compensate)<150,that is <1ms/cm,out of the range
    else if (CoefficientVolatge > 3300)
      Serial.println("Out of the range!"); //>20ms/cm,out of the range
    else
    {
      if (CoefficientVolatge <= 448)
        ECcurrent = 6.84 * CoefficientVolatge - 64.32; //1ms/cm<EC<=3ms/cm
      else if (CoefficientVolatge <= 1457)
        ECcurrent = 6.98 * CoefficientVolatge - 127; //3ms/cm<EC<=10ms/cm
      else ECcurrent = 5.3 * CoefficientVolatge + 2278;                     //10ms/cm<EC<20ms/cm
      ECcurrent /= 1000;  //convert us/cm to ms/cm
      Serial.print(ECcurrent, 2); //two decimal
      Serial.println("ms/cm");
    }
  }
  lcd.clear();//清屏
  lcd.setCursor(3, 0);
  lcd.print("PlantComputer");
  lcd.setCursor(0, 1);
  lcd.print("EC Value:");
  lcd.print(ECcurrent);//水温
  lcd.print("  ms/cm");

}




/*
   ________________waterTemperature__________________
*/
void water_Temp()
{
  sensors.requestTemperatures();
  temperature = sensors.getTempCByIndex(0);
  Serial.println(temperature);
  lcd.setCursor(0, 2);
  lcd.print("Water TEMP:");
  lcd.print(sensors.getTempCByIndex(0));//水温
}

void get_water_Temp()
{
  sensors.requestTemperatures();
  temperature = sensors.getTempCByIndex(0);
  Serial.println(temperature);
}

/*
   ________________AirTemperatureHumidity__________________
*/

float temperature_air = 0;
float humidity_air = 0;
void Temp_Hum()//温湿度
{
  int chk = DHT11.read(DHT11PIN);

  // lcd.print(" ·C");
  lcd.setCursor(0, 3);
  lcd.print("H:");
  humidity_air = DHT11.humidity;
  lcd.print((float)DHT11.humidity, 2);//打印光照值
  lcd.setCursor(10, 3);
  lcd.print("T :");
  temperature_air = DHT11.temperature;
  lcd.print((float)DHT11.temperature, 2);//打印光照值
}
/*
   ________________light__________________
*/
void light()
{
  uint16_t lux = lightMeter.readLightLevel();
  Serial.print("Light: ");
  Serial.print(lux);
  Serial.println(" lx");
  lcd.setCursor(0, 1);
  lcd.print("Light Value:");
  lcd.print(lux);//打印光照值
  lcd.print(" lx");
}
#define         DC_GAIN                      (3)   //define the DC gain of amplifier
#define         MG_PIN                       (A1)
/*
   ________________CO2__________________
*/

void CO2()
{
  float volts;
  volts = (float)analogRead(MG_PIN) * 5 / 1024;

  if (volts > 0.9)
  {
    Serial.print( "<400" );
    lcd.setCursor(0, 3);
    lcd.print("CO2:");
    lcd.print("<400");
    lcd.print(" PPM");
  }
  else
  {
    Serial.print(320 / volts);
    lcd.setCursor(0, 3);
    lcd.print("CO2:");
    lcd.print(320 / volts);
    lcd.print(" PPM");
  }

}
/*
   ________________LCD4002__________________
*/
void LCD4002_Show()
{

}

/*
   ________________Adjustment通过打开各种执行器来调整参数__________________
*/
void Adjust()
{
  if (temperature_air < 20)
  { //打开加热器
    digitalWrite(52, LOW); //加热
  }
  if (temperature_air > 25)
  { //打开加热器
    digitalWrite(52, HIGH); //不加热
  }
  if (temperature_air > 30)//开风扇
  { //打开风扇
    digitalWrite(50, LOW);
  }
  if (temperature_air < 25)
  { //打开加热器
    digitalWrite(50, HIGH); //关风扇
  }


}
//  if ()
//  { // 如果光照小于光照阈值就打开生长灯
//
//  }
//  else
//  {


//  if ()
//  { // 如果温度低于   就打开加热器
//
//  }
//  if ()
//  { // 如果温度高于    就打开风扇散热
//
//  }
//
//  if ()
//  { // 如果二氧化碳浓度高于    就打开风扇散热
//
//  }



/*
   ________________main__________________
*/
void loop() {
  // put your main code here, to run repeatedly:
  //_____page1_____
  lcd.clear();//清屏
  lcd.setCursor(3, 0);
  lcd.print("PlantComputer");
  water_Temp();
  Temp_Hum();
  light();
  delay(3000);
  //_____page2_____
  Conductivity();
  PH_detect();

  CO2();
  // Adjust();
  delay(3000);
}



