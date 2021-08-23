#include <arduino-timer.h>
#include <Wire.h>
#include <DS3231.h>
#include <DHT.h>

RTClib myRTC;

#define DHTPIN 12
#define DHTTYPE    DHT22

#define DECODER_C     9
#define DECODER_B     10

#define DECODER_D     8
#define DECODER_A     11

#define HOUR_10       3
#define HOUR_1        2
#define MINUTE_10     4
#define MINUTE_1      5

#define DISPLAY_MS    2
#define BLANKING_US   200

#define FILTER_ALPHA  0.1

DHT dht(DHTPIN, DHTTYPE);
auto DHTtimer = timer_create_default();
unsigned long starttime = 0;
unsigned long endtime = 0;
bool showDHT = false;
float h, t;

int spoolMinute = 0;
int spoolHour = 0;
auto timer = timer_create_default();
int lastMinute;
int lastHour;

void displayDigit(unsigned short digit, unsigned short pin) {
  digitalWrite(DECODER_A, (1 << 0) & digit);
  digitalWrite(DECODER_B, (1 << 1) & digit);
  digitalWrite(DECODER_C, (1 << 2) & digit);
  digitalWrite(DECODER_D, (1 << 3) & digit);
  digitalWrite(pin, HIGH);
  delay(DISPLAY_MS);
  digitalWrite(pin, LOW);
  delayMicroseconds(BLANKING_US);
}

void calcRoll() {
  if (spoolMinute < 9) {
    spoolMinute++;
  }
  else {
    spoolMinute = 0;
  }
}

void readDHT() {
  h = dht.readHumidity();
  t = dht.readTemperature();

  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t)) {
    return;
  }
  showDHT = true;
  starttime = millis();
  endtime = starttime;
  lastMinute -= 1;
}

void setup() {
  pinMode(DECODER_A, OUTPUT);
  pinMode(DECODER_B, OUTPUT);
  pinMode(DECODER_C, OUTPUT);
  pinMode(DECODER_D, OUTPUT);

  pinMode(SET_HOUR, INPUT_PULLUP);
  pinMode(SET_MINUTE, INPUT_PULLUP);

  pinMode(HOUR_10, OUTPUT);
  pinMode(HOUR_1, OUTPUT);
  pinMode(MINUTE_10, OUTPUT);
  pinMode(MINUTE_1, OUTPUT);

  dht.begin();
  DHTtimer.every(30000, readDHT);

  timer.every(50, calcRoll);
  Wire.begin();

}

void loop() {

  DHTtimer.tick();
  DateTime now = myRTC.now();

  if ((endtime - starttime) >= 3000) {
    showDHT = false;
    starttime = 0;
    endtime = 0;
    lastMinute -= 1;
  }


  if (lastMinute < now.minute() || spoolMinute > 0) {
    timer.tick();
    displayDigit(spoolMinute, HOUR_10);
    displayDigit(spoolMinute, HOUR_1);
    displayDigit(spoolMinute, MINUTE_10);
    displayDigit(spoolMinute, MINUTE_1);
  }
  else if (showDHT == true)
  {
    displayDigit((int)round(t) / 10, HOUR_10);
    displayDigit((int)round(t) % 10, HOUR_1);
    displayDigit((int)round(h) / 10, MINUTE_10);
    displayDigit((int)round(h) % 10, MINUTE_1);
    endtime = millis();
  }
  else {
    displayDigit(now.hour() / 10, HOUR_10);
    displayDigit(now.hour() % 10, HOUR_1);
    displayDigit(now.minute() / 10, MINUTE_10);
    displayDigit(now.minute() % 10, MINUTE_1);
  }
  lastMinute = now.minute();
}
