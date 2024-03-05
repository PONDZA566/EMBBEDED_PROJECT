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
//#include <LiquidCrystal_I2C.h> 
#include <Wire.h>
#include <ESP32Servo.h> 
#include <LCD_I2C.h>
#include <Adafruit_PCF8574.h>
#define PCF8574_Address 0x20
#include <face_detect_reai_inferencing.h>
#include <eloquent_esp32cam.h>
#include <eloquent_esp32cam/edgeimpulse/fomo.h>
#include <WiFi.h>
#include <MQTT.h>

#define SDA 15
#define SCL 14

using eloq::camera;
using eloq::ei::fomo;

Adafruit_PCF8574 pcf;
Servo baseservo; 
Servo midservo;

int buttonState = 0;
int i = 0;
int x=6;
int y=6;
int a;
int b;
bool c;
bool d;
bool t = false;

int pos_x = 0;
int last_target_x = 0;
int target_x = 0;
int center_cam_x = 48;

int pos_y = 0;
int last_target_y = 0;
int target_y = 0;
int center_cam_y = 48;

LCD_I2C lcd(0x27, 16, 2); 

const char ssid[] = "Ok";
const char pass[] = "q12345678";

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
      baseservo.write(x*15);
      midservo.write(y*15);
      Serial.println(x);
      Serial.println(y);
  }


void setup() {

    
    Serial.begin(115200);
    Wire.begin(SDA, SCL);
    Serial.println("__EDGE IMPULSE FOMO (NO-PSRAM)__");

    // camera settings
    // replace with your own model!
    camera.pinout.aithinker();
    camera.brownout.disable();
    // NON-PSRAM FOMO only works on 96x96 (yolo) RGB565 images
    camera.resolution.yolo();
    camera.pixformat.rgb565();

    // init camera
    while (!camera.begin().isOk())
        Serial.println(camera.exception.toString());
    
    Serial.println("Camera OK");
    Serial.println("Put object in front of camera");
    baseservo.attach(12); 
    midservo.attach(13);

    if (!pcf.begin(PCF8574_Address, &Wire)) {
    Serial.println("Couldn't find PCF8574");
    }
    pcf.pinMode(0, INPUT);
    pcf.pinMode(1, INPUT);
    
    lcd.begin();
    baseservo.attach(12); 
    midservo.attach(13);
    WiFi.begin(ssid, pass);

    // Note: Local domain names (e.g. "Computer.local" on OSX) are not supported
    // by Arduino. You need to set the IP address directly.
    client.begin(mqtt_broker, MQTT_PORT, net);
    client.onMessage(messageReceived);
    lcd.backlight();
    connect();
}


void loop() {


    // capture picture
    if (!camera.capture().isOk()) {
        Serial.println(camera.exception.toString());
        return;
    }

    // run FOMO
    if (!fomo.run().isOk()) {
      Serial.println(fomo.exception.toString());
      return;
    }

    // how many objects were found?
    Serial.printf(
      "Found %d object(s) in %dms\n", 
      fomo.count(),
      fomo.benchmark.millis()
    );

    // if no object is detected, return
    if (!fomo.foundAnyObject())
      return;

    // if you expect to find a single object, use fomo.first
    Serial.printf(
      "Found %s at (x = %d, y = %d) (size %d x %d). "
      "Proba is %.2f\n",
      fomo.first.label,
      fomo.first.x,
      fomo.first.y,
      fomo.first.width,
      fomo.first.height,
      fomo.first.proba
    );
    
    
     
    // if you expect to find many objects, use fomo.forEach
//    if (fomo.count() > 1) {
//      fomo.forEach([](int i, bbox_t bbox) {
//        Serial.printf(
//          "#%d) Found %s at (x = %d, y = %d) (size %d x %d). "
//          "Proba is %.2f\n",
//          i + 1,
//          bbox.label,
//          bbox.x,
//          bbox.y,
//          bbox.width,
//          bbox.height,
//          bbox.proba
//        );
//      });
//    }

  target_x = (int)fomo.first.x;
  if(abs(target_x - last_target_x) < 10){                     //Safety about Frame-Lacking
    if(center_cam_x - target_x > 2){
      pos_x -= 1;
      baseservo.write(pos_x);
    }
    else if(center_cam_x - target_x < 2){
      pos_x += 1;
      baseservo.write(pos_x);
    }
  }
  last_target_x = target_x;
  Serial.println("Base Servo Position: " + String(pos_x));



  target_y = (int)fomo.first.y;
  if(abs(target_y - last_target_y) < 20){                     //Safety about Frame-Lacking
    if(center_cam_y - target_y > 5){
      pos_y -= 1;
      midservo.write(pos_y);
    }
    else if(center_cam_y - target_y < 5){
      pos_y += 1;
      midservo.write(pos_y);
    }
  }
  last_target_y = target_y;
  Serial.println("Mid Servo Position: " + String(pos_y));

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


