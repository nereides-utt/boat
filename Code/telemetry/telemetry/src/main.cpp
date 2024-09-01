#include <WiFi.h>
#include "PubSubClient.h"
//GPS
#include <TinyGPSPlus.h>
#include <HardwareSerial.h>

//carte SD
#include "FS.h"
#include "SD.h"
#include "SPI.h"
#include <OneWire.h>

//FTP
#include <SimpleFTPServer.h>
FtpServer ftpSrv;

//librairie créée
#include "DataLayout.h"
#include "carteSDFunction.h"

//Communication Série vers le teensy
//Port série 2 de l'ESP32
#define TxTeensy 21
#define RxTeensy 22
HardwareSerial serialToTeensy(1);

//GPS
#define TxGps 17
#define RxGps 16
HardwareSerial SerialGPS(2);
TinyGPSPlus gps;

//Température
OneWire ds(13);

//WIFI
//const char* ssid = "Tenda_0B9BB8";
//const char* password = "back5754";
const char* ssid = "chiv";
const char* password = "12345679";

WiFiClient espClient;
PubSubClient client(espClient);

// Configuration du serveur MQTT
const char* mqtt_server = "test.mosquitto.org";
const int mqtt_port = 1883;

String nomFichier="test";
String nomFichierLog="testetes";
String setupLog = "";
String recu;
Data_layout extractedData;
int compteurBoucle = 0;
int32_t day;
int32_t month;
int32_t year;

//Définition des fonctions:
String getSubPart(String data, String prefix);
void parseDataString(String data);
void setup_wifi();
void reconnect();
bool sendMQTT(const char* topic, const char* msg);

//Carte SD :
void teteCsv();
void writeCsv();

void _callback(FtpOperation ftpOperation, unsigned int freeSpace, unsigned int totalSpace) {
  Serial.print(">>>>>>>>>>>>>>> _callback ");
  Serial.print(ftpOperation);
  Serial.print(" ");
  Serial.print(freeSpace);
  Serial.print(" ");
  Serial.println(totalSpace);

  if (ftpOperation == FTP_CONNECT) Serial.println(F("CONNECTED"));
  if (ftpOperation == FTP_DISCONNECT) Serial.println(F("DISCONNECTED"));
};

void _transferCallback(FtpTransferOperation ftpOperation, const char* name, unsigned int transferredSize) {
  Serial.print(">>>>>>>>>>>>>>> _transferCallback ");
  Serial.print(ftpOperation);
  Serial.print(" ");
  Serial.print(name);
  Serial.print(" ");
  Serial.println(transferredSize);
};

float debug;
float voltage;
void setup() {
  pinMode(27, INPUT);
  debug = analogRead(27);
  Serial.println(debug);
  voltage = debug * (5 / 4095.0);

  if (voltage > 3) {
    setup_wifi();
    delay(500);
    if (SD.begin(5, SPI, 4000000)) {
      Serial.println("SD opened!");

      ftpSrv.setCallback(_callback);
      ftpSrv.setTransferCallback(_transferCallback);

      ftpSrv.begin("esp32", "esp32");  // username, password for ftp. (default 21, 50009 for PASV)
      Serial.println("FTP server started on port 21");
      listDir(SD, "/", 0);

    } else {
      writeFile(SD, "chien", "devie");
      Serial.println("SD Card Mount Failed");
    }
  } else {
    Serial.begin(115200);
    compteurBoucle = 0;
    serialToTeensy.begin(115200, SERIAL_8N1,  RxTeensy, TxTeensy);
    SerialGPS.begin(9600, SERIAL_8N1, RxGps, TxGps);
    setup_wifi();
    client.setServer(mqtt_server, mqtt_port);
    delay(500);
    while (!SD.begin() && compteurBoucle < 5) {
      setupLog += "Card Mount Failed\n";
      delay(1000);
      compteurBoucle++;
    }
    compteurBoucle = 0;
    uint8_t cardType = SD.cardType();
    if (cardType == CARD_NONE) {
      setupLog += "No SD card attached\n";
      return;
    }
    nomFichierLog ="hey.csv";
    nomFichier="testgps.txt";
    /*
    int32_t day = gps.date.day();
    int32_t month = gps.date.month();
    int32_t year = gps.date.year();
    nomFichierLog = "/log_" + String(year) + "_" + String(month) + "_" + String(day) + "_" + String(gps.time.hour() + 2) + "_" + String(gps.time.minute()) + "_" + String(gps.time.second()) + ".txt";
    nomFichier = "/nereideAnalytics_" + String(year) + "_" + String(month) + "_" + String(day) + "_" + String(gps.time.hour() + 2) + "_" + String(gps.time.minute()) + "_" + String(gps.time.second()) + ".csv";*/
    teteCsv();
    writeFile(SD, nomFichierLog.c_str(), setupLog.c_str());
    //0 R1 R0 1 1 1 1 1
    //00 : 9 bits - 01 : 10 bits - 10 : 11 bits - 11 12 bits
    //x1F - x3F - x5F - x8F
    //94 ms - 188ms - 375 ms - 750 ms
    configureSensorResolution(0x1F);
  }
}

