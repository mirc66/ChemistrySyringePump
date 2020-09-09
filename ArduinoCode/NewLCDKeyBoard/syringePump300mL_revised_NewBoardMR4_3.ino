/*********************
//file:syringepump300mlrevisednewboard-MR2 ;openscreen:syringe5.2   
//      testing key recognition inside if (button&button up loop) "works"
//file:syringepump300mlrevisednewboard-MR3 ;openscreen:syringe5.3
//      testing sendind key presed to key outside the if button loop needs to use buttons variable
//file:syringepump300mlrevisednewboard-MR4 ;openscreen:syringe5.4 clean readKey()to use button
//      remove getKey()subroutine; Main Speed up and down arrows adjust speed which is set correctly
//      when you hit Select again in this menu the up and down arrows work to adjust the Volume but 
//      left and right not set up properly
//file:syringepump300mlrevisednewboard-MR4_2 ;openscreen:syringe5.4_2 works now with bolus() commented out
//file:syringepump300mlrevisednewboard-MR4_3 ;openscreen:syringe5.4_2 works now with bolus()"works"


// New Adafruit LCD-key shield https://www.adafruit.com/product/772
// BigEasyDriver board to control stepper motor:https://www.sparkfun.com/products/12859

//7/14/20 Hans asked to add 20mL/min and change 400mL/min to 300mL/min

//   SpeedSetting in microSecDelay      Speed Display in mL/min
//          2250                              10    tested with timer
//          1150                              20    tested with timer
//           450                              50   tested with timer
//            75                             300  tested with timer
//  
// The shield uses the I2C SCL and SDA pins. On classic Arduinos
// this is Analog 4 and 5 so you can't use those for analogRead() anymore
// However, you can connect other I2C sensors to the I2C bus and share
// the I2C bus.

Example code for the Adafruit RGB Character LCD Shield and Library

Note SyringePumpV5 is same as V4 but with new LCD-KEY shield board

**********************/

//=========================================== include the library code:=====================================================================================================
#include <Wire.h>
#include <Adafruit_RGBLCDShield.h>
#include <utility/Adafruit_MCP23017.h>      //port expander needed for rgblcdshield
//
//==========================================variable definitions:===========================================================================================================

Adafruit_RGBLCDShield lcd = Adafruit_RGBLCDShield();

// These #defines make it easy to set the backlight color
#define RED 0x1
#define WHITE 0x7

/* -- Constants --  placed here become global */
#define SYRINGE_VOLUME_ML 300.0
#define SYRINGE_BARREL_LENGTH_MM 155.0 //measured with ruler

#define THREADED_ROD_PITCH 1.25  //metric thread 1.25 mm between thread peaks Rod advances 1.25mm/Rev(MR)
#define STEPS_PER_REVOLUTION 200.0  //default from BigEasyDriver board
#define MICROSTEPS_PER_STEP 16.0 //big easy driver defaults to 16 microsteps per Step

#define SPEED_MICROSECONDS_DELAY 450 // (75=300mL/min for 300mL Syringe)(450 = 50mL/min for 300mL syringe) (2250 = 10mL/min for 300mL syringe)

#define mLBolusStep 10.0

float SpeedDisplay = 50.0;// in mL/min

long ustepsPerML = (MICROSTEPS_PER_STEP * STEPS_PER_REVOLUTION * SYRINGE_BARREL_LENGTH_MM) / (SYRINGE_VOLUME_ML * THREADED_ROD_PITCH );

/* -- Pin definitions -- */
int motorDirPin = 2;
int motorStepPin = 3;


//
//key#:0          1             2       3       4   //not for new board it uses the button variable
enum{KEY_SELECT, KEY_RIGHT, KEY_LEFT, KEY_DOWN, KEY_UP, KEY_NONE};//gives order of key numbers not new board
int NUM_KEYS = 5;
int adc_key_in;
int key = KEY_NONE;

/* -- Enums and constants -- */
enum{PUSH,PULL}; //syringe movement direction
enum{MAIN, SPEED_MENU}; //UI states

const int SpeedSettingsLength = 4;
//index                     0     1     2     3
float SpeedSettings[4] = {2250.0,1150.0,450.0,75.0};//in microsecDelay if this changes need to adjust updateScreen()

