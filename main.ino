
//
//Requires PubSubClient found here: https://github.com/knolleary/pubsubclient
//Requires DHT from Adafruit here: https://github.com/adafruit/DHT-sensor-library
//
//ESP8266 MQTT temp sensor node


#include <PubSubClient.h>
#include <ESP8266WiFi.h>
#include <DHT.h>
#define DHTTYPE DHT22
#define DHTPIN 0



// Initialize DHT sensor 
// NOTE: For working with a faster than ATmega328p 16 MHz Arduino chip, like an ESP8266,
// you need to increase the threshold for cycle counts considered a 1 or 0.
// You can do this by passing a 3rd parameter for this threshold.  It's a bit
// of fiddling to find the right value, but in general the faster the CPU the
// higher the value.  The default for a 16mhz AVR is a value of 6.  For an
// Arduino Due that runs at 84mhz a value of 30 works.
// This is for the ESP8266 processor on ESP-01 
DHT dht(DHTPIN, DHTTYPE, 11); // 11 works fine for ESP8266

float humidity, temp_c, temp_f;          // Values read from sensor

unsigned long previousMillis = 0;        // will store last temp was read
const long interval = 2000;              // interval at which to read sensor

//EDIT THESE LINES TO MATCH YOUR SETUP
#define MQTT_SERVER "###.###.###.###"
const char* ssid = "############";
const char* password = "##########";


//topic to publish to for the temperature
char* tempTopic = "your temperature topic";
char* currentTemp;

char* humTopic = "your humidity topic";
char currentHum;

//MQTT callback
void callback(char* topic, byte* payload, unsigned int length) {
  
}

WiFiClient wifiClient;
PubSubClient client(MQTT_SERVER, 1883, callback, wifiClient);

//networking functions

void reconnect() {

  //attempt to connect to the wifi if connection is lost
  if(WiFi.status() != WL_CONNECTED){

    //loop while we wait for connection
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
    }

  }

  //make sure we are connected to WIFI before attemping to reconnect to MQTT
  if(WiFi.status() == WL_CONNECTED){
  // Loop until we're reconnected to the MQTT server
    while (!client.connected()) {

      // Generate client name based on MAC address and last 8 bits of microsecond counter
      String clientName;
      clientName += "esp8266-";
      uint8_t mac[6];
      WiFi.macAddress(mac);
      clientName += macToStr(mac);

      //if connected, subscribe to the topic(s) we want to be notified about
      if (client.connect((char*) clientName.c_str())) {
        //subscribe to topics here
      }
    }
  }
}

//generate unique name from MAC addr
String macToStr(const uint8_t* mac){

  String result;

  for (int i = 0; i < 6; ++i) {
    result += String(mac[i], 16);

    if (i < 5){
      result += ':';
    }
  }

  return result;
}

void gettemperature() {
  // Wait at least 2 seconds seconds between measurements.
  // if the difference between the current time and last time you read
  // the sensor is bigger than the interval you set, read the sensor
  // Works better than delay for things happening elsewhere also
  unsigned long currentMillis = millis();
  

 
    if(currentMillis - previousMillis >= interval) {
      // save the last time you read the sensor 
      previousMillis = currentMillis;   
 
      // Reading temperature for humidity takes about 250 milliseconds!
      // Sensor readings may also be up to 2 seconds 'old' (it's a very slow sensor)
      humidity = dht.readHumidity();          // Read humidity (percent)
      temp_f = dht.readTemperature(true);     // Read temperature as Fahrenheit
      temp_c = (((temp_f-32.0)*5/9)-1);       // Convert temperature to Celsius + Correction
      // Check if any reads failed and exit early (to try again).
      if (isnan(humidity) || isnan(temp_c)) {
        Serial.println("Failed to read from DHT sensor!");
        return;
      }      
    }
  
}


void setup() {

  //null terminate the temp string to be published
  

  //start the serial line for debugging
  Serial.begin(115200);
  dht.begin(); 
  delay(100);
  
  Serial.println("");
  Serial.println("DHT Temp/Hum MQTT Client");

  //start wifi subsystem
  WiFi.begin(ssid, password);
  WiFi.enableAP(false);
  

  //attempt to connect to the WIFI network and then connect to the MQTT server
  reconnect();

  //wait a bit before starting the main loop
  delay(2000);
}



void loop(){

  // Send the command to update temperatures
  gettemperature();

 
  // Define 
  String str1 = String(temp_c); 
  // Length (with one extra character for the null terminator)
  int str1_len = str1.length() + 1; 
  // Prepare the character array (the buffer) 
  char char_array_temp[str1_len];
  // Copy it over 
  str1.toCharArray(char_array_temp, str1_len);
  
  String str2;
  if(humidity <= 100){
     str2 = String(int(humidity)); 
  }else{
    str2 = "nan"; 
  }
  int str2_len = str2.length() + 1; 
  char char_array_hum[str2_len];
  str2.toCharArray(char_array_hum, str2_len);
 
  //publish the new temperature
  client.publish(tempTopic, char_array_temp);
  client.publish(humTopic, char_array_hum);
  Serial.println(String((float)temp_c)+" : "+String((float)temp_f)+" : "+String((int)humidity));
  Serial.println(str2);

  delay(10000);

  //reconnect if connection is lost
  if (!client.connected() && WiFi.status() == 3) {reconnect();}
  //maintain MQTT connection
  client.loop();
  //MUST delay to allow ESP8266 WIFI functions to run
  delay(5000); 
}