int i = 51;
unsigned long lastHardwareSerialTime = 0;
unsigned long lastSendData = 0;
unsigned long lastSendVitesse = 0;
unsigned long currentTime = 0;

void loop() {
  if (voltage > 3) {
    ftpSrv.handleFTP();
  } else {
    while (!client.connected()) {
      reconnect();
    }

    /*while (SerialGPS.available() > 0) {
    char c = SerialGPS.read(); 
    if (gps.encode(c)) {
      if (gps.location.isValid()) {
        extractedData.gps_vitesse = gps.speed.kmph();
        extractedData.gps_latitude = gps.location.lat();
        extractedData.gps_longitude = gps.location.lng();
      } else {
        Serial.println("Localisation GPS non valide");
      }
    }
  }*/

    //Température :
    byte data[12];  // Tableau de données pour stocker les données du capteur
    byte addr[8];   // Tableau d'adresses pour stocker l'adresse unique du capteur
    String logTemp = "";
    if (!ds.search(addr)) {
      logTemp += "Pas de capteur trouvé";
      ds.reset_search();
      delay(250);
    }
    if (OneWire::crc8(addr, 7) != addr[7]) {
      logTemp += "CRC incorrect";
    }
    ds.reset();
    ds.select(addr);
    // Lancez une conversion de température
    ds.write(0x44, 1);  
    //93,75 si 9 bits / 187.5 si 10 bits / 375 si 11 / 750 si 12
    delay(200);        
    // Attendez la fin de la conversion
    byte present = ds.reset();
    ds.select(addr);
    ds.write(0xBE);                // Demandez les données de température
    for (int i = 0; i < 9; i++) {  // Boucle pour lire les données
      data[i] = ds.read();
    }
    ds.reset_search();
    byte MSB = data[1];
    byte LSB = data[0];
    float tempRead = ((MSB << 8) | LSB);  // Concaténez les octets pour obtenir la température
    float TemperatureSum = tempRead / 16;
    logTemp += "Température : ";
    logTemp += String(TemperatureSum);
    logTemp += "°C";
    extractedData.batterySE_temp = String(TemperatureSum);
    appendFile(SD, nomFichierLog.c_str(), logTemp.c_str());

    currentTime = millis();
    if (currentTime - lastHardwareSerialTime >= 1000) {
      while (serialToTeensy.available()) {
        //voir comment faire pour qu'il le prenne qu'une fois ou pas trop longtemps (je pense faire un truc genre tout les 0-500ms temps je suis ici 500-1000 je fais MQTT 1000-2000 : ex robin houd )
        recu = serialToTeensy.readStringUntil(';');
        parseDataString(recu);
        String log1 = recu;
        log1 = "\n";
        appendFile(SD, nomFichierLog.c_str(), log1.c_str());
        enregistrerLogCsv();
      }
      lastHardwareSerialTime = currentTime;
    }

    client.loop();

    if (currentTime - lastSendData >= 1000) {
      //attention a ne pas envoyer si on reçoit plus d'info du teensy!
      //remplirData();
      bool a = sendData();
      writeCsv();
      lastSendData = currentTime;
      debugData();
    }

    if (currentTime - lastSendVitesse >= 1000) {
      String canMsg = extractedData.gps_vitesse + ";";
      serialToTeensy.print(canMsg);
      lastSendVitesse = currentTime;
    }
  }
}

