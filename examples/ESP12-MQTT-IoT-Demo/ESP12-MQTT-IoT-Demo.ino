#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WiFiMulti.h>

#include <PubSubClient.h>

ESP8266WiFiMulti wifiMulti;
WiFiClient espClient;
PubSubClient client(espClient);

const char* brokerUser = "your_mqttBroker_username";  // exp: myemail@mail.com
const char* brokerPass = "your_mqttBroker_pass";
const char* broker = "your_broker_server";
const char* adcTopic =  "/your_mqttBroker_username/out/adc";
const char* statTopic = "/your_mqttBroker_username/out/stat";
const char* inTopic =   "/your_mqttBroker_username/in";

long currentTime, lastTime;
bool enADC = false;

void setupWiFi()
{
  delay(100);
  
  wifiMulti.addAP("ssid_from_AP_1", "your_password_for_AP_1");
  wifiMulti.addAP("ssid_from_AP_2", "your_password_for_AP_2");
  wifiMulti.addAP("ssid_from_AP_3", "your_password_for_AP_3");

  Serial.println("Connecting ...");

  digitalWrite(LED_BUILTIN, LOW);

  while (wifiMulti.run() != WL_CONNECTED) 
  {
    delay(250);
    Serial.print('.');
  }

  digitalWrite(LED_BUILTIN, HIGH);
  
  Serial.println('\n');
  Serial.print("Connected to:\t");
  Serial.println(WiFi.SSID());
  
  Serial.print("IP address:\t");
  Serial.println(WiFi.localIP());
}

void reconnect()
{
  while(!client.connected())
  {
    Serial.print("\nConnecting to ");
    Serial.println(broker);

    if(client.connect("WimosD1", brokerUser, brokerPass))
    {
      Serial.print("\nConnected to ");
      Serial.println(broker);
      client.subscribe(inTopic);
    }
    else
    {
      Serial.println("Connecting");
      delay(2500);
    }
  }
}

void callback(char* topic, byte* payload, unsigned int len)
{
  Serial.print("Received messages: ");
  Serial.println(topic);
  for(unsigned int i=0; i<len; i++)
  {
    Serial.print((char) payload[i]);

    if(len == 1 && payload[0] == '1')
    {
      digitalWrite(LED_BUILTIN, LOW);
      enADC = true;
    }
    else if(len == 1 && payload[0] == '0')
    {
      digitalWrite(LED_BUILTIN, HIGH);
      enADC = false;
    }
  }
  Serial.println();
}

void setup()
{
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(A0,INPUT);
  digitalWrite(LED_BUILTIN, HIGH);
  
  Serial.begin(115200);
  setupWiFi();
  client.setServer(broker, 1883);
  client.setCallback(callback);
}

void loop()
{
  float adcValue;

  char messagesStat[50];
  char messagesADC[50];
  
  if(!client.connected())
  {
    reconnect();
  }

  client.loop();

  currentTime = millis();
  if(currentTime - lastTime > 1500)
  {
    if(enADC)
    {
      adcValue = 1.0 * analogRead(A0) / 1024;
      
      snprintf(messagesStat, 75, "Enabled");
      dtostrf(adcValue, 1, 4, messagesADC);
    }
    else
    {
      snprintf(messagesStat, 50, "Disabled");
    }

    Serial.print("Sending messages:\t");
    Serial.println(messagesStat);
    Serial.println(messagesADC);
    
    client.publish(statTopic, messagesStat);
    
    if(enADC)
    {
      client.publish(adcTopic, messagesADC);
    }
    
    lastTime = millis();
  }
}
