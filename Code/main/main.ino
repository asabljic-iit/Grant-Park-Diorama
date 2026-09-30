/*
 * FILE: 
 * AUTHORS: Amir, Nathan, Mayan
 * DATE: 07-21-2021
 * DESCRIPTION: Creating a function to light up LED lights with SQM sensor.
 */


//// LIBRARIES ////
#include <Arduino.h>
#include <SD.h>
#include <SPI.h>
#include <Wire.h>
#include "Adafruit_TCS34725.h"    //Library for RGB Sensor
#include "rgb.h"                  // Import of a local header (.h) file


//// VARIABLES ////
// SD Variables
String dataHeader = "Time(ms),SQM Value,Red,Green,Blue,Color Temp,Lux,r,g,b,c";  // Provides the string to write in at setup to explain the data.
String dataString ="";      // holds the data to be written to the SD card
File sensorData;
File myFile;
const int chipSelect = BUILTIN_SDCARD; //for Teensy3.5
// SQM Variables
volatile unsigned long cnt = 0;
unsigned long oldcnt = 0;
unsigned long t = 0;
unsigned long last;
float SQM_value;
// PIR Variables
int ledPin = 13;                // choose the pin for the LED
int inputPin = 7;               // choose the input pin (for PIR sensor)
int pirState = LOW;             // we start, assuming no motion detected
int val = 0;                    // variable for reading the pin status

//pir2
int ledPin2 = 15;                // choose the pin for the LED
int inputPin2 = 8;               // choose the input pin (for PIR sensor)
int pirState2 = LOW;             // we start, assuming no motion detected
int val2 = 0;                    // variable for reading the pin status

//Light Control Variables
bool night = false;


//// FUNCTIONS ////
// SD Functions
void SD_setup(){ // initalizes and creates a file
  Serial.print("Initializing SD card...");
  // IF FAILURE OCCURS: 
    if (!SD.begin(chipSelect)) {
      Serial.println("initialization failed!");
      return;
    }
  
  Serial.println("initialization done.");
  sensorData = SD.open("data.csv", FILE_WRITE);
  sensorData.close();
  //read_SD_Directory(); 
}

void prepare_data(){ // prepares data
  dataString = String(millis()) + "," + String(SQM_value)
                                + "," + String(red)
                                + "," + String(green)
                                + "," + String(blue)
                                + "," + String(colorTemp)
                                + "," + String(lux)
                                + "," + String(r)
                                + "," + String(g)
                                + "," + String(b)
                                + "," + String(c);
  Serial.println(dataString); //Only to show on the Serial Monitor.
}

void saveData(String writeThistoSD){ // save to SD
    if(SD.exists("data.csv")){ // check the card is still there
      // now append new data file
      sensorData = SD.open("data.csv", FILE_WRITE);
      if (sensorData){
      sensorData.println(writeThistoSD);
      Serial.print("Data saved! Data: ");
      Serial.println(writeThistoSD);
      sensorData.close(); // close the file
    }
  }
  else{
    Serial.println("Error writing to file !");
  }
}

// SQM Functions
void irq1()
{
  cnt++;
}

void SQM_setup() 
{
  Serial.println("START");
  pinMode(14, INPUT);
  digitalWrite(14, HIGH);
  attachInterrupt(14, irq1, RISING);
}

float SQM_loop() 
{

  //if (millis() - last >= 1000)
  //{
    //last = millis();
    //t = cnt;
    unsigned long hz = t - oldcnt;
        SQM_value = (hz+50)/100;
    //Serial.print("FREQ: "); 
    //Serial.print(hz);
    //Serial.print("\t = ");
    //Serial.print(SQM_value);  // +50 == rounding last digit
    //Serial.println(" mW/m2");
    //oldcnt = t;
  //}
  return SQM_value;  
}

// PIR Functions
//pir1
void PIR1_loop(){
  if (night == true){
  val = digitalRead(inputPin);  // read input value
  if (val == HIGH) {            // check if the input is HIGH
    analogWrite(ledPin, 200);  // turn LED ON
    if (pirState == LOW) {
      // we have just turned on
      Serial.println("DEBUG: Motion detected on PIR1!");
      // We only want to print on the output change, not state
      pirState = HIGH;
    }
  } else {
    analogWrite(ledPin, 10); // dim LED
    if (pirState == HIGH){
      // we have just turned of
      Serial.println("DEBUG: Motion ended on PIR1!");
      // We only want to print on the output change, not state
      pirState = LOW;
    }
  }
  }
}

void PIR1_setup(){
  pinMode(ledPin, OUTPUT);      // declare LED as output
  pinMode(inputPin, INPUT);     // declare sensor as input
  //analogWrite(ledPin, 10);

  // ALWAYS PLACE at the bottom/last line of the setup
  //attachInterrupt(inputPin, PIR1_loop, CHANGE); //or FALLING, CHANGE
}

//pir2
void PIR2_loop(){
  if (night == true) {
  val2 = digitalRead(inputPin2);  // read input value
  if (val2 == HIGH) {            // check if the input is HIGH
    analogWrite(ledPin2, 200);  // turn LED ON
    if (pirState2 == LOW) {
      // we have just turned on
      Serial.println("DEBUG: Motion detected on PIR2!");
      // We only want to print on the output change, not state
      pirState2 = HIGH;
    }
  } else {
    analogWrite(ledPin2, 10); // dim LED
    if (pirState2 == HIGH){
      // we have just turned of
      Serial.println("DEBUG: Motion ended on PIR2!");
      // We only want to print on the output change, not state
      pirState2 = LOW;
    }
  }
  }
}

void PIR2_setup(){
  pinMode(ledPin2, OUTPUT);      // declare LED as output
  pinMode(inputPin2, INPUT);     // declare sensor as input
  //analogWrite(ledPin, 10);

  // ALWAYS PLACE at the bottom/last line of the setup
  //attachInterrupt(inputPin2, PIR2_loop, CHANGE); //or FALLING, CHANGE
}

// Dim Functions
void lightControl_setup()
{
  pinMode(ledPin, OUTPUT);
}


void lightControl_loop()
{
  if (SQM_value < 250){
    Serial.println("DEBUG: Its dark outside!");
    if (night == false){
      night = true;
      //Serial.println(night);
      analogWrite(ledPin, 10);
      analogWrite(ledPin2, 10);
    }
    attachInterrupt(inputPin, PIR1_loop, CHANGE);
    attachInterrupt(inputPin2, PIR2_loop, CHANGE);
  } else {
    night = false;
    Serial.println("DEBUG: Its bright outside!");
    //Serial.println(night);
    delay(500);
    analogWrite(ledPin, 0);
    analogWrite(ledPin2, 0);
  }
}


//// SETUP ////
void setup() {
  Serial.begin(9600); // Start serial
  
  // Data Collection Setup
  SD_setup();
  SQM_setup();
  RGB_setup();
  delay(1000);
  saveData(dataHeader); // Prints the header
  delay(500);

  // Light Control Setup
  PIR1_setup();
  PIR2_setup();
  lightControl_setup();
}


//// LOOP ////
void loop() {
  if (millis() - last >= 1000){ //makes sure that data collection is being done every second instead of millisecond
    last = millis();
    t = cnt;

    // Data Collection & Saving
    SQM_loop();
    RGB_loop();
    prepare_data();
    saveData(dataString); // save to SD card
    
    // Light Control
    lightControl_loop();
    oldcnt = t;
  }
}