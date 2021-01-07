#include <Arduino.h>

#ifdef ESP32
#include <WiFi.h>
#include "SPIFFS.h"
#else
#include <ESP8266WiFi.h>
#endif
#include "FS.h"
#include "AudioFileSourceSPIFFS.h"
#include "AudioFileSourceICYStream.h"
#include "AudioFileSourceBuffer.h"
#include "AudioGeneratorMP3.h"
#include "AudioOutputI2SNoDAC.h"

int btm = 0;
int led = 2;

const char *SSID = "ap";
const char *PASSWORD = "pwd";
const int URLsize = 4;
const char *URL[URLsize] = {"http://185.33.21.112:80/atr_128", "http://nas.lan:81/play.mp3", "http://nas.lan:81/play1.mp3", "http://nas.lan:81/play2.mp3"};

boolean flag = false;
int URLindex = 0;

AudioGeneratorMP3 *mp3;
AudioFileSourceSPIFFS *file;
AudioFileSourceICYStream *fileStream;
AudioFileSourceBuffer *buff;
AudioOutputI2SNoDAC *out;

void StopPlaying() {
  Serial.printf("StopPlaying()\n");
  if (mp3) {
    //    Serial.printf("mp3 stop\n");
    mp3->stop();
  }
  if (file) {
    //    Serial.printf("file close\n");
    file->close();
    delete file;
    file = NULL;
  }
  if (fileStream) {
    //    Serial.printf("fileStream close\n");
    fileStream->close();
    delete fileStream;
    fileStream = NULL;
  }
  if (buff) {
    //    Serial.printf("buff close\n");
    //        buff->close();
    delete buff;
    buff = NULL;
  }
  Serial.printf("stop OK\n");
}

void initMP3() {
  Serial.printf("initMP3()\n");
  file = new AudioFileSourceSPIFFS("/NyanCat.mp3");
  mp3->begin(file, out);
  Serial.printf("Playing...\n");
}

void initHTTP() {
  Serial.printf("initHTTP()\n");
  if (WiFi.status() != WL_CONNECTED) {
    Serial.printf("WiFi Connecting ");
    WiFi.begin(SSID, PASSWORD);
  }

  while (WiFi.status() != WL_CONNECTED) {
    Serial.printf(".");
    digitalWrite(led, LOW);
    delay(100);
    digitalWrite(led, HIGH);
    delay(100);
  }
  Serial.printf("\nWiFi OK\n");
  digitalWrite(led, HIGH);
  Serial.printf("URLindex: %d  URLsize: %d\n", URLindex, URLsize);
  fileStream = new AudioFileSourceICYStream(URL[URLindex]);
  buff = new AudioFileSourceBuffer(fileStream, 4096);
  mp3->begin(buff, out);
  Serial.printf("Playing...\n");
}

void setup() {
  Serial.begin(115200);
  Serial.printf("\n\n");
  out = new AudioOutputI2SNoDAC();
  mp3 = new AudioGeneratorMP3();

  pinMode(led, OUTPUT);
  pinMode(btm, INPUT);

  digitalWrite(led, LOW);
  delay(500);
  if (digitalRead(btm) == LOW) {
    Serial.printf("Start 'flag' change\n");
    flag = !flag;
  }
  digitalWrite(led, HIGH);

  if (flag) {
    initHTTP();
  } else {
    initMP3();
  }
}

void loop() {
  if (mp3->isRunning()) {

    if (mp3->loop()) {
      digitalWrite(led, LOW);
    } else {
      StopPlaying();
      digitalWrite(led, HIGH);
    }

  } else {

    if (flag) {
      initHTTP();
    } else {
      initMP3();
    }

  }

  if (digitalRead(btm) == LOW) {
    Serial.printf("btm PUSH\n");
    while (digitalRead(btm) == LOW) {
      digitalWrite(led, LOW);
      delay(50);
      digitalWrite(led, HIGH);
      delay(50);
    }
    digitalWrite(led, HIGH);
    StopPlaying();
    if (flag) {
      if ( ++URLindex < URLsize) {
        //        for (int i = 0; i < URLindex + 1; i++) {
        //          digitalWrite(led, LOW);
        //          delay(1000 / URLsize * ( URLindex + 1 ) );
        //          digitalWrite(led, HIGH);
        //        }
        initHTTP();
      } else {
        URLindex = 0;
        //        digitalWrite(led, LOW);
        //        delay(1000);
        //        digitalWrite(led, HIGH);
        flag = false;
        initMP3();
      }
    } else {
      //      digitalWrite(led, LOW);
      //      delay(1000 / URLsize);
      //      digitalWrite(led, HIGH);
      flag = true;
      initHTTP();
    }

  }
}

