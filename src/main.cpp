#include <Arduino.h>
#include "DFRobotDFPlayerMini.h"
#include <SoftwareSerial.h>
#include <OneButton.h>
SoftwareSerial softSerial(/*rx =*/2, /*tx =*/3);
#define FPSerial softSerial
unsigned long timer;
const uint16_t timerElmo = 15000;
const uint8_t maxVolume = 30;
const uint16_t durationBroadcast = 18500;
const uint16_t durationSecondBroadcast = 7000;
const uint32_t durationTvFirst = 80000; 
const uint16_t durationCoin = 10000;
const uint32_t delayBeforeCoin = 78000;
const uint16_t durationBorisVid = 40000;
const uint32_t durationPranksBeforeSoundChange = 98000;
const uint16_t durationVideo31 = 19000;
const uint16_t durationWinSeq = 3000;
const uint16_t timerTv3 = 1000;
const uint16_t timerMusicOnOffDuration = 60;
uint32_t timerCoin = 0;
class Action{
    public:
        bool isQued = false;
        bool isOnDelay = false;
        bool isRunning = false;
        bool isCompleted = false;
        uint32_t duration;
        uint32_t delayBefore = 0;
        callbackFunction startFunc = NULL;
        callbackFunction endFunc = NULL;

        void tick(){
            if (isCompleted) return;
            if (isQued){
                if (delayBefore == 0){
                    timer = millis()+duration;
                    isQued = false;
                    isRunning = true;
                    startFunc();
                    return;
                }
                timer = millis() + delayBefore;
                isQued = false;
                isOnDelay = true;
                return;
            }
            if (isOnDelay){
                if (millis()>timer){
                    timer = millis()+duration;
                    isRunning = true;
                    isOnDelay = false;
                    startFunc();
                }
            }
            if (isRunning && millis()>timer)            {
                isRunning = false;
                isCompleted = true;
                endFunc();
            }
        }
    Action(callbackFunction _startFunc, callbackFunction _endFunc, uint32_t _duration, uint32_t _delayBefore = 0){
        startFunc = _startFunc;
        endFunc = _endFunc;
        duration = _duration;
        delayBefore = _delayBefore;
    } 
};
DFRobotDFPlayerMini myDFPlayer;

//INPUTS
OneButton inputs[10] ={
OneButton(4, true), //remote1  
OneButton(5, true), //remote2
OneButton(6, true), //elmo done
OneButton(7, true), //prank done
OneButton(8, true), ////shigur door
OneButton(9, true), //last movie on
OneButton(18, true),  // baloons done
OneButton(10, true), // laughter after the pipes
OneButton(19, true),  // door to 2 opens
OneButton(1, true)   //music on/off
};
void onRemote1();   // open the door, start music, shut off the light -- letting customers in, 
void onRemote2();   // lock the door, start the broadcast, set broadcast timer  -- start the game!!!
void onElmo();      // start prank videos, turn music off
void onPrank();     // return music, turn balons on turn on baloons vid
void onTv1Room3();  // turn music off, set music timer
void onTv2Room3();  // turn music off, set music timer
void onBaloons();   // 
void changeVolume(bool on);
void startDFPlayer();
void musicGoesUp();
void musicGoesDown();
void pipeLaugh();
void musicButton();



uint8_t musicVolumeCounter = 0;
unsigned long timerMusicOnOff = 0;
bool musicIsOn = true;
bool gameOff = true;
bool gameStarted = false;
void broadcast1();
void endBroadcast1();
void startVid1();
void endVid1();
void startBaloons();
void emptyFunc();
void startPrank();
void endPrank();
void startBroadcast2();
void endBroadcast2();
void startVid31();
void endVid31();
void winSeq();
void winSeqEnd();
void onRoom2();
void onElmoStopLaughter();
void dropTheCoin();
void coinEnd();
void startBorisVid();
void endBorisVid();
void pipeLaughter();


