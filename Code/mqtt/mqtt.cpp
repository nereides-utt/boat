#include <Arduino.h>

//permet de commmuniquer avec la SIM
HardwareSerial mySerial2(2);

#define DEBUG true

#define IO_RXD2 47
#define IO_TXD2 48

const int PIN_SD_SELECT = 10;


#define IO_GSM_PWRKEY 4
#define IO_GSM_RST 5

unsigned long currentTime;

//Permet d'envoyer (mySerial2.println(command)) et de recevoir des infos selons le temps découte et les afficher si y'a un DEBUG!
String sendData(String command, const int timeout, boolean debug) {
  String response = "";
  mySerial2.println(command);
  long int time = millis();
  while ((time + timeout) > millis()) {
    while (mySerial2.available()) {
      char c = mySerial2.read();
      response += c;
    }
  }
  if (debug) {
    USBSerial.print(response);
  }
  return response;
}

void setup() {
  USBSerial.begin(115200);
  USBSerial.print(F("Hello! ESP32-S3 AT command V1.0 Test"));
  mySerial2.begin(115200, SERIAL_8N1, IO_RXD2, IO_TXD2);
  pinMode(IO_GSM_RST, OUTPUT);
  digitalWrite(IO_GSM_RST, LOW);

  pinMode(IO_GSM_PWRKEY, OUTPUT);
  digitalWrite(IO_GSM_PWRKEY, HIGH);
  delay(3000);
  digitalWrite(IO_GSM_PWRKEY, LOW);

  USBSerial.println("ESP32-S3 4G LTE CAT1 Test Start!");
  delay(2000);
  USBSerial.println("Wait a few minutes for 4G star");
  delay(3000);

  seConnecterSIM("9735");

  setupMQTT();

  currentTime = millis();
}

void loop() {

  if (millis() - currentTime > 50) {
    for (int i = 0; i < 10; i++) {
      sendMQTT("nereides/bateau/vitesse","23",String(i*10),"3");
      sendMQTT("nereides/bateau/position/lat","28",String(i*2),"3");
      sendMQTT("nereides/bateau/position/lng","28",String(i*100),"3");
      sendMQTT("nereides/batterie1/temp","23",String(i*5),"3");
      sendMQTT("nereides/batterie1/soc","22",String(i*17),"3");
      sendMQTT("nereides/batterie1/sante","24",String(i*32),"3");
      sendMQTT("nereides/batterie1/voltage","26",String(i*0),"3");
      sendMQTT("nereides/batterie1/current","26",String(i*55),"3");
    }
  }
  while (USBSerial.available() > 0) {
    mySerial2.write(USBSerial.read());
    yield();
  }
  while (mySerial2.available() > 0) {
    USBSerial.write(mySerial2.read());
    yield();
  }
}

//Envoyer en MQTT sur le TOPIC un message (doit être des chiffres) : on pourraiut aussi ajouter des alertes en fonctions des réponses reçus
void sendMQTT(String topic,String lenTopic,String msg,String lenMsg) {
  String cmdTopic="AT+CMQTTTOPIC=0,"+lenTopic;
  String cmdPayload = "AT+CMQTTPAYLOAD=0,"+lenMsg;
  sendData(cmdTopic, 50, DEBUG);
  sendData(topic, 50, DEBUG);
  sendData(cmdPayload, 50, DEBUG);
  sendData(msg, 50, DEBUG);
  sendData("AT+CMQTTPUB=0,1,60", 50, DEBUG);
}

//Configure le mqtt : on pourrait faire des alertes si jamais on reçois pas les bons messages
void setupMQTT() {
  sendData("AT+CMQTTSTART", 1000, DEBUG);
  String retour = sendData("AT+CMQTTACCQ=0,\"client test0\"", 1000, DEBUG);
  while(retour=="ERROR"){
    retour = sendData("AT+CMQTTACCQ=0,\"client test0\"", 1000, DEBUG);
    delay(1000);
  }
  sendData("AT+CMQTTCONNECT=0,\"tcp://test.mosquitto.org:1883\",60,1", 1000, DEBUG);
}


//faire des fonctions pour alleger (mettre des retours pour vérifier si ça a bien fonctionné)
void seConnecterSIM(String codePin) {
  String command = "AT+CPIN="+codePin;
  sendData(command, 1000, DEBUG);
  delay(1000);
  sendData("AT+CICCID", 1000, DEBUG);
  delay(1000);
}

void envoyerMQTT(String msg) {
}