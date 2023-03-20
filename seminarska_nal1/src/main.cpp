#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <WebSocketsServer.h>
#include <ArduinoJson.h>

const char* ssid = "Mojca";
const char* password = "realmadrid";

int portNumber = 8811; // preko vrat 8811 se bo esp32 povezal na node.js strežnik

// port 80 je prednastavljeni port za posredovanje HTML stran
WebServer server(80); // objekt razreda WebServer (HTTP port, 80 je prednastavljeni port oz. vrata)
                      // vrednost porta posredujemo kot argument konstruktorja

// na vratih 81 ustvarimo strežnik spletnih vtičnikov - WebSocketsServer
WebSocketsServer webSocket = WebSocketsServer(81);

void handle_root() { // če dobimo zahtevo na strežnik ("request")
  server.send(200); // strežnik klientu pošlje kot odziv, t. j. "response"
                                        // HTML0 v argumentu je spremenljivka tipa String
                                        // v njej je spravljena naša spletna stran
                                        // koda 200 predstavlja standardni odziv na uspešno HTTP zahtevo - OK
}

void handle_NotFound() { // v primeru, da ne najdemo spletne strani, npr. če vpišemo /abc
  server.send(200, "text/html", "Spletna stran ni bila najdena."); // klientu sporočimo, da zahtevane spletne strani nismo našli na našem strežniku
}



// koda ki jse izvede ob prejemu sporočila preko spletnega vtičnika 8811
void onWebSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t lenght){
  switch (type)
  {
    case WStype_DISCONNECTED:
    {
      Serial.printf("[%u] Klient odklopljen!\n", num);
    }
    break;

  case WStype_CONNECTED:
    {
      IPAddress ip = webSocket.remoteIP(num);
      Serial.printf("[%u] Vzpostavljena WebSocket povezava iz IP naslova ", num);
      Serial.println(ip.toString());
    }
    break;
  case WStype_TEXT:
  {
    // v tem delu prenešeni string z JSON sporočilom pretvorimo v JSON objekt
    // <256> določa kapaciteto objekta doc - StaticJsonDocument - 256 bytov
    StaticJsonDocument<256> doc;
    // deserializacija JSON dokumenta oz. sporočila
    // sporočilo bo shranjeno v spremenljivki doc
    DeserializationError error = deserializeJson(doc, payload);
    // Ali je razčlenjevanje uspešno?
    if(error){
      Serial.print(F("deserializeJson() - napaka"));
      Serial.println(error.c_str());
      return;
    }

    char tip[40]; // spremenljivka za besedo iz JSON strukture
    memset(tip, 0, 40); // izbrišemo vse vrednosti iz znakovnega niza tip
    strcpy(tip, doc["tipSporočila"]); // iz JSON strukture v spremenljivko tip kopiramo vrednost pod ključem "tipSporočila" | v našem primeru je tip="LED"

    if(strcmp(tip, "LED") == 0){ //če je tip sporočila "LED"
      int vrednost = doc["vrednost"]; // v spremenljivko vrednost zapišemo vrednost iz JSON strukture pod ključem "vrednost | v našem primeru je le-ta enaka1"
      int pin = doc["pin"]; // pin je enak 2
      if(pin == 1){
        if(vrednost == 0){
          digitalWrite(2, 0); // na pin 2 zapišemo 0 | na pin 2 je vgrajena LED dioda
        }
        else if(vrednost == 1){
          digitalWrite(2, 1); // na pin 2 zapišemo 1
        }
      }
      if(pin == 2){
        if(vrednost == 0){
          digitalWrite(5, 0); // na pin 2 zapišemo 0 | na pin 2 je vgrajena LED dioda
        }
        else if(vrednost == 1){
          digitalWrite(5, 1); // na pin 2 zapišemo 1
        }
      }
      if(pin == 12){
        if(vrednost == 0){
          digitalWrite(2, 0); // na pin 2 zapišemo 0 | na pin 2 je vgrajena LED dioda
          digitalWrite(5, 0); // na pin 2 zapišemo 0 | na pin 2 je vgrajena LED dioda
        }
        else if(vrednost == 1){
          digitalWrite(2, 1); // na pin 2 zapišemo 1
          digitalWrite(5, 1); // na pin 2 zapišemo 1
        }
      }
      if(pin == 1111){
        if(vrednost == 0){
          digitalWrite(2, 0); // na pin 2 zapišemo 0 | na pin 2 je vgrajena LED dioda
          digitalWrite(5, 0); // na pin 2 zapišemo 0 | na pin 2 je vgrajena LED dioda
        }
        else if(vrednost == 1){
          digitalWrite(2, 1); // na pin 2 zapišemo 1
          digitalWrite(5, 1); // na pin 2 zapišemo 1
        }
      }
      else // default:
      {
        Serial.println("Neprepoznana vrednost v JSON sporočili");
      }
      break;
    }; // konec if stavka, če je tip sporočila "LED"
  }
  default:
    break;
  } // konec switch stavka (po C++ standardu moramo ibvezno imeti stavek default break;)
}; //konec onWebSocketEvent


void setup() {
  pinMode(2, OUTPUT); // tu določimo, da je pin 2 izhod (vgrajena LED dioda)
  pinMode(5, OUTPUT);

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
  Serial.print("Številka port-a: ");
  Serial.println(portNumber);
  Serial.print("WiFi.macAddress: ");
  Serial.println(WiFi.macAddress());


  server.on("/", handle_root); // ko vpišemo IP naslov v brskalnik, pokličemo funkcijo handle_root, ki vrne kratko sporočilo klientu
  server.onNotFound(handle_NotFound); // v primeru, da spletne strani ni na strežniku izpišemo obvestilo, da strani ne najdemo
  server.begin(); // zagon HTTP strežnika
  Serial.print("HTTP strežnik je zagnan, vpišite IP naslov v brskalnik");

  // poženemo WebSocket strežnik in določimo "callback"
  webSocket.begin(); // zagon WebSocket strežnika
  webSocket.onEvent(onWebSocketEvent); // kadarkoli se bo dogodil določen dogodek preko spletnega vtičnika
                                       // WebSocket bomo izvedli funkcijo onWebSocketEvent, ki je definirana zgoraj

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

void loop() {
  // put your main code here, to run repeatedly:
  // tu zapišemo kodo, ki se ponavljajoče izvaja:
  server.handleClient(); // spremlja prisotnost klientov in posluša njihove HTML zahteve ("request")

  webSocket.loop(); // poslušamo na spletnem vtičniku - WebSocket za podatke in dogodke

}