Action startTheGame(broadcast1, endBroadcast1, durationBroadcast);
Action openingVid(startVid1, endVid1, durationTvFirst);
Action startTheBalloons(startBaloons, emptyFunc, 1200000, timerElmo);
Action startThePrank(startPrank, endPrank, durationPranksBeforeSoundChange, timerElmo);
Action borisVid(emptyFunc, endBorisVid, durationBorisVid);
Action coin(dropTheCoin, coinEnd, durationCoin, delayBeforeCoin);
Action room3FirstVid(startVid31, endVid31, durationVideo31);
Action onWin(winSeq, winSeqEnd, durationWinSeq);
Action laughter(pipeLaughter, emptyFunc, 4000, 2000);
Action  *actions[9]  = {&startTheGame, &coin, &openingVid, &startTheBalloons, 
                        &startThePrank, &borisVid, &room3FirstVid, 
                        &onWin, &laughter};

void setup(){
    //Serial.begin(9600);
    pinMode(11, OUTPUT);    //Fire the coin  -- to implement yet
    pinMode(12, OUTPUT);    //1video (chair)
    pinMode(13, OUTPUT);    //lights for coin
    pinMode(15, OUTPUT);    //2video (pranks) (filename should start with 3)
    pinMode(16, OUTPUT);    //4video (turn tv to logo)
    pinMode(17, OUTPUT);    //baloons
    digitalWrite(11, HIGH); 
    digitalWrite(12, HIGH);
    digitalWrite(13, HIGH);
    digitalWrite(15, HIGH);
    digitalWrite(17, LOW);
    digitalWrite(16, LOW);
    
    inputs[0].attachLongPressStart(onRemote1);
    inputs[1].attachLongPressStart(onRemote2);
    inputs[2].attachLongPressStart(onElmo);
    inputs[3].attachLongPressStart(onPrank);
    inputs[4].attachLongPressStart(onTv1Room3);
    inputs[5].attachLongPressStart(onTv2Room3);
    inputs[6].attachLongPressStart(onBaloons);
    inputs[7].attachLongPressStart(emptyFunc);
    inputs[8].attachLongPressStart(onRoom2);
    inputs[9].attachLongPressStart(musicButton);
    //inputs[8].attachLongPressStop(onElmoStopLaughter);
    for (uint8_t i = 0; i<10; i++){
        inputs[i].setClickTicks(30);
    }
    delay(1000);
    digitalWrite(17, HIGH);
    digitalWrite(16, HIGH);
    delay(2000);
    digitalWrite(16, LOW);

    
    startDFPlayer();
    

    digitalWrite(16, HIGH);

    digitalWrite(16, LOW);
    delay(500);
    digitalWrite(16, HIGH);

}

void loop(){
    changeVolume(musicIsOn);
    for (uint8_t i = 0; i<10; i++){
        inputs[i].tick();
    }
    for (uint8_t i = 0; i<9; i++){
        actions[i]->tick();
    }
}


void startDFPlayer(){
    FPSerial.begin(9600);
  //Serial.begin(115200);
  //Serial.println();
  ///Serial.println(F("DFRobot DFPlayer Mini Demo"));
  //Serial.println(F("Initializing DFPlayer ... (May take 3~5 seconds)"));
  if (!myDFPlayer.begin(FPSerial, /*isACK = */true, /*doReset = */true)) {  //Use serial to communicate with mp3.
    //Serial.println(F("Unable to begin:"));
    //Serial.println(F("1.Please recheck the connection!"));
    //Serial.println(F("2.Please insert the SD card!"));
    while(true){
      delay(0); // Code to compatible with ESP8266 watch dog.
    }
  }
  //Serial.println(F("DFPlayer Mini online."));
  myDFPlayer.volume(0);  //Set volume value. From 0 to 30
  //myDFPlayer.play(1);  //Play the first mp3
}