/*  -- Default Parameters -- */
float mLBolus = 300.00; //default bolus size in mL
int SpeedSettingsIdx = 3; //microsecDealy default array SpeedSetting begins with item idex 0 the 2250.0 microsec delay;
float SpeedSetting = SpeedSettings[SpeedSettingsIdx];// calculates to 75microsecDelay or 300mL/min

long stepperPos = 0; //in microsteps
char charBuf[16];

//menu stuff
int uiState = MAIN;
//
//===================================================================end variable definitions================================================================================



void setup() {//==========================================================void setup()========================================================================================
 
  /* LCD setup */  
  lcd.begin(16, 2);
  lcd.clear();

  lcd.print("SyrPumpV5.4_3");


  /* Motor Setup */ 
  pinMode(motorDirPin, OUTPUT);
  pinMode(motorStepPin, OUTPUT);
  
  uint8_t i=0;//uint8_t is a unsigned integer on 8 bits
}//=====================================================================end void setup()==============================================================================


void loop() {//========================================================== void loop()====================================================================================
  
   //check for LCD updates
  readKey();

    
}//====================================================================end void loop()======================================================================================
//


//=============================================================subroutines==================================================================================================
//
//fUNCT DEF TO move motor (tried placing at bottom of code caused error need to be defined before use.
//
//
 void bolus(int direction){//=======================================bolus() subroutine===
        //Move stepper. Will not return until stepper is done moving.        
  
 //change units to steps
  long steps = (mLBolus * ustepsPerML);
  if(direction == PUSH){
                digitalWrite(motorDirPin, HIGH);
                steps = mLBolus * ustepsPerML;
  }
  else if(direction == PULL){
                digitalWrite(motorDirPin, LOW);

  } 

      float usDelay = SpeedSetting;
    
      for(long i=0; i < steps; i++){ 
        digitalWrite(motorStepPin, HIGH); 
        delayMicroseconds(usDelay); 
    
        digitalWrite(motorStepPin, LOW); 
        delayMicroseconds(usDelay); 
      } 

}//====================================================================end bolus()=====
//
void doKeyAction(unsigned int key){//============================== doKeyAction()======
  
  if(key == KEY_NONE){
        return;
    }//if(key == KEY_NONE)
//==========================
  if(key == 1){//1=Select

           if(uiState == MAIN){
                  uiState = SPEED_MENU;
           }
           else if(SPEED_MENU){
                  uiState = MAIN;
          }
  }// end if (key==1)
//=============================
  if(uiState == MAIN){
//=============================read all the button    
          uint8_t buttons = lcd.readButtons();// reads all buttons
                    if (buttons) {
                             if (buttons & BUTTON_UP) {
                                       mLBolus += mLBolusStep;
                                       lcd.setBacklight(RED);
                              }//===== end if (buttons & BUTTON_UP)
                             if (buttons & BUTTON_DOWN) {
                                
                                       if((mLBolus - mLBolusStep) > 0){
                                              mLBolus -= mLBolusStep;
                                       }
                                       else{
                                              mLBolus = 0;
                                        }
                                        lcd.setBacklight(RED);
                             }//===== end if (buttons & BUTTON_DOWN) 
                             if (buttons & BUTTON_LEFT) {
                                 // for testing only
                                       lcd.clear();
                                       lcd.setCursor(0,0);
                                       lcd.print(" pull bolus ");
                                 // 
                                        bolus(PULL);
                                        lcd.setBacklight(RED);
                                 //       exit(0);// testing only stop execution so i can see message
                             }//====== end if (buttons & BUTTON_LEFT)
                             if (buttons & BUTTON_RIGHT) {
                                 // for testing only
                                        lcd.clear();
                                        lcd.setCursor(0,0);
                                        lcd.print("push bolus ");
                                 // 
                                        bolus(PUSH);
                                        lcd.setBacklight(RED);
                                  //      exit(0);// for testing only stop execution so i can see message
                             }//======  end if (buttons & BUTTON_RIGHT)
                    }//=====end if (buttons)
  }//===if(uiState == MAIN)

  else if(uiState == SPEED_MENU){
        uint8_t buttons = lcd.readButtons();// reads all buttons but define only ones needed
               if (buttons) {
                          if (buttons & BUTTON_UP) {
                                  if(SpeedSettingsIdx < SpeedSettingsLength-1){
                                          SpeedSettingsIdx++;
                                          SpeedSetting = SpeedSettings[SpeedSettingsIdx];
                                  }
                                  lcd.setBacklight(RED);
                          }//====if (buttons & BUTTON_UP)
       
                          if (buttons & BUTTON_DOWN) {
                                    if(SpeedSettingsIdx > 0){
                                             SpeedSettingsIdx -= 1;
                                             SpeedSetting = SpeedSettings[SpeedSettingsIdx];
                                    }//end if(SpeedSettingsIdx
                                    lcd.setBacklight(RED);
                          }// end button& buttondown
             
              }//end if (buttons) speed menu
  

   }//-===end else if(uiState == SPEED_MENU)
 updateScreen();//
}// end dokeyAction
//===============================================================end doKeyAction()====

