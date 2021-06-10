// Define libraries

#include <WiFi.h>
#include <DHTesp.h>  // Library to be used to read the DHT22 sensor
#include <PubSubClient.h>
#define dhtPin 23  
DHTesp dht;
//DHT dht(DHTPIN, DHTTYPE);

// MQTT connection parameters--------------------------------------
const char* MQTT_USER = "user1"; // *** Replace with your username, e.g. user2, user3 etc. (leave it as it is for the test)
const char* MQTT_CLIENT = "user1";  // *** Must be unique, replace with your username, e.g. user2, user3 etc. (same as MQTT_USER)
const char* MQTT_TOPIC_ESP32 = "esp32";
const char* MQTT_TOPIC_SERVER = "server";
const char* MQTT_ADDRESS = "esp-32.zapto.org";
//----------------------------------------------------------------
// WiFi connection parameters (WPA security protocol) ------------
const char* WIFI_SSID     = "VODAFONE_2885";
const char* WIFI_PASSWORD = "korinaki2590";
//-----------------------------------------------------------------

unsigned long measPreviousMillis = 0;
//-----------------------------------------------------------------

unsigned long currentMillis = 0;
char topicControl[150],topicData[150], topicDataCrc[150];
char Delay[3];
char Temp[3];
char Humid[3]; 
int delayThreshold, tempThreshold, humidThreshold;
byte d1, t1, h1;
bool startCrc;
bool startMeasuring=false;
bool convertInt2Char;
char buffer1[10];
double tempArray[5];
double x[5];

///////////////////////////////////////////////////////////////
void callback(char*, byte*, unsigned int);

WiFiClient wifiClient;
PubSubClient client(MQTT_ADDRESS, 1883, callback, wifiClient);


void callback(char* topic, byte* payload, unsigned int length) {
  char p[length + 1];
  memcpy(p, payload, length);
  p[length] = 0;
  String message(p);
  String topic_str(topic);
  //Print received messages and topics---------------------------
  Serial.println("");
  Serial.println("Callback function");
  Serial.print("Topic: ");
  Serial.println(topic);
  Serial.print("Message: ");
  Serial.println(message);
  //--------------------------------------------------------------
  // Check received messages and topics---------------------------
   if (topic_str == topicData){
      Serial.println("message from callback function from topicData returns: " + message);
      //Serial.println(message);
      sixDigitCode(message);
  }
  if (topic_str == topicControl) {
  if (message == "startMeasurements"){
    Serial.print("from callback function\n"); Serial.print(message);
    startMeasuring = true;
  }
  if(message == "crcError"){
    Serial.println("Error in the crc code");
  }
  }  
}

//---------------------------------------------------------------

void sixDigitCode(String brokerMessage) {  
  Delay[0] = brokerMessage[0];
  Delay[1] = brokerMessage[1];
  Delay[2] = '\0';
  delayThreshold = atoi(Delay);//converts the string argument to an integer for manipulating with CRC
  d1 = lowByte(delayThreshold); //Extract the low-order (rightmost) byte
  Serial.println("Delay = "+ String(delayThreshold));
   
  Temp[0] = brokerMessage[2];
  Temp[1] = brokerMessage[3];
  Temp[2] = '\0';
  tempThreshold = atoi(Temp);
  t1 = lowByte(tempThreshold);
  Serial.println("Temp = " + String(tempThreshold));
  
  Humid[0] = brokerMessage[4];
  Humid[1] = brokerMessage[5];
  Humid[2] = '\0';
  humidThreshold = atoi(Humid); 
  h1 = lowByte(humidThreshold);
  Serial.println("Humidity = " + String(humidThreshold));
  
  crcCalc();
  //publishCrc();
  
  startCrc=true;
  client.loop();
}

//---------------

byte crc8(byte x){  //http://www.sunshine2k.de/articles/coding/crc/understanding_crc.html
  const byte generator = 0x01;//0000-0001
  byte crc = x;

    for (int i = 0; i < 8; i++){
    
        if ((crc & 0x80) != 0){
         
            crc = (byte)((crc << 1) ^ generator);
        }
        else{
         /* most significant bit not set, go to next bit */
            crc <<= 1;
        }
    }

    return crc;
  
}

//----------

void crcCalc(){
  delayThreshold=crc8(d1);
  tempThreshold=crc8(t1);
  humidThreshold=crc8(h1);
  Serial.println("the crc for delay =  " + String(delayThreshold)); 
  Serial.println("the crc for Temp =  " + String(tempThreshold) ); 
  Serial.println("the crc for humidity =  " + String(humidThreshold) ); 
  startCrc = false;
  convertInt2Char = true;

  //saving all the values in a buffer for further manipulation.
  int buffer2[3];
  buffer2[0]= delayThreshold;  
  buffer2[1]= tempThreshold;
  buffer2[2]= humidThreshold;
  //zero padding for 9 digit CRC
  sprintf(buffer1, "%03d""%03d""%03d", buffer2[0],buffer2[1],buffer2[2]);
  Serial.println("Nine digit CRC is: "  + String(buffer1)+ "\n");
  
  client.publish(topicDataCrc, buffer1);
  convertInt2Char = false;
}
//not in use. 
//void publishCrc(){ 
//  if (client.connected()) { 
//      client.publish(topicDataCrc, buffer1);
//      Serial.println("");
//      Serial.println("**************************");
//      Serial.println("Message published");  
//    }    
//}
  
