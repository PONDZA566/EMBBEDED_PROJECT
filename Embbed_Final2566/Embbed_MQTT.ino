/**
 * Run Edge Impulse FOMO model.
 * It works on both PSRAM and non-PSRAM boards.
 * 
 * The difference from the PSRAM version
 * is that this sketch only runs on 96x96 frames,
 * while PSRAM version runs on higher resolutions too.
 * 
 * The PSRAM version can be found in my
 * "ESP32S3 Camera Mastery" course
 * at https://dub.sh/ufsDj93
 *
 * BE SURE TO SET "TOOLS > CORE DEBUG LEVEL = INFO"
 * to turn on debug messages
 */
#include <Wire.h> 




// Exmaple of using the MQTT library for ESP32
// Library by Joël Gähwiler
// https://github.com/256dpi/arduino-mqtt
// Modified by Arnan Sipitakiats
#include <LCD_I2C.h>
#include <ESP32Servo.h>
#include <Adafruit_PCF8574.h>
#define PCF8574_Address 0x20
Adafruit_PCF8574 pcf;
Servo myservo; 
Servo myservo2;
int buttonState = 0;
int i = 0;
int x=6;
int y=6;
int a;
int b;
bool c;
bool d;
bool t = true;
LCD_I2C lcd(0x27, 16, 2); // Default address of most PCF8574 modules, change according
#include <WiFi.h>
#include <MQTT.h>

const char ssid[] = "@JumboPlusIoT";
const char pass[] = "coolpassword";

const char mqtt_broker[] = "test.mosquitto.org";
const char mqtt_topic[] = "group24/command";
const char mqtt_client_id[] = "arduino_group_24";  // must change this string to a unique value
int MQTT_PORT = 1883;

int counter = 0;

WiFiClient net;
MQTTClient client;

unsigned long lastMillis = 0;

void connect() {
  Serial.print("checking wifi...");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.println("wifi");
    delay(1000);
  }

  Serial.print("\nconnecting...");
  while (!client.connect(mqtt_client_id)) {
    Serial.print(".");
    delay(1000);
  }

  Serial.println("\nconnected!");

  client.subscribe(mqtt_topic);
  // client.unsubscribe("/hello");
}

void messageReceived(String &topic, String &payload) {
  Serial.println("incoming: " + topic + " - " + payload);
    lcd.clear();
    if (payload == "Right"){
        x+=1;
        if(x>12) x=12;
        c= false;
        d= false;
      }
      else if (payload == "Left"){
        x-=1;
        if(x<0) x=0;
        c=true;
        d=false;
      }
      else if (payload == "Up"){
        y+=1;
        if(y>12) y=12;
        c= false;
        d= true;
      }
      else if (payload == "Down"){
        y-=1;
        if(y<0) y=0;
        c = true;
        d= true;
      }
      myservo.write(x*15);
      myservo2.write(y*15);
      Serial.println(x);
      Serial.println(y);
  }
void setup() {
    
    
    Serial.begin(115200);
    if (!pcf.begin(PCF8574_Address, &Wire)) {
    Serial.println("Couldn't find PCF8574");
    }
    pcf.pinMode(0, INPUT);
    pcf.pinMode(1, INPUT);
    
    lcd.begin();
    myservo.attach(18); 
    myservo2.attach(19);
    WiFi.begin(ssid, pass);

    // Note: Local domain names (e.g. "Computer.local" on OSX) are not supported
    // by Arduino. You need to set the IP address directly.
    client.begin(mqtt_broker, MQTT_PORT, net);
    client.onMessage(messageReceived);
    lcd.backlight();
    connect();
}


void loop() {

  if(pcf.digitalRead(0) == HIGH) t = true;
  if(pcf.digitalRead(1) == HIGH) t = false;
    // capture picture
  if( t == true){
    //tracking();
    lcd.clear();
    Serial.println("tracking");
    lcd.print("Tracking");
    delay(100);
  }
  else if( t == false){
    lcd.clear();
    lcd.print("MQTT");
    Serial.println("MQTT");
    lcd.setCursor(0,1);
    if( c==false and d==false) lcd.print("Right");
    else if(c==true and d==false) lcd.print("Left");
    else if(c==false and d==true) lcd.print("Up");
    else if(c==true and d==true) lcd.print("Down");
    client.loop();
    delay(10);  // <- fixes some issues with WiFi stability

    if (!client.connected()) {
      connect();
    }
  }
}