//-------------------------------------------------------------------------------------------------------------------------------------------------------
//------------------------------------------------- Traitement de la chaine de caractère envoyer du teensy à l'esp32 -----------------------------------
// Fonction pour obtenir une sous-partie de la chaîne
String getSubPart(String data, String prefix) {
  int startIndex = data.indexOf(prefix);
  if (startIndex == -1) return "";
  startIndex += prefix.length();
  int endIndex = data.indexOf("/", startIndex);
  if (endIndex == -1) endIndex = data.length();
  return data.substring(startIndex, endIndex);
}

void remplirData(){
  extractedData.motor_current_a = 666;
  extractedData.motor_voltage_v = 666;
  extractedData.motor_rpm = 666;
  extractedData.motor_throttle = 666;
  extractedData.motor_temp = 666;
  extractedData.motor_controller_temp = 666;
  extractedData.motor_error_code = 666;
  extractedData.motor_controller_status = 666;
  extractedData.motor_switch_signals_status = 666;

  // Battery data
  extractedData.battery_voltage_v = 666;
  extractedData.battery_current_a = 666;
  extractedData.battery_soc = 666;
  extractedData.battery_soh = 666;

  // Fuel Cell data
  extractedData.pac_emergency_stop = 666;
  extractedData.pac_start = 666;
  extractedData.pac_stop = 666;
  extractedData.pac_current_a = 666;
  extractedData.pac_voltage_v = 666;
  extractedData.pac_system_state = 666;
  extractedData.pac_error_flag = 666;
  extractedData.pac_hydrogen_consumption_mgs = 666;
  extractedData.pac_temperature_c = 666;
  extractedData.pac_system_errors = 666;
  extractedData.pac_fan_error = 666;
  extractedData.pac_operation_time = 666;
  extractedData.pac_produced_energy = 666;
  extractedData.pac_total_operation_time = 666;
  extractedData.pac_total_produced_energy = 666;

}

void parseDataString(String data) {
  // Motor data
  extractedData.motor_current_a = getSubPart(data, "a/");
  extractedData.motor_voltage_v = getSubPart(data, "v/");
  extractedData.motor_rpm = getSubPart(data, "r/");
  extractedData.motor_throttle = getSubPart(data, "t/");
  extractedData.motor_temp = getSubPart(data, "temp/");
  extractedData.motor_controller_temp = getSubPart(data, "ctemp/");
  extractedData.motor_error_code = getSubPart(data, "err/");
  extractedData.motor_controller_status = getSubPart(data, "stat/");
  extractedData.motor_switch_signals_status = getSubPart(data, "sig/");

  // Battery data
  extractedData.battery_voltage_v = getSubPart(data, "bv/");
  extractedData.battery_current_a = getSubPart(data, "ba/");
  extractedData.battery_soc = getSubPart(data, "bsoc/");
  extractedData.battery_soh = getSubPart(data, "bsoh/");

  // Fuel Cell data
  extractedData.pac_emergency_stop = getSubPart(data, "estop/");
  extractedData.pac_start = getSubPart(data, "pstart/");
  extractedData.pac_stop = getSubPart(data, "pstop/");
  extractedData.pac_current_a = getSubPart(data, "pc/");
  extractedData.pac_voltage_v = getSubPart(data, "pv/");
  extractedData.pac_system_state = getSubPart(data, "pstate/");
  extractedData.pac_error_flag = getSubPart(data, "perr/");
  extractedData.pac_hydrogen_consumption_mgs = getSubPart(data, "ph2/");
  extractedData.pac_temperature_c = getSubPart(data, "ptemp/");
  extractedData.pac_system_errors = getSubPart(data, "psyserr/");
  extractedData.pac_fan_error = getSubPart(data, "pfanerr/");
  extractedData.pac_operation_time = getSubPart(data, "ptime/");
  extractedData.pac_produced_energy = getSubPart(data, "pprod/");
  extractedData.pac_total_operation_time = getSubPart(data, "ptottime/");
  extractedData.pac_total_produced_energy = getSubPart(data, "ptotprod/");
}


