// This example shows how to connect to Cayenne using an ESP8266 and send/receive sample data.
// Make sure you install the ESP8266 Board Package via the Arduino IDE Board Manager and select the correct ESP8266 board before compiling.

//#define CAYENNE_DEBUG
#define CAYENNE_PRINT Serial
#include <CayenneMQTTESP8266.h>

// WiFi network info.
char ssid[] = "maker";
char wifiPassword[] = "swjtumaker";

// Cayenne authentication info. This should be obtained from the Cayenne Dashboard.
char username[] = "4cf8f760-aa02-11e6-82ee-87310fcf8579";
char password[] = "ecfbcb926bd4580a0f734621363927b2b8fbc10e";
char clientID[] = "e2adea60-aa46-11e6-a85d-c165103f15c2";

unsigned long lastMillis = 0;
char str[4];

void setup() {
  Serial.begin(9600);
  pinMode(D1, OUTPUT);
  Cayenne.begin(username, password, clientID, ssid, wifiPassword);
}

void loop() {
  Cayenne.loop();
  Cayenne.virtualWrite(0, analogRead(A0));
  //delay(5);
  Serial.println(analogRead(A0));
  int i=0;
  if (Serial.available()) // check if the esp is sending a message
  {
    while (Serial.available() )
    {
      str[i++] = Serial.read();
    }
    str[i++]='\0';
 //   int a=(str[0]-49)*256+str[1]-49;
    Serial.print("str[0]=");
    Serial.println(str[0]);
    Serial.print("str[1]=");
    Serial.println(str[1]);
    
  }
  //Publish data every 10 seconds (10000 milliseconds). Change this value to publish at a different interval.
  //if (millis() - lastMillis > 10000) {
  //lastMillis = millis();
  //Write data to Cayenne here. This example just sends the current uptime in milliseconds.
  //Cayenne.virtualWrite(0, lastMillis);
  //Some examples of other functions you can use to send data.
  //Cayenne.celsiusWrite(1, 22.0);
  //Cayenne.luxWrite(2, 700);
  //Cayenne.virtualWrite(3, 50, TYPE_PROXIMITY, UNIT_CENTIMETER);
  //}
}

//Default function for processing actuator commands from the Cayenne Dashboard.
//You can also use functions for specific channels, e.g CAYENNE_IN(1) for channel 1 commands.
CAYENNE_IN(1)
{
  CAYENNE_LOG("CAYENNE_IN_DEFAULT(%u) - %s, %s", request.channel, getValue.getId(), getValue.asString());
  //Process message here. If there is an error set an error message using getValue.setError(), e.g getValue.setError("Error message");
  int currentValue = getValue.asInt(); // 0 to 1

  // assuming you wire your relay as normally open
  if (currentValue == 0) {
    digitalWrite(D1, HIGH);
  } else {
    digitalWrite(D1, LOW);
  }
}

CAYENNE_IN(2)
{

}