//----------  
void mqttReconnect() {
  // Loop until we're reconnected----------------------------------
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect(MQTT_CLIENT, "samos", "karlovasi33#")) {
      Serial.println("MQTT connected");
      topicSubscribe();
    } 
    else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
     //Wait 5 seconds before retrying-------------------------------
      delay(5000);
    }
  }
}

void topicSubscribe() {
  if(client.connected()) {
    Serial.println("Subscribe to MQTT topics: ");
    Serial.println(topicControl);
    Serial.println(topicData);
    Serial.println(topicDataCrc);
    client.subscribe(topicControl);
    client.subscribe(topicData);
    client.subscribe(topicDataCrc);
    client.loop();
  }  
}

void initialization(){ //https://github.com/beegee-tokyo/DHTesp/blob/master/examples/DHT_ESP32/DHT_ESP32.ino
  // Initialize temperature sensor
  dht.setup(dhtPin, DHTesp::DHT22);
  Serial.println("Sensor is on");
  
}

bool getTempHumid(){ //https://github.com/beegee-tokyo/DHTesp/blob/master/examples/DHT_ESP32/DHT_ESP32.ino
  TempAndHumidity newValues = dht.getTempAndHumidity();
  // Check if any reads failed and exit early (to try again).
  if (dht.getStatus() != 0) {
    Serial.println("DHT22 error status: " + String(dht.getStatusString()));
    return false;
  }
  Serial.println("Temperature:" + String(newValues.temperature) + " Humidity:" + String(newValues.humidity));
  if (newValues.temperature > tempThreshold){
    Serial.println("Temperature high. "); 
  }
  if (newValues.humidity > humidThreshold){
    Serial.println("Humidity high. ");
  }
  for (int i=0; i<5; i++){
    //Make room for new array element by shifting to right
    tempArray[i] = tempArray[i+1];
    } 
    //Insert new element at given position and increment size 
     tempArray[5-1]=newValues.temperature;
    
  return true;
}

//https://www.seas.upenn.edu/~biophys/training_manuals/virtual_workshop_in_parallel_computing/mpi2/least-squares.c
void leastSquare(){
  // x axis 
 double  x[5]={0*delayThreshold,1*delayThreshold,2*delayThreshold,3*delayThreshold, 4*delayThreshold};
 double SUMx=0, SUMy=0, SUMxy=0, SUMxx=0, slope, b;
  for (int i=0; i<5; i++){
    SUMx = SUMx + x[i];
    SUMy = SUMy + tempArray[i];
        SUMxy = SUMxy + x[i]*tempArray[i];
        SUMxx = SUMxx + x[i]*x[i];
  }
    
  slope = ( SUMx*SUMy - 5*SUMxy ) / ( SUMx*SUMx - 5*SUMxx );
  b = ( SUMy - slope*SUMx ) / 5;
  Serial.println("y="+String(slope)+"x + "+'('+String(b)+')');
}

void wifiSetup(){
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Wait for WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi Connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  return;
  //------------------------------------------------------------------ 
}


void setup() {
  
  // Connect to WiFi and establish serial connection---------------- 
 Serial.begin(115200);
  delay(1000);
  wifiSetup();
  
  // Connect to MQTT broker and subscribe to topics
  
  client.setCallback(callback);
  
  // Define MQTT topic names
 
  sprintf(topicControl, "%s/%s", MQTT_USER, "control");
  sprintf(topicData, "%s/%s", MQTT_USER, "data");
  sprintf(topicDataCrc, "%s/%s", MQTT_USER, "dataCrc");
 
  // Subscribe to topics and reconnect to MQTT server
  
  mqttReconnect();
  initialization();
  
  if (client.connected()) { // if MQTT server connected
      client.publish(topicControl, "sendConfig");   
      Serial.println("");
      Serial.println("**************************");
      Serial.println("Message published");
    }

   startCrc = false;
   convertInt2Char = false;
  
}

void loop() {
  
  // Reconnect to WiFi if not connected--------------------------------
 if (WiFi.status() != WL_CONNECTED) {
     wifiSetup();
  } 
  
  //--------------------------------------------------------------------
  
  currentMillis = millis();
  if (startMeasuring==true){
  if ((currentMillis - measPreviousMillis) >= (delayThreshold*1000)) { 
  getTempHumid();
  if(currentMillis>=5*(delayThreshold*1000)){
  leastSquare();
  }
  measPreviousMillis = currentMillis;

// Uncoment for array shifting display
//   for (int i=0; i<5; i++){    
//     Serial.println(" array["+String(i)+"]="+String(tempArray[i]));
//    }
  }
  }
 
  //------------------------------------------------------------------- 
  
  // Reconnect to MQTT broker if not connected--------------------------
  if (!client.connected()) { mqttReconnect(); }
  //--------------------------------------------------------------------
  if (startCrc== true){
    crcCalc();
    //publishCrc();
  }
  if (convertInt2Char == true){
    //int2Char();
    crcCalc();
  }
  
  client.loop();
}
