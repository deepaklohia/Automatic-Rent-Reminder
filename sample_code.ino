#define BLYNK_TEMPLATE_ID "your template id here"
#define BLYNK_TEMPLATE_NAME "ANNOUNCEMENT"
#define BLYNK_FIRMWARE_VERSION "0.0.2"

#define BLYNK_PRINT Serial
//#define BLYNK_DEBUG
//#define APP_DEBUG

// Uncomment your board, or configure a custom board in Settings.h
//#define USE_SPARKFUN_BLYNK_BOARD
#define USE_NODE_MCU_BOARD
//#define USE_WITTY_CLOUD_BOARD
//#define USE_WEMOS_D1_MINI

#define ER_OFF HIGH
#define ER_ON LOW 

#define VPIN_MOD_POWER  V0  //MODE POWER
#define VPIN_TRK_NXT    V1 //NEXT TRACT
#define VPIN_TRK_PRE    V2 //PREVIOUS TRACK
#define VPIN_VOL_DOWN   V3 // VOL UP
#define VPIN_VOL_UP     V4 // VOLUME DOWN
#define VPIN_RENT_REM_AUTO   V5 // RENT DAY OR NOT --- FOR AUTOMATIONS
#define VPIN_ANNOUCE_TIMEOUT  V6 // ANNOUCE TIMEOUT
#define VPIN_INFO             V7 // info display
#define VPIN_START_STOP       V8 //START STOP TRACK
#define VPIN_VOLUME_RELAY     V9 //Volume Relay

#define VPIN_TRACK_ONS_LATA_JI  V11 //TRACK 1 ONS LATA JI
#define VPIN_TRACK_ONS_DHUN  V12 //TRACK 2 ONS DHUN
#define VPIN_TRACK_DAILY_ANNOUCE  V13 //TRACK 3  DAILY ANNOUCEMENT
#define VPIN_RENT_REM_FORCE  V14 //TRACK 4  FORCE RENT REMINDER
#define VPIN_TRACK5  V15 //TRACK 5 SOLO DAILY ANNOUNCEMENT

#define VPIN_TRACK_SOLO_NEAR_TANK  V17 //TRACK 7 SOLO NEAR TANK
#define VPIN_TRACK_SOLO_FINE  V18 //TRACK 8 SOLO FINE DIRT
#define VPIN_REND_REMIND_START_DATE  V19 //START DATE
#define VPIN_REND_REMIND_END_DATE  V20 //END DATE

#include "BlynkEdgent.h"
#include <TimeLib.h>
#include <WidgetRTC.h>

//D1 ARDIUNO BASED
static const uint8_t D2   = 16; //SAFE TO USE? --- HIGH ON BOOT ??
static const uint8_t D5   = 14; //SAFE TO USE? --- HIGH ON BOOT ??
static const uint8_t D6   = 12; //SAFE TO USE?
static const uint8_t D7   = 13; //SAFE TO USE?

int const relayPin_AMP_POWER = D6; 
int const relayPin_NEXT_TRK = D7;
int const relayPin_PRE_TRK = D2 ;
int const relayPin_MOD_POWER = D5 ; 
String lastAnnouceDate;
String annouceDate;
String txt ;
bool rentDay;
bool powerOn;

unsigned long annouceStartMilliSec;
unsigned long annouceTimeout ;  //15Min **** HOW MANY 
int annouceTimeoutSec ;
unsigned long volRelayTimeMilli;
int start_date;
int end_date;
String currentTime;
String currentDate;
bool runOnce;

BlynkTimer timer;
WidgetRTC rtc;

// Digital clock display of the time
void clockDisplay(){
  if (year()>2020 && !runOnce){
    currentTime = String(hour()) + ":" + minute() + ":" + second();
    currentDate = String(day()) + "-" + month() + "-" + year();
    if (day() >= start_date && day() <= end_date){rentDay = true;}else{ rentDay = false; }
    runOnce = true;
  }
  //timeout
  if (powerOn &&  ((millis()  -  annouceStartMilliSec) >= annouceTimeout )){ power(false); }
  //show rent day or not
  txt = rentDay? "RENT DAY":"GREAT DAY" ; 
  Blynk.virtualWrite(VPIN_INFO, currentDate + "|" + currentTime + ">>" + txt);
}

