/*
 * Sample code #2 - IoT Workshop
 * Send button press count to MQTT broker, and listen to the channel to get back the count to play a music
 * 
 * Author: Raymond Xie
 * Date: 9/6/2016
 * 
 */
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <MQTT.h>

// HINT:  provide the following information
//
// WiFi connection: SSID and Password
const char* ssid = "your_wifi_ssid";
const char* password = "your_wifi_passwd";

// MQTT server params: server, port, user, password, and unique clientid
const char* mqtt_server = "m12.cloudmqtt.com";
const int mqtt_port = 11565; 
const char *mqtt_user = "ask_your_instructor";
const char *mqtt_pass = "ask_your_instructor";
const char *mqtt_clientid = "ask_your_instructor";  // follow the format:  CID-J1-Table01, CID-OOW-Table02
const char *mqtt_topic = "RX-music";            // prefix with your initials, so you don't interference with your fellow participants

WiFiClient espClient;
PubSubClient client(espClient, mqtt_server, mqtt_port);

// your button press input
const int buttonPin = D2; 
int buttonPushCounter = 0;   // counter for the number of button presses
int buttonState = 0;         // current state of the button
int lastButtonState = 0;     // previous state of the button

// play music
const int buzzerPin = D6;
byte names[] = {'c', 'd', 'e', 'f', 'g', 'a', 'b', 'C'};  
int tones[] = {1915, 1700, 1519, 1432, 1275, 1136, 1014, 956};
byte melody0[] = "2d2a1f2c2d2a2d2c2f2d2a2c2d2a1f2c2d2a2a2g2p8p8p8p";
byte melody1[] = "2c2e2g2C2C2g2e2c2f2d2a2c2d2a1f2c2d2a2a2g2p8p";

void playMelody(String payload) {
  byte melody[100];
  int MAX_COUNT = 24;
  int count = 0;
  int count2 = 0; 
  int count3 = 0;

  if( payload == "0" ) {
    MAX_COUNT = 24;  
    for(count=0; count < MAX_COUNT*2; count++) {
      melody[count] = melody0[count];  
    }
  }
  else if( payload == "1") {
    MAX_COUNT = 22;
    for(count=0; count < MAX_COUNT*2; count++) {
      melody[count] = melody1[count];  
    }
  }

  analogWrite(buzzerPin, 0);     
  for (count = 0; count < MAX_COUNT; count++) {
    for (count3 = 0; count3 <= (melody[count*2] - 48) * 30; count3++) {
      for (count2=0;count2<8;count2++) {
        if (names[count2] == melody[count*2 + 1]) {       
          analogWrite(buzzerPin,500);
          delayMicroseconds(tones[count2]);
          analogWrite(buzzerPin, 0);
          delayMicroseconds(tones[count2]);
        } 
        if (melody[count*2 + 1] == 'p') {
          // make a pause of a certain size
          analogWrite(buzzerPin, 0);
          delayMicroseconds(500);
        }
      }
    }
    delay(1);
  }  
}


// Initial one-time setup
void setup() {
  pinMode(buttonPin, INPUT);      
  pinMode(BUILTIN_LED, OUTPUT);     // Initialize the BUILTIN_LED pin as an output
  Serial.begin(115200);
    
  setup_wifi();
}

// Loop forever
void loop() {
  
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // check user input
  checkButtonPress();

  // pace it
  delay(500);
}


void checkButtonPress() {
  // Serial.print("checkButtonPress: ");
  // read the pushbutton input pin:
  buttonState = digitalRead(buttonPin);
  Serial.println(buttonState);
  
  // compare the buttonState to its previous state
  if (buttonState != lastButtonState) {
    // if the state has changed, increment the counter
    if (buttonState == HIGH) {
      // if the current state is HIGH then the button was pressed
      buttonPushCounter++;
      if( buttonPushCounter == 2 ) {
        // reset 
        buttonPushCounter = 0;
      }
      Serial.print("number of button pushes:  ");
      Serial.println(buttonPushCounter);

      // publish button press count
      client.publish(mqtt_topic, String(buttonPushCounter));

    }
  }
  
  // keep tracking buttonstate
  lastButtonState = buttonState;
}


// Callback function upon receiving message from MQTT
const int BUFFER_SIZE = 100;
void callback(const MQTT::Publish& pub) {
  Serial.print(pub.topic());
  Serial.print(" => ");
  String payload = pub.payload_string();
  Serial.println(payload);

  // play different music based on return value
  playMelody(payload);
  delay(1);
}

// Connect to WiFi network
void setup_wifi() {
  delay(10);

  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

// Connect to MQTT broker
void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    Serial.print(mqtt_clientid);
    Serial.print("  ");
    Serial.print(mqtt_user);
    
    // Attempt to connect
    if (client.connect(MQTT::Connect(mqtt_clientid).set_auth(mqtt_user, mqtt_pass))) {
      Serial.println("MQTT connected");
      
      // subscribe to a topic channel
      client.subscribe(mqtt_topic);
      client.set_callback(callback);
    } 
    else {
      Serial.print("failed, rc=");
      // Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}