// Fonction de connexion au réseau WiFi
void setup_wifi() {
  delay(10);
  setupLog = "Connecté à : ";
  setupLog += ssid;
  setupLog += "\n";

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    setupLog += ".";
  }

  setupLog += "WiFi connecté\n";
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------
//--------------------------------------------------------- MQTT ----------------------------------------------------------------------------------------------------
void reconnect() {
  while (!client.connected()) {
    String log = "Connexion au serveur MQTT...\n";
    appendFile(SD, nomFichierLog.c_str(), log.c_str());
    if (client.connect("ESPChivasNereides")) {
      String log = "connecté\n";
      appendFile(SD, nomFichierLog.c_str(), log.c_str());
    } else {
      String log = "échec, rc=";
      log += client.state();
      log += "\n";
      log += " nouvelle tentative dans 5 secondes\n";
      appendFile(SD, nomFichierLog.c_str(), log.c_str());
      delay(5000);
    }
  }
}

bool sendMQTT(const char* topic, const char* msg) {
  if (client.publish(topic, msg)) {
    String log = "\nPublication du message\n";
    appendFile(SD, nomFichierLog.c_str(), log.c_str());
    return true;
  } else {
    String log = "Echec de la publication du message\n";
    appendFile(SD, nomFichierLog.c_str(), log.c_str());
    return false;
  }
}

bool sendData() {
  // Send GPS data
  sendMQTT(gps_millis, extractedData.gps_millis.c_str());
  sendMQTT(gps_time, extractedData.gps_time.c_str());
  sendMQTT(gps_latitude, extractedData.gps_latitude.c_str());
  sendMQTT(gps_longitude, extractedData.gps_longitude.c_str());
  sendMQTT(gps_vitesse, extractedData.gps_vitesse.c_str());


  // Send Motor data
  sendMQTT(motor_current_a, extractedData.motor_current_a.c_str());
  sendMQTT(motor_voltage_v, extractedData.motor_voltage_v.c_str());
  sendMQTT(motor_rpm, extractedData.motor_rpm.c_str());
  sendMQTT(motor_throttle, extractedData.motor_throttle.c_str());
  sendMQTT(motor_temp, extractedData.motor_temp.c_str());
  sendMQTT(motor_controller_temp, extractedData.motor_controller_temp.c_str());
  sendMQTT(motor_error_code, extractedData.motor_error_code.c_str());
  sendMQTT(motor_controller_status, extractedData.motor_controller_status.c_str());
  sendMQTT(motor_switch_signals_status, extractedData.motor_switch_signals_status.c_str());

  // Send Battery data
  sendMQTT(battery_voltage_v, extractedData.battery_voltage_v.c_str());
  sendMQTT(battery_current_a, extractedData.battery_current_a.c_str());
  sendMQTT(battery_soc, extractedData.battery_soc.c_str());
  sendMQTT(battery_soh, extractedData.battery_soh.c_str());

  // Send Fuel Cell data
  sendMQTT(pac_emergency_stop, extractedData.pac_emergency_stop.c_str());
  sendMQTT(pac_start, extractedData.pac_start.c_str());
  sendMQTT(pac_stop, extractedData.pac_stop.c_str());
  sendMQTT(pac_current_a, extractedData.pac_current_a.c_str());
  sendMQTT(pac_voltage_v, extractedData.pac_voltage_v.c_str());
  sendMQTT(pac_system_state, extractedData.pac_system_state.c_str());
  sendMQTT(pac_error_flag, extractedData.pac_error_flag.c_str());
  sendMQTT(pac_hydrogen_consumption_mgs, extractedData.pac_hydrogen_consumption_mgs.c_str());
  sendMQTT(pac_temperature_c, extractedData.pac_temperature_c.c_str());
  sendMQTT(pac_system_errors, extractedData.pac_system_errors.c_str());
  sendMQTT(pac_fan_error, extractedData.pac_fan_error.c_str());
  sendMQTT(pac_operation_time, extractedData.pac_operation_time.c_str());
  sendMQTT(pac_produced_energy, extractedData.pac_produced_energy.c_str());
  sendMQTT(pac_total_operation_time, extractedData.pac_total_operation_time.c_str());
  sendMQTT(pac_total_produced_energy, extractedData.pac_total_produced_energy.c_str());
  sendMQTT(batterySE_temp, extractedData.batterySE_temp.c_str());

  return true;
}