BLYNK_CONNECTED(){
  //startup defaults
  Blynk.virtualWrite(VPIN_INFO, "");
  Blynk.virtualWrite(VPIN_TRK_NXT, 0);
  Blynk.virtualWrite(VPIN_START_STOP, 0);
  Blynk.virtualWrite(VPIN_MOD_POWER, 0);
  Blynk.virtualWrite(VPIN_RENT_REM_AUTO, 0);
  Blynk.virtualWrite(VPIN_TRACK_ONS_LATA_JI, 0);
  Blynk.virtualWrite(VPIN_TRACK_ONS_DHUN, 0); 
  Blynk.virtualWrite(VPIN_TRACK_DAILY_ANNOUCE, 0);
  Blynk.virtualWrite(VPIN_RENT_REM_FORCE, 0);
  Blynk.virtualWrite(VPIN_TRACK5, 0);
  Blynk.virtualWrite(VPIN_TRACK_SOLO_NEAR_TANK, 0); 
  Blynk.virtualWrite(VPIN_TRACK_SOLO_FINE, 0);

  Blynk.sendInternal("rtc", "sync"); //request current local time for device
  Blynk.syncVirtual(VPIN_MOD_POWER);
  Blynk.syncVirtual(VPIN_TRK_NXT);  
  Blynk.syncVirtual(VPIN_ANNOUCE_TIMEOUT);
  Blynk.syncVirtual(VPIN_RENT_REM_AUTO);
  Blynk.syncVirtual(VPIN_INFO);
  Blynk.syncVirtual(VPIN_START_STOP);
  Blynk.syncVirtual(VPIN_VOLUME_RELAY);
  Blynk.syncVirtual(VPIN_VOL_UP);
  Blynk.syncVirtual(VPIN_VOL_DOWN);
  Blynk.syncVirtual(VPIN_TRACK_ONS_LATA_JI);
  Blynk.syncVirtual(VPIN_TRACK_ONS_DHUN);
  Blynk.syncVirtual(VPIN_TRACK_DAILY_ANNOUCE);
  Blynk.syncVirtual(VPIN_RENT_REM_FORCE);
  Blynk.syncVirtual(VPIN_TRACK5);
  Blynk.syncVirtual(VPIN_TRACK_SOLO_NEAR_TANK);
  Blynk.syncVirtual(VPIN_TRACK_SOLO_FINE);
  Blynk.syncVirtual(VPIN_REND_REMIND_START_DATE);
  Blynk.syncVirtual(VPIN_REND_REMIND_END_DATE);
}

BLYNK_WRITE(VPIN_MOD_POWER){if (param.asInt()){power(true); }  else{ power(false); }}
BLYNK_WRITE(VPIN_TRK_PRE){ if (param.asInt()){ preTrk();}}
BLYNK_WRITE(VPIN_TRK_NXT){if (param.asInt()){ nextTrk();}}
BLYNK_WRITE(VPIN_VOL_UP){if (param.asInt()){ volup(true); }}
BLYNK_WRITE(VPIN_VOL_DOWN){if (param.asInt()){ volup(false); }}
BLYNK_WRITE(VPIN_VOLUME_RELAY){ volRelayTimeMilli = param.asInt();}
BLYNK_WRITE(VPIN_ANNOUCE_TIMEOUT){ annouceTimeout  = param.asInt() * 60UL * 1000UL;   }
BLYNK_WRITE(VPIN_START_STOP){if (param.asInt()){  nextTrk(); }}
BLYNK_WRITE(VPIN_TRACK_ONS_LATA_JI){if (param.asInt()){ playAnnouce(1);}}
BLYNK_WRITE(VPIN_TRACK_ONS_DHUN){if (param.asInt() ){ playAnnouce(2); }}
BLYNK_WRITE(VPIN_TRACK_DAILY_ANNOUCE){ if (param.asInt()){playAnnouce(3); }}
BLYNK_WRITE(VPIN_RENT_REM_FORCE){if (param.asInt()){ playAnnouce(4); }}
BLYNK_WRITE(VPIN_RENT_REM_AUTO){if (param.asInt() && rentDay){playAnnouce(4); }}
BLYNK_WRITE(VPIN_TRACK5){if (param.asInt()){playAnnouce(5); }}
BLYNK_WRITE(VPIN_TRACK_SOLO_NEAR_TANK){if (param.asInt()){ playAnnouce(7); }}
BLYNK_WRITE(VPIN_TRACK_SOLO_FINE){if (param.asInt()){ playAnnouce(8); }}
BLYNK_WRITE(VPIN_REND_REMIND_START_DATE){ start_date = param.asInt();}
BLYNK_WRITE(VPIN_REND_REMIND_END_DATE){ end_date = param.asInt();}