void updateScreen(){//============================================updateScreen()=======
  //convert speed delay settings to mL/min values
  if(SpeedSetting == 2250.0){//microSecDelay
    SpeedDisplay = 10.0;//mL/min
  }
  else if(SpeedSetting == 1150.0){//microSecDelay
    SpeedDisplay = 20.0;//mL/min
  }
  else if(SpeedSetting == 450.0){//microSecDelay
    SpeedDisplay = 50.0;//mL/min
  }
  else if(SpeedSetting == 75.0){//microSecDelay
   SpeedDisplay = 300.0;//mL/min
  }
  
  //build strings for upper and lower lines of screen
  String s1; //upper line
  String s2; //lower line
  
  if(uiState == MAIN){
    s1 = (String("Speed: ") + String(SpeedDisplay, 0) + String("mL/min"));
    s2 = (String("Volume: ") + String(mLBolus, 0) + String(" mL"));   
  }
  else if(uiState == SPEED_MENU){
    s1 = String("Menu> Speed Set");
    s2 = (String(SpeedDisplay, 0) + String("mL/min"));
  }

  //do actual screen update
  lcd.clear();

  s2.toCharArray(charBuf, 16);
  lcd.setCursor(0, 1);  //line=2, x=0
  lcd.print(charBuf);
  
  s1.toCharArray(charBuf, 16);
  lcd.setCursor(0, 0);  //line=1, x=0
  lcd.print(charBuf);
}//==================================================end updateScreen()=======================
//
String decToString(float decNumber){//====================decToString()=====================
  //not a general use converter! Just good for the numbers we're working with here.
  int wholePart = decNumber; //truncate
  int decPart = round(abs(decNumber*1000)-abs(wholePart*1000)); //3 decimal places
        String strZeros = String("");
        if(decPart < 10){
          strZeros = String("00");
        }  
        else if(decPart < 100){
          strZeros = String("0");
        }
  return String(wholePart) + String('.') + strZeros + String(decPart);
}// ========================================================end decToString()=================
//
void readKey(){//======================================================readKey()==============


// new board no need to debounce  read keys:=========

uint8_t buttons = lcd.readButtons();// reads all buttons
 
  if (buttons) {//=================
    if (buttons & BUTTON_UP) {
        lcd.setBacklight(RED);
    }
    if (buttons & BUTTON_DOWN) {
        lcd.setBacklight(RED);
    }
    if (buttons & BUTTON_LEFT) {
       lcd.setBacklight(RED);
    }
    if (buttons & BUTTON_RIGHT) {
       lcd.setBacklight(RED);
    }
    if (buttons & BUTTON_SELECT) {
       lcd.setBacklight(RED);
    }
    
 //=============================which key selected?==========
 //
                    lcd.clear();
                    lcd.setCursor(0,0);
                 
                    lcd.print(buttons);// this reads diff values for each key selected "works"
//testing that numerical values for buttons can be used in loop
//                        if(buttons==1){ lcd.print("  Select");}
//                        if(buttons==2){ lcd.print("  Right");}
//                        if(buttons==4){ lcd.print("  down");}
//                        if(buttons==8){ lcd.print("  UP");}
//                        if(buttons==16){ lcd.print(" Left");}
                        //
                        if(buttons==1){ key=1;}
                        if(buttons==2){ key=2;}
                        if(buttons==4){ key=4;}
                        if(buttons==8){ key=8;}
                        if(buttons==16){key=16;}
                    
                    //button=1(Select);2(right);4(down);8(up);16(left)                        
  doKeyAction(key);//
  }//==============end if (buttons)// this section reads buttons
  
      
  
}//================================================ end readKey()===
//
//============================================================End Subroutines=================================================================================================
