#include <Arduino.h>

//pour ajouter des scripts sur la bibliothque consulter : https://espressif-docs.readthedocs-hosted.com/projects/arduino-esp32/en/latest/api/sdmmc.html
//ou directement : https://github.com/Makerfabs/ESP32-S3-4G-LTE-CAT1-A7670X/blob/main/example/ESP32_S3_4G_SD_Card/ESP32_S3_4G_SD_Card.ino

#define DEBUG true

#include "FS.h"
#include <SD_MMC.h>
#include "SPI.h"

#define IO_RXD2 47
#define IO_TXD2 48


#define PIN_SD_CMD 11
#define PIN_SD_CLK 12
#define PIN_SD_D0 13

#define IO_GSM_PWRKEY 4
#define IO_GSM_RST 5

unsigned long currentTime;

void writeFile(fs::FS &fs, const char *path, const char *message) {
  Serial.printf("Writing file: %s\n", path);

  File file = fs.open(path, FILE_WRITE);
  if (!file) {
    Serial.println("Failed to open file for writing");
    return;
  }
  if (file.print(message)) {
    Serial.println("File written");
  } else {
    Serial.println("Write failed");
  }
}

void appendFile(fs::FS &fs, const char * path, const char * message){
    Serial.printf("Appending to file: %s\n", path);

    File file = fs.open(path, FILE_APPEND);
    if(!file){
        Serial.println("Failed to open file for appending");
        return;
    }
    if(file.print(message)){
        Serial.println("Message appended");
    } else {
        Serial.println("Append failed");
    }
}
struct Batterie{
    String courant;
    String voltage;
    String temperature;
    String soc;
    String santee;
  };
struct Donnee {
    String vitesse;
    String lat;
    String lng;
    Batterie bat1; 
  };
  struct Donnee donnee;
  struct Batterie bat;
  

void setup() {
  USBSerial.begin(115200);
  delay(5000);
  SD_MMC.setPins(PIN_SD_CLK, PIN_SD_CMD, PIN_SD_D0);
  if (!SD_MMC.begin("/sdcard", true, true)) {
    USBSerial.println("Card Mount Failed");
    return;
  } else {
    USBSerial.println("Card Mount Succeded");
  }
  uint8_t cardType = SD_MMC.cardType();

  if (cardType == CARD_NONE) {
    USBSerial.println("No SD_MMC card attached");
    return;
  }

  USBSerial.print("SD_MMC Card Type: ");
  if (cardType == CARD_MMC) {
    USBSerial.println("MMC");
  } else if (cardType == CARD_SD) {
    USBSerial.println("SDSC");
  } else if (cardType == CARD_SDHC) {
    USBSerial.println("SDHC");
  } else {
    USBSerial.println("UNKNOWN");
  }

  uint64_t cardSize = SD_MMC.cardSize() / (1024 * 1024);
  USBSerial.printf("SD_MMC Card Size: %lluMB\n", cardSize);

  //Write va directement remplacer le fichier / append va ajouter !
  writeFile(SD_MMC, "/hello.txt", "Hello ");
  appendFile(SD_MMC, "/hello.txt", "Wow!!");
  appendFile(SD_MMC, "/hello.txt", "World!\n");
  
  modifierDonnee("100","36.3432","23.2324","23","32","100","100","99");
  sauvegarderDonnee("bellecouille");
}

void loop() {
}

void sauvegarderDonnee(String nomFichier){
  String nomCsv = "/" + nomFichier +".csv";
  writeFile(SD_MMC,nomCsv.c_str(),"Vitesse,latitude,longitude,courantBat1,voltageBat1,temperatureBat1,socBat1,santeeBat1\n");
  appendFile(SD_MMC,nomCsv.c_str(),donnee.vitesse.c_str());
  appendFile(SD_MMC,nomCsv.c_str(),",");
  appendFile(SD_MMC,nomCsv.c_str(),donnee.lat.c_str());
  appendFile(SD_MMC,nomCsv.c_str(),",");
  appendFile(SD_MMC,nomCsv.c_str(),donnee.lng.c_str());
  appendFile(SD_MMC,nomCsv.c_str(),",");
  appendFile(SD_MMC,nomCsv.c_str(),donnee.bat1.courant.c_str());
  appendFile(SD_MMC,nomCsv.c_str(),",");
  appendFile(SD_MMC,nomCsv.c_str(),donnee.bat1.voltage.c_str());
  appendFile(SD_MMC,nomCsv.c_str(),",");
  appendFile(SD_MMC,nomCsv.c_str(),donnee.bat1.temperature.c_str());
  appendFile(SD_MMC,nomCsv.c_str(),",");
  appendFile(SD_MMC,nomCsv.c_str(),donnee.bat1.soc.c_str());
  appendFile(SD_MMC,nomCsv.c_str(),",");
  appendFile(SD_MMC,nomCsv.c_str(),donnee.bat1.santee.c_str());
  appendFile(SD_MMC,nomCsv.c_str(),"\n");
  appendFile(SD_MMC,nomCsv.c_str(),donnee.vitesse.c_str());
  appendFile(SD_MMC,nomCsv.c_str(),",");
  appendFile(SD_MMC,nomCsv.c_str(),donnee.lat.c_str());
  appendFile(SD_MMC,nomCsv.c_str(),",");
  appendFile(SD_MMC,nomCsv.c_str(),donnee.lng.c_str());
  appendFile(SD_MMC,nomCsv.c_str(),",");
  appendFile(SD_MMC,nomCsv.c_str(),donnee.bat1.courant.c_str());
  appendFile(SD_MMC,nomCsv.c_str(),",");
  appendFile(SD_MMC,nomCsv.c_str(),donnee.bat1.voltage.c_str());
  appendFile(SD_MMC,nomCsv.c_str(),",");
  appendFile(SD_MMC,nomCsv.c_str(),donnee.bat1.temperature.c_str());
  appendFile(SD_MMC,nomCsv.c_str(),",");
  appendFile(SD_MMC,nomCsv.c_str(),donnee.bat1.soc.c_str());
  appendFile(SD_MMC,nomCsv.c_str(),",");
  appendFile(SD_MMC,nomCsv.c_str(),donnee.bat1.santee.c_str());
  appendFile(SD_MMC,nomCsv.c_str(),"\n");
}

void modifierDonnee(String vitesse, String lat, String lng, String courant, String voltage, String temp, String soc, String santee){
  donnee.vitesse=vitesse;
  donnee.lat=lat;
  donnee.lng=lng;

  bat.courant=courant;
  bat.voltage=voltage;
  bat.temperature=temp;
  bat.soc=soc;
  bat.santee=santee;
  donnee.bat1 = bat;
}

void listDir(fs::FS &fs, const char *dirname, uint8_t levels) {
  USBSerial.printf("Listing directory: %s\n", dirname);

  File root = fs.open(dirname);
  if (!root) {
    USBSerial.println("Failed to open directory");
    return;
  }
  if (!root.isDirectory()) {
    USBSerial.println("Not a directory");
    return;
  }

  File file = root.openNextFile();
  while (file) {
    if (file.isDirectory()) {
      USBSerial.print("  DIR : ");
      USBSerial.println(file.name());
      if (levels) {
        listDir(fs, file.path(), levels - 1);
      }
    } else {
      USBSerial.print("  FILE: ");
      USBSerial.print(file.name());
      USBSerial.print("  SIZE: ");
      USBSerial.println(file.size());
    }
    file = root.openNextFile();
  }
}