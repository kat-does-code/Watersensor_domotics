#include <Wire.h>
#include <WiFi.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP085.h>
#include <base64.h>
#include <HTTPClient.h>

/*
 * Pinout for water sensor & BMP:
 * BMP    SDA      SCL      +      -
 * ESP    21       22       3.3    GND
 * 
 * Water  S        +        -
 * ESP    35       5V       GND
 */

//#define DEBUG

// system vars
bool overridePrintOut = false;

// wifi
struct s_wifi {
  const char* ssid = "Domoticz";
  const char* pass = "";
  WiFiClient client;
} wifi;

// domoticz
struct s_domoticz{
  const char * ip = "192.168.10.52"; //Domoticz port
  int port = 8080; //Domoticz port
} domoticz;


// sensors
Adafruit_BMP085 bmp;
int waterPIN = 35;

#ifdef DEBUG
void debug(){

  if(WiFi.status()== WL_CONNECTED){ //Check WiFi connection status
  
    HTTPClient http; //Declare an object of class HTTPClient
    http.begin(String("http://" + String(domoticz.ip) + ":" + String(domoticz.port) + "/json.htm?type=devices")); //Specify request destination
    int httpCode = http.GET(); //Send the request
    if (httpCode > 0) { //Check the returning code
      String payload = http.getString(); //Get the request response payload
      Serial.println(payload); //Print the response payload
    }
    http.end(); //Close connection
  } else {
    Serial.println("Error in WiFi connection");   
  }
}
#endif

void sendRequest(String request = "", bool printOut = false){
    wifi.client.println(request);
    if(printOut || overridePrintOut) Serial.println(request);
}

void updateDomoticz(int idx, float value, bool printOut = false)
{
  // http://domoticz-ip<:port>/json.htm?username=<USERNAME>=&password=<PASSWORD>=&api-call

  // Temperature
  ///json.htm?type=command&param=udevice&idx=IDX&nvalue=0&svalue=TEMP

  // Water level (pressure)
  ///json.htm?type=command&param=udevice&idx=IDX&nvalue=0&svalue=BAR

  if(WiFi.status()== WL_CONNECTED){
    HTTPClient http; 
    String req = ("http://" + String(domoticz.ip) + ":" + String(domoticz.port) + "/json.htm?type=command&param=udevice");
    req += "&idx=" + String(idx);
    req += "&nvalue=0&svalue=" + String(value);
    http.begin(req);
    int httpCode = http.GET();
    http.end(); //Close connection
  } else {
    Serial.println("Error in WiFi connection");   
  }
}

float getTemp(bool printOut = false) {
  float temp = bmp.readTemperature();
  if(printOut || overridePrintOut){
    Serial.print("BMP says temp: ");
    Serial.println(bmp.readTemperature());
  }
  return temp;
}

float getWater(bool printOut = false){
  float waterLevel = analogRead(waterPIN);
  if(printOut || overridePrintOut){
    Serial.print("Water measured: ");
    Serial.println(waterLevel);
  }
  return waterLevel;
}

void setupBMP(){
  if(!bmp.begin())
  {
    /* There was a problem detecting the BMP085 ... check your connections */
    Serial.print("Oops, no BMP085 detected ... Check your wiring or I2C ADDR!");
    while(1);
  }
}

void setupWifi(void){
  int i = -1;
  WiFi.begin(wifi.ssid, wifi.pass);
  while(WiFi.status() != WL_CONNECTED){
    i++;
    Serial.print("Attempt ");
    Serial.println(i);
    delay(1000);
    }
  Serial.print("Connected as ");
  Serial.println(WiFi.localIP());
}

void setup()
{
  Serial.begin(9600);
  Serial.println("starting connection");

  setupWifi();
  setupBMP();
}

void loop()
{
  #ifdef DEBUG
    debug();
  #else
    updateDomoticz(4, getWater());
    updateDomoticz(3, getTemp());

    delay(5000);
#endif
}