void onRemote1(){   // open the door, shut off the light -- letting customers in
    if (gameOff){
        myDFPlayer.playMp3Folder(1);
        gameOff = false;
    }
}
void onRemote2() {   // lock the door, start the broadcast, set broadcast timer  -- start the game!!!
    //digitalWrite(17, LOW);
    if (!gameOff && !gameStarted && !startTheGame.isCompleted){
        startTheGame.isQued = true;
        gameStarted = true;
        
    }
}

void onElmo(){      // start baloon pic and turn on baloons
    startTheBalloons.isQued = true;
}
void onBaloons(){
    startTheBalloons.isCompleted = true;
    startTheBalloons.isRunning = false;
    startThePrank.isQued = true;
}
void onPrank(){     // turns off prank video
        digitalWrite(16, LOW);
        delay(300);
        digitalWrite(16, HIGH);
}


void onTv1Room3(){  // play the broadcast, set music timer
        if (borisVid.isCompleted) room3FirstVid.isQued = true;
    }
void onTv2Room3(){  // turn music off, set music timer
        onWin.isQued = true;
    }

void changeVolume(bool on){
    //if (inputs[8].isLongPressed()) {
    //    musicGoesDown();
    //    return;}
    if (on) musicGoesUp();//myDFPlayer.volume(maxVolume);
    else musicGoesDown();//myDFPlayer.volume(1);
}
void musicGoesUp(){
    if (millis()>timerMusicOnOff && (musicVolumeCounter<31)){
            myDFPlayer.volume(musicVolumeCounter);
            timerMusicOnOff = millis()+timerMusicOnOffDuration;
            musicVolumeCounter++;
        }
}
void musicGoesDown(){
    if (millis()>timerMusicOnOff && (musicVolumeCounter>0)){
            myDFPlayer.volume(musicVolumeCounter);
            timerMusicOnOff = millis()+timerMusicOnOffDuration;
            musicVolumeCounter--;
        }
}
//void pipeLaugh(){
    //musicIsOn = true;
    //laughter.isQued = true;
//}
void broadcast1(){
    myDFPlayer.advertise(1);
}
void endBroadcast1(){
    openingVid.isQued =true;
    musicIsOn = false;
}
void startVid1(){
    digitalWrite(12, LOW);
    digitalWrite(13, LOW);
    delay(300); 
    digitalWrite(12, HIGH);
    coin.isQued = true;
}
void endVid1(){
    musicIsOn = true;
    
    
}
void startBaloons(){
    digitalWrite(17, LOW);
    
}
void emptyFunc(){}
void startPrank(){
    musicIsOn = false;
    digitalWrite(15, LOW);
    delay(300);
    digitalWrite(15, HIGH);
}
void endPrank(){    // thats when music returns, not when vid ends. vid ends on prank done
    musicIsOn = true;
}

void startVid31(){
    
    musicIsOn = false;
}
void endVid31(){
    musicIsOn = true;
}
void winSeq(){
    digitalWrite(17, LOW);
    myDFPlayer.playMp3Folder(2);
}
void winSeqEnd(){
    musicIsOn = false;
}
void onRoom2(){

    myDFPlayer.advertise(3);
    delay(1000);
    digitalWrite(16, LOW);
    delay(1000);
    digitalWrite(16, HIGH);
    delay(6000);
    musicIsOn = false;
    borisVid.isQued = true;
    
    }

void onElmoStopLaughter(){
    myDFPlayer.volume(30);
}
void dropTheCoin(){
    digitalWrite(11, LOW);
    digitalWrite(13, HIGH);
}
void coinEnd(){
    digitalWrite(11, HIGH);
}

void endBorisVid(){
    musicIsOn = true;
    //myDFPlayer.advertise(3);    
}
void musicButton(){
    if(musicIsOn) musicIsOn = false;
    else musicIsOn = true;
}
void pipeLaughter(){

    //myDFPlayer.advertise(2);
}