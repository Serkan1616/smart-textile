//#include <SoftwareSerial.h>                                   //SoftwareSerial library
#include <WiFi.h>
#include "ThingSpeak.h"
#include <DabbleESP32.h>

#define CHANNEL_ID 2410900
#define CHANNEL_API_KEY "9GJTS3MCW8O84N0L"

#define CUSTOM_SETTINGS
#define INCLUDE_NOTIFICATION_MODULE

#define INCLUDE_SENSOR_MODULE  //yeni
#define INCLUDE_DATALOGGER_MODULE   //yeni
bool isFileOpen = true;    //yeni

WiFiClient client;

/*
String agAdi = "FiberHGW_ZTSRU2_2.4GHz";                 //Write network name here    
String agSifresi = "74zbkXkHh4zd";           //Write network password here
*/
const char* ssid = "Yasin";
const char* password = "11111111";
unsigned long previousMillis = 0;
unsigned long interval = 30000;
//On arduino cables cross. So rxPin connected to the 11th and txPin connected to the 10th port.
//int rxPin = 10;                                               //ESP8266 RX pin
//int txPin = 11;                                               //ESP8266 TX pin
int mq2_pin = 36; //old value A1             
//int mq4_pin = A0;
int smoke, CNG;
int buzzer_pin = 5;
int green_led_safe = 23;
int red_led_smoke = 18; //mq2 //old value 5
int blue_led_data = 16;  //mq4
#define treshold_smoke 1000//360 //mq2
#define treshold_cng  290//220  //mq4
String ip = "184.106.153.149";                                //Thingspeak ip address
#define preheat_time 5000                     //Preheat needed for sensors to work well

int high_green_mq2  = 1; //control flag
int high_green_mq4  = 1;  //control flag
int duration = 100;

int count=0;

//SoftwareSerial esp(rxPin, txPin);                             //Serial communication pin settings
void initWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi ..");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(1000);
  }
  Serial.println(WiFi.localIP());
}

void initializeFile(){         //yeni
  Serial.println("Initialize");
  DataLogger.createFile("Gas values");
  DataLogger.createColumn("Values");
}


void setup() {  
  
  
  Serial.begin(9600);  //Serial port communication baudrate definition
  
  Dabble.begin("Esp32");                     //set bluetooth name of your device
  DataLogger.sendSettings(&initializeFile);    //yeni
  
  
  initWiFi();
  Serial.print("RRSI: ");
  Serial.println(WiFi.RSSI());

  ThingSpeak.begin(client);
  
  Serial.println("Started");
  //pinMode(mq4_pin, INPUT);
  pinMode(mq2_pin, INPUT);
  pinMode(green_led_safe, OUTPUT);
  pinMode(red_led_smoke, OUTPUT);
  pinMode(blue_led_data, OUTPUT);
  pinMode(buzzer_pin, OUTPUT);

  Dabble.waitForAppConnection();               //waiting for App to connect
  Notification.clear();                        //clear previous notifictions
  Notification.setTitle("Exceeding Threshold Value");      //Enter title of your notification

  delay(preheat_time);
 
}

void loop() {

  

 Dabble.processInput();    //this function is used to refresh data obtained from smartphone.Hence calling this function is mandatory in order to get data properly from your mobile.

  // put your main code here, to run repeatedly:

  digitalWrite(blue_led_data, LOW);
  unsigned long currentMillis = millis();
  if ((WiFi.status() != WL_CONNECTED) && (currentMillis - previousMillis >=interval)) {
    Serial.print(millis());
    Serial.println("Reconnecting to WiFi...");
    WiFi.disconnect();
    WiFi.reconnect();
    Serial.print(WiFi.status()); // wifi status yazdır
    previousMillis = currentMillis;
  }
  
  int smoke = analogRead(mq2_pin);  //Read from MQ-2 sensor to detect smoke leakage
  Serial.print("SMOKE: ");
  Serial.println(smoke);
  if( isFileOpen == true)      //yeni
  {
    DataLogger.send("Values",smoke);
    count++;
    
  }
  if( count == 100) {
    isFileOpen = false;
    DataLogger.stop();
    while(true){
      
    }
    }
   if(smoke > treshold_smoke){  //İf smoke detected red led will be high

    Notification.notifyPhone(String("Danger Value: ") + smoke);
    tone(buzzer_pin, 3000, 1000);
    digitalWrite(green_led_safe, LOW);
    digitalWrite(red_led_smoke, HIGH);
    //high_green_mq2 = 0;
    delay(duration);
    noTone(buzzer_pin); 
   }
   else{               //İf smoke not detected red led will be low
    digitalWrite(red_led_smoke, LOW);
    digitalWrite(green_led_safe, HIGH);
    //high_green_mq2 = 1;
   }
int status = ThingSpeak.writeField(CHANNEL_ID, 1, smoke, CHANNEL_API_KEY);

if (status == 200) {
  digitalWrite(blue_led_data, HIGH);
  Serial.println("Data sent successfully!");
} else {
  //Serial.print("Failed to send data. HTTP error code: ");
  //Serial.println(status);
}

  delay(1000);


}