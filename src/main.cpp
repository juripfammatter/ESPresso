#include <Arduino.h>
#include "OneWire.h"
#include "DallasTemperature.h"

#include <SPI.h> 
#include <Wire.h>
#include "Adafruit_GFX.h"
#include "Adafruit_SSD1306.h"

//OLED Defines
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET     4 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3D ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// GPIO setup
#define ONE_WIRE_BUS 32

//TempSensor setup
#define DS18B20RESOLUTION 12          //12 bit
OneWire oneWire(ONE_WIRE_BUS); 
DallasTemperature sensor_1(&oneWire);
static uint8_t address_1[3];

//Iterables
uint8_t i = 0, j=0, tempIter = 0;

//Arrays
float temperatures1[128], temperatures2[128], temperatures3[128];

//States
#define pageButtonPin 35
#define timerButtonPin 14
static uint8_t pageButtonState, lastPageButtonState=0;
static uint8_t timerButtonState, lastTimerButtonState=0;
static uint8_t pageState = 0;

//Timer
static unsigned long customTime = 0,startcustomTime = 0;
bool customTimerStarted = false;


//Predefinitions
uint8_t readPageButton(void);
uint8_t readTimerButton(void);
void plotArray(float array[]);

void setup() {
  /*--- Buttons ---*/
  pinMode(pageButtonPin, INPUT_PULLDOWN);
  pinMode(timerButtonPin, INPUT_PULLDOWN);

  /*--- TEMP SENSOR ---*/

  // Start up the library 
  sensor_1.begin();
  sensor_1.getAddress(&address_1[0], 0);
  sensor_1.setResolution(&address_1[0],DS18B20RESOLUTION);
  sensor_1.getAddress(&address_1[1], 1);
  sensor_1.setResolution(&address_1[1],DS18B20RESOLUTION);
  //sensor_1.getAddress(&address_1[2], 2);
  //sensor_1.setResolution(&address_1[2],DS18B20RESOLUTION);
  
  Serial.begin(9600); 


  /*--- OLED ---*/
  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  // Show initial display buffer contents on the screen --
  // the library initializes this with an Adafruit splash screen.
  display.display();
  delay(100); // Pause

  // Clear the buffer
  display.clearDisplay();

  // Draw a single pixel in white
  display.drawPixel(10, 10, SSD1306_WHITE);

  // Show the display buffer on the screen. You MUST call display() after
  // drawing commands to make them visible on screen!
  display.display();
  delay(100);


  // Invert and restore display, pausing in-between
  display.invertDisplay(true);
  delay(100);
  display.invertDisplay(false);
  delay(100);
  display.clearDisplay();
  display.display();

}

void loop() {
  //Get Temperatures
  sensor_1.requestTemperatures();
  float temp_1 = sensor_1.getTempCByIndex(0);
  float temp_2 = sensor_1.getTempCByIndex(1);
  float temp_3 = sensor_1.getTempCByIndex(2);
  //Serial.printf("%.3f;%.3f;%.3f\n",temp_1,temp_2,temp_3);
  if(millis()%100 == 0){
    temperatures1[tempIter] = temp_1;
    temperatures2[tempIter] = temp_2;
    temperatures3[tempIter] = temp_3;

    if(tempIter<128){
      tempIter++;
    }else{
      tempIter = 0;
    }
  }
  //States
  if(readPageButton()){
    pageState++;
  }
  if(pageState == 3){
    pageState = 0;
  }
  switch(pageState){
    case 0: {
            //Display Temperatures
            display.clearDisplay();
            display.setTextColor(SSD1306_WHITE);
            
            //Labels
            display.setTextSize(1);
            display.setCursor(0,0);
            display.printf("Boiler:");
            display.setCursor(0,25);
            display.printf("Anfang:");
            display.setCursor(0,50);
            display.printf("Ende:");

            //Measurements
            display.setTextSize(2);
            display.setCursor(56,0);
            display.printf("%.2f\n",abs(temp_1));
            display.setCursor(56,25);
            display.printf("%.2f\n",abs(temp_2));
            display.setCursor(56,50);
            display.printf("%.2f\n",abs(temp_3));
            display.display();
            delay(10);
            }break;
    case 1: {
            if(readTimerButton()){
              if(!customTimerStarted){
                customTimerStarted = true;
                startcustomTime = millis();
              }else{
                customTimerStarted = false;
              }
            }
            if(customTimerStarted == true){
              customTime = millis()-startcustomTime;
            }
            display.clearDisplay();
            display.setTextColor(SSD1306_WHITE);
            //Labels
            display.setTextSize(4);
            display.setCursor(0,0);
            display.printf("%.2f",(float) customTime/1000.0);
            

            //Graphics
            float totalTime = 25.5, preInfusion = 6.1;
            float firstBlock = preInfusion/(totalTime+5)*128.0;
            display.drawRect(0,32,int(firstBlock),16,SSD1306_WHITE);
            display.drawRect(int(firstBlock)-1,32,int((totalTime/preInfusion-1)*firstBlock),16,SSD1306_WHITE);

            //Text
            display.setTextSize(1);
            display.setCursor(3,36);
            display.printf("Pre");
            display.setCursor(int(firstBlock)+1,36);
            display.printf("Infusion");
            display.fillRect(0,48,int(customTime/(1000.0*preInfusion)*firstBlock),16,SSD1306_WHITE);
            display.display();
            }break;
    case 2: {
            plotArray(temperatures1);
            }break;
  }
}

/*User Defines*/

uint8_t readPageButton(void){
  pageButtonState = digitalRead(pageButtonPin);
  //TBD edge detection
  if(pageButtonState && !lastPageButtonState){
    lastPageButtonState = pageButtonState;
    return 1;
  }
  lastPageButtonState = pageButtonState;
  return 0;
}

uint8_t readTimerButton(void){
  timerButtonState = digitalRead(timerButtonPin);
  //TBD edge detection
  if(timerButtonState && !lastTimerButtonState){
    lastTimerButtonState = timerButtonState;
    Serial.print("1");
    return 1;
  }
  lastTimerButtonState = timerButtonState;
  return 0;
}

void plotArray(float array[]){
  display.clearDisplay();
  for(j = 0; j<128; j++){
    display.drawPixel(j, abs(array[j]/2),SSD1306_WHITE);
  }
  display.display();
}