// --------------------- TEMPERATURE -----------------------
void configureSensorResolution(byte resolution) {
  byte addr[8];
  if (!ds.search(addr)) {
    Serial.println("Pas de capteur trouvé pour configurer la résolution");
    ds.reset_search();
    delay(250);
    return;
  }
  if (OneWire::crc8(addr, 7) != addr[7]) {
    Serial.println("CRC incorrect lors de la configuration de la résolution");
    return;
  }
  ds.reset();
  ds.select(addr);
  ds.write(0x4E);        // Commande d'écriture au registre de configuration
  ds.write(0x00);        // TH register (non utilisé ici)
  ds.write(0x00);        // TL register (non utilisé ici)
  ds.write(resolution);  // Écrire la résolution
  ds.reset();
}


//------------------------------------------------------------------------------------------------------------------------------------------------------------------
//--------------------------------------------------------- DEBUG DATA  ----------------------------------------------------------------------------------------------------

void debugData() {
  String log = "Millis time: " + extractedData.gps_millis + " - "
                                                            "GPS Time: "
               + extractedData.gps_time + " - " + "Latitude : " + extractedData.gps_latitude + " - " + "Longitude: " + extractedData.gps_longitude + " - " + "Vitesse: " + extractedData.gps_vitesse + " - " + "Motor Current: " + extractedData.motor_current_a + " - " + "Motor Voltage: " + extractedData.motor_voltage_v + " - " + "Motor RPM: " + extractedData.motor_rpm + " - " + "Motor Throttle: " + extractedData.motor_throttle + " - " + "Motor Temp: " + extractedData.motor_temp + " - " + "Motor Controller Temp: " + extractedData.motor_controller_temp + " - " + "Motor Error Code: " + extractedData.motor_error_code + " - " + "Motor Controller Status: " + extractedData.motor_controller_status + " - " + "Motor Switch Signals Status: " + extractedData.motor_switch_signals_status + " - " + "Battery Voltage: " + extractedData.battery_voltage_v + " - " + "Battery Current: " + extractedData.battery_current_a + " - " + "Battery SoC: " + extractedData.battery_soc + " - " + "Battery SoH: " + extractedData.battery_soh + " - " + "PAC Emergency Stop: " + extractedData.pac_emergency_stop + " - " + "PAC Start: " + extractedData.pac_start + " - " + "PAC Stop: " + extractedData.pac_stop + " - " + "PAC Current: " + extractedData.pac_current_a + " - " + "PAC Voltage: " + extractedData.pac_voltage_v + " - " + "PAC System State: " + extractedData.pac_system_state + " - " + "PAC Error Flag: " + extractedData.pac_error_flag + " - " + "PAC Hydrogen Consumption: " + extractedData.pac_hydrogen_consumption_mgs + " - " + "PAC Temperature: " + extractedData.pac_temperature_c + " - " + "PAC System Errors: " + extractedData.pac_system_errors + " - " + "PAC Fan Error: " + extractedData.pac_fan_error + " - " + "PAC Operation Time: " + extractedData.pac_operation_time + " - " + "PAC Produced Energy: " + extractedData.pac_produced_energy + " - " + "PAC Total Operation Time: " + extractedData.pac_total_operation_time + " - " + "PAC Total Produced Energy: " + extractedData.pac_total_produced_energy + " - " + "Temperature Battery SE: " + extractedData.batterySE_temp;

  appendFile(SD, nomFichierLog.c_str(), log.c_str());
}