void setup(){
  Serial.begin(115200);
  delay(100);
  BlynkEdgent.begin();

  pinMode(relayPin_AMP_POWER, OUTPUT);
  pinMode(relayPin_NEXT_TRK, OUTPUT);
  pinMode(relayPin_PRE_TRK, OUTPUT);
  pinMode(relayPin_MOD_POWER, OUTPUT);

  digitalWrite(relayPin_AMP_POWER, ER_OFF);
  digitalWrite(relayPin_NEXT_TRK, ER_OFF);
  digitalWrite(relayPin_PRE_TRK, ER_OFF);
  digitalWrite(relayPin_MOD_POWER, ER_OFF);

  setSyncInterval(60 * 60); // Sync interval in seconds (10 minutes)
  timer.setInterval(5000L, clockDisplay);
}

void loop() {
  BlynkEdgent.run();
  timer.run();
  delay(1000);
}

void playAnnouce(int whichSound){
  annouceStartMilliSec = millis();
  //annouce
  if (powerOn) {power(false);}
  power(true);
  volup(false);

  nextTrk(); //first Track
  delay(1000);

  if (whichSound == 2){trackPlayer(2-1); }  //0002 ONS Dhun
  else if (whichSound == 3){trackPlayer(3-1); }  //0003 Daily Annoucement
  else if (whichSound == 4){trackPlayer(4-1); }  //0004 - rent reminder-MP3
  else if (whichSound == 5){trackPlayer(5-1); } //0005 DAILY ANNOUCEMENT
  else if (whichSound == 6){trackPlayer(6-1); }//0006 RENT REMINDRE SOLO
  else if (whichSound == 7){ trackPlayer(7 - 1);} //0007 NEAR TANKER
  else if (whichSound == 8){ trackPlayer(8-1); } //0008 GANDAGI FINE
  else if (whichSound == 10){ trackPlayer(10-1); } //0009 BABA NAZAR
}

void trackPlayer(int loop){
  for (int i = 0; i <loop; i++){
    nextTrk();
    delay(500);
    nextTrk();
    delay(500);
  }
}

void preTrk(){
  digitalWrite(relayPin_PRE_TRK, ER_ON);
  delay(300);
  digitalWrite(relayPin_PRE_TRK, ER_OFF);
  delay(200);
}

void nextTrk(){
  delay(500);
  digitalWrite(relayPin_NEXT_TRK, ER_ON);
  delay(300);
  digitalWrite(relayPin_NEXT_TRK, ER_OFF);
  delay(200);
}

void volup(bool stat){
  if (stat){ //VOLUMNE UP
    delay(500);
    digitalWrite(relayPin_NEXT_TRK, ER_ON);
    delay(volRelayTimeMilli);
    digitalWrite(relayPin_NEXT_TRK, ER_OFF);
    delay(500);
  }else{ //VOLUME DOWN
    delay(500);
    digitalWrite(relayPin_PRE_TRK, ER_ON);
    delay(volRelayTimeMilli);
    digitalWrite(relayPin_PRE_TRK, ER_OFF);
    delay(500);
  }
}

void power(bool stat){
  if (stat && !powerOn){
    digitalWrite(relayPin_MOD_POWER, ER_ON);
    digitalWrite(relayPin_AMP_POWER, ER_ON);
    Blynk.virtualWrite(VPIN_MOD_POWER, 1);
    powerOn  = true ;
  }else if (!stat && powerOn){
    digitalWrite(relayPin_MOD_POWER, ER_OFF);
    digitalWrite(relayPin_AMP_POWER, ER_OFF);
    Blynk.virtualWrite(VPIN_MOD_POWER, 0);
    powerOn  = false ;
    delay(2000);
  }
  delay(1000);
}
