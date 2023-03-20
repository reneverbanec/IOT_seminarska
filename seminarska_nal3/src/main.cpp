#include <Arduino.h>
#include <WiFi.h>
#include <ArduinoJson.h>
#include <WebSocketsClient.h>

const char* ssid = "Mojca";
const char* password = "realmadrid";

const char* serverAddress = "192.168.1.12";
const int serverPort = 3000;

const int photoPIN = 34; // fotoupornik
const int ledPIN = 5; // led pin

WebSocketsClient webSocketClient;

int lastPhotoresistorValue = -1; // Shrani zadnjo vrednost foto upora
bool ledStatusChanged = false; // Beleži spremembo stanja LED
bool ledStatus = false; // Beleži trenutno stanje LED
bool wasLedDisconnected = false; // Beleži, ali je bila LED prekinjena


const int ledCheckInterval = 500; // interval preverjanja LED
unsigned long lastLedCheck = 0; // zadnji čas preverjanja LED
int threshold = 20; // Prag zaznavanja

void webSocketEvent(WStype_t type, uint8_t *payload, size_t length) {
  switch (type) {
    case WStype_DISCONNECTED: {
      Serial.println("WebSocket disconnected");
      break;
    }
    case WStype_CONNECTED: {
      Serial.println("WebSocket connected");
      break;
    }
    case WStype_TEXT: { // če prejme text
      Serial.printf("Received: %s\n", payload); // izpiše sporočilo
      StaticJsonDocument<64> jsonDoc; // ustvari statični JSON dokument
      DeserializationError error = deserializeJson(jsonDoc, payload); //deserializacija
      if (error) { // če pride do napake
        Serial.println("Failed to parse JSON");
        return;
      }
      String messageType = jsonDoc["type"]; // prebere tip sporočila
      Serial.println(messageType); // izpiše tip sporočila
      if (messageType == "toggleLED") { // če je tip spororčila "toggleLED"
        ledStatus = !ledStatus; // spremeni stanje LED
        ledStatusChanged = true; // zabeleži spremembo
      }
      break;
    }
    default:
      break;
  }
}

void setup() {
  pinMode(ledPIN, OUTPUT); // pin kot izhod
  pinMode(photoPIN, INPUT); //pin kot vhod

  Serial.begin(115200);

  Serial.println("Povezovanje");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("Modul ESP32 je povezan !");
  Serial.print("IP naslov esp32 modula: ");
  Serial.println(WiFi.localIP());

  webSocketClient.begin(serverAddress, serverPort); // začni povezavo klienta
  webSocketClient.onEvent(webSocketEvent); // funkcija za obdelava dogodkov

  digitalWrite(ledPIN, HIGH);
  delay(200);
  digitalWrite(ledPIN, LOW);
  delay(200);
  digitalWrite(ledPIN, HIGH);
  delay(200);
  digitalWrite(ledPIN, LOW);  

  delay(500);
}

void sendData(int value, const char *type) { // funkcija za pošiljanje podatkov
  StaticJsonDocument<64> jsonDoc;
  jsonDoc["type"] = type; // nastavimmo tip sporočila
  jsonDoc["value"] = value; // nastavimo vrednost
  String jsonString; // ustvarimo JSON niz
  serializeJson(jsonDoc, jsonString); // serializitamo JSON
  Serial.println(jsonString);
  webSocketClient.sendTXT(jsonString); // pošljemo JSON niz preko websocketa
}

void loop() {
  webSocketClient.loop();
  int photoresistorValue = analogRead(photoPIN); // prebreemo vrednost fotoupornika
  int mappedValue = map(photoresistorValue, 0, 4095, 0, (int)((3.3 / 5) * 100)); // zapišemo vreednost

  if (lastPhotoresistorValue != mappedValue) { // če se je vrednost upornika spremenila
    sendData(mappedValue, "mappedValue"); // pošljemo novo vrednost
    lastPhotoresistorValue = mappedValue; // nastavimo novo vrednost za nadaljne preverjanje
  }

  if (ledStatusChanged) { // če se je stanje LED spremenilo
    digitalWrite(ledPIN, ledStatus ? HIGH : LOW); // nastavimo pin glede na PIN kateri je 1 ali 0
    sendData(ledStatus ? 1 : 0, "status"); // pošljemo sporočilo o trenutnem stanju LED diode
    ledStatusChanged = false; // ponastavimo vrednost
  }

if (millis() - lastLedCheck > ledCheckInterval) {// če je minilo več časa od preverjanja LED
    lastLedCheck = millis(); // posodobimo čas preverjanja
    if (ledStatus) { // če je LED prižgan
      int currentValue = analogRead(photoPIN); // preberemo trenutno vrednost fotouporanika
      int offValue = 0; // LED izklopljen
      if (abs(currentValue - offValue) < threshold) { // Če je razlika med trenutno in izklopljeno vrednostjo manjša od praga
        if (!wasLedDisconnected) { // Če LED dioda še ni bila izvlečena od prejšnjega stanja
          ledStatus = false; // spremenio stanje na izklopljeno
          sendData(0, "status"); // Pošljemo trenutno stanje LED
          sendData(1, "LEDDisconnected"); // Pošljemo sporočilo, da je povezava prekinjena z led
          Serial.println("LED disconnected");
          wasLedDisconnected = true; // nastavimo, da je bila povezava prekinjena 
        }
      } else {
        wasLedDisconnected = false; // drugače nastavimo, da LED ni prekinjena
      }
    }
  }

  delay(200);
}