//-------------------------------------------------------------------------
// ------------------------------------------------------------------------
//-------------------------- FONCTION CARTE SD ----------------------------
void enregistrerLogCsv() {
  String log = "Time millis: ";
  log += millis();
  log += " - Latitude: ";
  log += extractedData.gps_latitude;
  log += " - Longitude: ";
  log += extractedData.gps_longitude;
  log += " - GPS Time: ";
  log += extractedData.gps_time;
  log += " - Motor Current: ";
  log += extractedData.motor_current_a;
  log += " - Motor Voltage: ";
  log += extractedData.motor_voltage_v;
  log += " - Motor RPM: ";
  log += extractedData.motor_rpm;
  log += " - Motor Throttle: ";
  log += extractedData.motor_throttle;
  log += " - Motor Temp: ";
  log += extractedData.motor_temp;
  log += " - Motor Controller Temp: ";
  log += extractedData.motor_controller_temp;
  log += " - Motor Error Code: ";
  log += extractedData.motor_error_code;
  log += " - Motor Controller Status: ";
  log += extractedData.motor_controller_status;
  log += " - Motor Switch Signals Status: ";
  log += extractedData.motor_switch_signals_status;
  log += " - Battery Voltage: ";
  log += extractedData.battery_voltage_v;
  log += " - Battery Current: ";
  log += extractedData.battery_current_a;
  log += " - Battery SoC: ";
  log += extractedData.battery_soc;
  log += " - Battery SoH: ";
  log += extractedData.battery_soh;
  log += " - PAC Emergency Stop: ";
  log += extractedData.pac_emergency_stop;
  log += " - PAC Start: ";
  log += extractedData.pac_start;
  log += " - PAC Stop: ";
  log += extractedData.pac_stop;
  log += " - PAC Current: ";
  log += extractedData.pac_current_a;
  log += " - PAC Voltage: ";
  log += extractedData.pac_voltage_v;
  log += " - PAC System State: ";
  log += extractedData.pac_system_state;
  log += " - PAC Error Flag: ";
  log += extractedData.pac_error_flag;
  log += " - PAC Hydrogen Consumption: ";
  log += extractedData.pac_hydrogen_consumption_mgs;
  log += " - PAC Temperature: ";
  log += extractedData.pac_temperature_c;
  log += " - PAC System Errors: ";
  log += extractedData.pac_system_errors;
  log += " - PAC Fan Error: ";
  log += extractedData.pac_fan_error;
  log += " - PAC Operation Time: ";
  log += extractedData.pac_operation_time;
  log += " - PAC Produced Energy: ";
  log += extractedData.pac_produced_energy;
  log += " - PAC Total Operation Time: ";
  log += extractedData.pac_total_operation_time;
  log += " - PAC Total Produced Energy: ";
  log += extractedData.pac_total_produced_energy;
  log += " - Temperature Battery SE: ";
  log += extractedData.batterySE_temp;

  appendFile(SD, nomFichierLog.c_str(), log.c_str());
}


//A ameliorer tableau avec pas de possibilité d'avoir des données différentes/nombre différent colonne
//For pour le nombre dans le tableau
void teteCsv() {
  //faire en sorte que le nom du fichier soit le nom+date+seconde comme ça on remplace pas les données à chaque fois mais on réécris dans un autre fichier
  String tete = nomMesure[0];
  for (int i = 1; i < nbrMesures; i++) {
    tete += ",";
    tete += nomMesure[i];
  }
  tete += "\n";
  //attention si jamais le gps n'est pas branché que se passe t'il?


  writeFile(SD, nomFichier.c_str(), tete.c_str());
}

void writeCsv() {
  String data =
    extractedData.gps_millis + "," + extractedData.gps_time + "," + extractedData.gps_latitude + "," + extractedData.gps_longitude + "," + extractedData.motor_current_a + "," + extractedData.motor_voltage_v + "," + extractedData.motor_rpm + "," + extractedData.motor_throttle + "," + extractedData.motor_temp + "," + extractedData.motor_controller_temp + "," + extractedData.motor_error_code + "," + extractedData.motor_controller_status + "," + extractedData.motor_switch_signals_status + "," + extractedData.battery_voltage_v + "," + extractedData.battery_current_a + "," + extractedData.battery_soc + "," + extractedData.battery_soh + "," + extractedData.pac_emergency_stop + "," + extractedData.pac_start + "," + extractedData.pac_stop + "," + extractedData.pac_current_a + "," + extractedData.pac_voltage_v + "," + extractedData.pac_system_state + "," + extractedData.pac_error_flag + "," + extractedData.pac_hydrogen_consumption_mgs + "," + extractedData.pac_temperature_c + "," + extractedData.pac_system_errors + "," + extractedData.pac_fan_error + "," + extractedData.pac_operation_time + "," + extractedData.pac_produced_energy + "," + extractedData.pac_total_operation_time + "," + extractedData.pac_total_produced_energy + ",";
  extractedData.batterySE_temp + "\n";

  appendFile(SD, nomFichier.c_str(), data.c_str());
}
//askip plus opti :
/*char data[500]; // Ajustez la taille selon vos besoins pour accueillir toutes les données

    snprintf(data, sizeof(data),
             "%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s\n",
             extractedData.latitude.c_str(),
             extractedData.longitude.c_str(),..
             
    appendFile(SD, nomFichier.c_str(), data);            */
