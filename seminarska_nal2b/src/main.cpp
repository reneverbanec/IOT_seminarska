#include <Arduino.h>
#include <WiFi.h>
#include <ArduinoJson.h>
#include <WebSocketsClient.h>

const char* ssid = "Mojca";
const char* password = "realmadrid";

const char* serverAddress = "192.168.1.13";
const int serverPort = 3000;

const int potPin = 34;

WebSocketsClient webSocketClient;


void webSocketEvent(WStype_t type, uint8_t *payload, size_t length) {
  switch (type) {
    case WStype_DISCONNECTED: // Če je povezava prekine
      Serial.println("WebSocket disconnected");
      break;
    case WStype_CONNECTED: // Ko se povezava vzpostavi
      Serial.println("WebSocket connected");
      break; 
    case WStype_TEXT: // Prejeto sporočilo
      Serial.printf("Received: %s\n", payload);
      break;
    default:
      break;
  }
}


void setup() {
  pinMode(2, OUTPUT); // tu določimo, da je pin 2 izhod (vgrajena LED dioda)
  pinMode(potPin, INPUT); // določimo da je PIN34 vhod (analogne vrednosti)

  // Zagon komunikacije preko serijskih vrat oz. USB
  Serial.begin(115200);

  //Povezava z WIFI
  Serial.println("Povezovanje");
  WiFi.begin(ssid, password);
  while(WiFi.status() != WL_CONNECTED){
    delay(500);
    Serial.print(".");
  }

  // Izpišemo IP naslov
  Serial.println("Modul ESP32 je povezan !");
  Serial.print("IP naslov esp32 modula: ");
  Serial.println(WiFi.localIP());

  // vzpostavimo WebSocket povezavo
  webSocketClient.begin(serverAddress, serverPort);
  webSocketClient.onEvent(webSocketEvent);

  delay(500);

  // Indikacija delovanja modula
  digitalWrite(2, 1); // na pin 2 pošljemo 1
  delay(500);
  digitalWrite(2, 0); // na pin 2 pošljemo 1
  delay(500);
  digitalWrite(2, 1); // na pin 2 pošljemo 1
  delay(500);
  digitalWrite(2, 0); // na pin 2 pošljemo 1
  delay(500);



}

  void sendData(int value) {
    StaticJsonDocument<64> jsonDoc;
    jsonDoc["type"] = "mappedValue"; //Tip podatka
    jsonDoc["value"] = value; // vrednost potenciometra
    String jsonString;
    serializeJson(jsonDoc, jsonString); // pretvorimo v JSON niz
    Serial.println(jsonString); // izpišemmo niz
    webSocketClient.sendTXT(jsonString); // Pošiljanje podatkov preko Websocket
  }


void loop() {
  // put your main code here, to run repeatedly:
  // tu zapišemo kodo, ki se ponavljajoče izvaja:
  webSocketClient.loop();
  int photoresistorValue = analogRead(potPin); // preberemo vrednost fotoupornika
  int mappedValue = map(photoresistorValue, 0, 4095, 0, (int)((3.3 / 5) * 100)); // zapišemo novo vrednost v mappedValue z uporabo funkcije map();
  sendData(mappedValue);
  delay(200);

}