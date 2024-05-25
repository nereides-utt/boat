#include <WiFi.h>
#include "PubSubClient.h"
#include "Wire.h"

//WIFI
//const char* ssid = "Tenda_0B9BB8";
//const char* password = "back5754";
const char* ssid = "chiv";
const char* password = "12345679";


// Configuration du serveur MQTT
const char* mqtt_server = "test.mosquitto.org";
const int mqtt_port = 1883;

WiFiClient espClient;
PubSubClient client(espClient);

// Fonction de connexion au réseau WiFi
void setup_wifi() {
  delay(10);
  Serial.print("Connexion à ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connecté");
  Serial.println("Adresse IP : ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Connexion au serveur MQTT...");
    if (client.connect("ESPChivasNereides")) {
      Serial.println("connecté");
    } else {
      Serial.print("échec, rc=");
      Serial.print(client.state());
      Serial.println(" nouvelle tentative dans 5 secondes");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  const char* topic = "nereides/bateau/vitesse";
  
  for(int i=0;i<10;i++){
    String message = "51";
    message+=String(i);
    sendMQTT(topic, message.c_str());
    delay(1000);
  }
  
  delay(2500);
}

bool sendMQTT(const char* topic, const char* msg) {
  if (client.publish(topic, msg)) {
    Serial.println("Message publié avec succès");
    return true;
  } else {
    Serial.println("Echec de la publication du message");
    return false;
  }
}
