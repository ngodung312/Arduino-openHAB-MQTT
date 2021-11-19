#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#define lvroomSwitch D7
#define bedroomSwitch D6

#define lvroomSensor D5
#define bedroomSensor D4

#define lvroomPin D3
#define bedroomPin D2

int lvroomLedState = 0;
int bedroomLedState = 0;
int lvroomSwitchState;
int bedroomSwitchState;

const char* ssid = "";
const char* password = "";
//const char* mqtt_server = "127.0.0.1";
const char* mqtt_server = "broker.mqttdashboard.com";

WiFiClient espClient;
PubSubClient client(espClient);

long lastMsg = 0;
char msg[50];
int value = 0;
int last_pub_time=0;

bool lvroomState = false;
bool bedroomState = false;

void setup_wifi()
{
  delay(100);
  // Start by connecting to a WiFi network
  Serial.print("Connecting to ");
  Serial.println(ssid);
  
//  WiFi.config(ip, gateway_dns, gateway_dns); 

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  randomSeed(micros());
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP Address: ");
  Serial.println(WiFi.localIP());
}

void lvroomPirSensor()
{
  int lvroomRead = digitalRead(lvroomSensor);
  //Serial.println(lvroomRead);
  if (lvroomRead == HIGH) 
  {
    Serial.println("RED Sensed Something");
    digitalWrite(lvroomPin, HIGH);
    client.publish("/ngodung312/switch/livingroom/state","1");
    delay(5000);
  }
  else
  {
    digitalWrite(lvroomPin, LOW);    
    client.publish("/ngodung312/switch/livingroom/state","0");
    Serial.println("RED Sensed Nothing");
  }
  delay(2000);  
}

void bedroomPirSensor()
{
  int bedroomRead = digitalRead(bedroomSensor);
  //Serial.println(bedroomRead);
  if (bedroomRead == HIGH)
  {
    Serial.println("YELLOW Sensed Something");
    digitalWrite(bedroomPin, HIGH);
    client.publish("/ngodung312/switch/bedroom/state","1");
    delay(5000);     
  }
  else
  {
    digitalWrite(bedroomPin, LOW);
    Serial.println("YELLOW Sensed Nothing");
    client.publish("/ngodung312/switch/bedroom/state","0");
  }
  delay(2000);  
}

void livingroomLedSwitch()
{
  // read the state of the pushbutton value:
  lvroomSwitchState = digitalRead(lvroomSwitch);

  // check if the pushbutton is pressed. If it is, the buttonState is HIGH:
  if (lvroomSwitchState == HIGH) 
  {
    if (lvroomLedState == 0)
    {
      // turn LED on:
      digitalWrite(lvroomPin, HIGH);
      client.publish("/ngodung312/switch/livingroom/state","1");
      lvroomLedState = 1;
    } 
    else 
    {
      // turn LED off:
      digitalWrite(lvroomPin, LOW);
      client.publish("/ngodung312/switch/livingroom/state","0");
      lvroomLedState = 0;
    }  
  }
  
// 	print out the state of the button
//  Serial.print("LVR: ");
//  Serial.println(lvroomSwitchState);
  delay(500);
}

void bedroomLedSwitch()
{
  // read the state of the pushbutton value:
  bedroomSwitchState = digitalRead(bedroomSwitch);

  // check if the pushbutton is pressed. If it is, the buttonState is HIGH:
  if (bedroomSwitchState == HIGH) 
  {
    if (bedroomLedState == 0)
    {
      // turn LED on:
      digitalWrite(bedroomPin, HIGH);
      client.publish("/ngodung312/switch/bedroom/state","1");
      bedroomLedState = 1;
    } 
    else 
    {
      // turn LED off:
      digitalWrite(bedroomPin, LOW);
      client.publish("/ngodung312/switch/bedroom/state","0");
      bedroomLedState = 0;
    }  
  }
  
  // print out the state of the button
//  Serial.print("BR: ");
//  Serial.println(bedroomSwitchState);
  delay(500);
}

void callback(char* topic, byte* payload, unsigned int length)
{
  //Serial.print("Message arrived [");
  //Serial.print(topic);
  //Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.println((char)payload[i]);
  }

  if (strcmp(topic,"/ngodung312/sensor/livingroom/command")==0)
    if (payload[0] == '1')
      lvroomState = true; 
    else 
      lvroomState = false;
  //Serial.println(lvroomState);
  
  if (strcmp(topic,"/ngodung312/sensor/bedroom/command")==0)
    if (payload[0] == '1')
      bedroomState = true; 
    else 
      bedroomState = false;

  if (strcmp(topic,"/ngodung312/switch/livingroom/command")==0) 
  { 
    if (payload[0] == '0') 
      digitalWrite(lvroomPin, LOW);
    else  
      digitalWrite(lvroomPin, HIGH);
  } 
  
  if (strcmp(topic,"/ngodung312/switch/bedroom/command")==0) 
  { 
    if (payload[0] == '0') 
      digitalWrite(bedroomPin, LOW);
    else  
      digitalWrite(bedroomPin, HIGH);
  } 
}

void reconnect()
{
  // Loop until we're reconnected
  while (!client.connected())
  {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ngodung312";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    // if your MQTT broker has clientID, username and password
    // please change following line to => if(client.connect(clientId, userName, passWord))
    //if (client.connect(clientId.c_str()))
    if(client.connect(clientId.c_str(), "ngodung312", "66668888"))
    {
      Serial.println("Connected");
      // once connected to MQTT broker, subscribe command if any
      client.publish("outTopic", "hello world");
      // resubscribe
      client.subscribe("/ngodung312/switch/livingroom/command");
      client.subscribe("/ngodung312/switch/bedroom/command");
      client.subscribe("/ngodung312/sensor/livingroom/command");
      client.subscribe("/ngodung312/sensor/bedroom/command");
    } else {
      Serial.print("Failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds ");
      // Wait 6 seconds before retrying
      delay(6000);
    }
  }
} //end reconnect()

void setup()
{
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  
  pinMode(lvroomPin, OUTPUT);
  digitalWrite(lvroomPin, LOW);

  pinMode(bedroomPin, OUTPUT);
  digitalWrite(bedroomPin, LOW);

  pinMode(lvroomSensor, INPUT_PULLUP);
  pinMode(bedroomSensor, INPUT_PULLUP);
    
  pinMode(lvroomSwitch, INPUT);
  pinMode(bedroomSwitch, INPUT);
}

void loop() 
{
  if (!client.connected()) 
  {
    reconnect();
  }
  
  livingroomLedSwitch();
  bedroomLedSwitch();
  
  if (lvroomState == true)
    lvroomPirSensor();
  
  if (bedroomState == true)
    bedroomPirSensor();
    
  client.loop();
}
