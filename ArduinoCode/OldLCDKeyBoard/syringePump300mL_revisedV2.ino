//syringePump300mL_revisedV2 MR remove all code not used has selectable speed setting  commented Maritza Romero 7/14/2020
// Final working code for 300mL syringe with speed settings menu in mL/min for use with pumps using lcd keypad from Sparkfun

//MR this code based on Naroom Open Syringe Pump code :https://github.com/naroom/OpenSyringePump/blob/master/syringePump/syringePump.ino
//MR uses the key LCD_Button Shield VS1 board from sparkfun (obsolete now):https://www.sparkfun.com/products/retired/13293
//MR uses BigEasyDriver board to control stepper motor:https://www.sparkfun.com/products/12859

//7/14/20 Hans asked to add 20mL/min and change 400mL/min to 300mL/min

//   SpeedSetting in microSecDelay      Speed Display in mL/min
//          2250                              10    tested with timer
//          1150                              20    tested with timer
//           450                              50   tested with timer
//            75                             300  tested with timer
//  

#include <LiquidCrystal.h>
#include <LCDKeypad.h>

/* -- Constants -- */
#define SYRINGE_VOLUME_ML 300.0
#define SYRINGE_BARREL_LENGTH_MM 155.0

#define THREADED_ROD_PITCH 1.25
#define STEPS_PER_REVOLUTION 200.0
#define MICROSTEPS_PER_STEP 16.0 //big easy driver defaults to 16 microsteps not 8

#define SPEED_MICROSECONDS_DELAY 450 //longer delay = lower speed (450 = 50mL/min for 300mL syringe) (2250 = 10mL/min for 300mL syringe)

#define mLBolusStep 10.0

float SpeedDisplay = 50.0;// in mL/min

long ustepsPerML = (MICROSTEPS_PER_STEP * STEPS_PER_REVOLUTION * SYRINGE_BARREL_LENGTH_MM) / (SYRINGE_VOLUME_ML * THREADED_ROD_PITCH );

/* -- Pin definitions -- */
int motorDirPin = 2;
int motorStepPin = 3;

/* -- Keypad states -- */
int adc_key_val[5] ={622, 822, 863, 913, 940};// measured voltages at the board

enum{KEY_SELECT, KEY_RIGHT, KEY_LEFT, KEY_DOWN, KEY_UP, KEY_NONE};
int NUM_KEYS = 5;
int adc_key_in;
int key = KEY_NONE;

/* -- Enums and constants -- */
enum{PUSH,PULL}; //syringe movement direction
enum{MAIN, SPEED_MENU}; //UI states

const int SpeedSettingsLength = 4;
float SpeedSettings[4] = {2250.0,1150.0,450.0,75.0};//in microsecDelay if this changes need to adjust updateScreen()

/*  -- Default Parameters -- */
float mLBolus = 300.00; //default bolus size
int SpeedSettingsIdx = 3; //microsecDealy default array SpeedSetting begins with item idex 0 the 2250.0 microsec delay;index 1 1150.0microsecDelay; index 2  450microsecDelay
float SpeedSetting = SpeedSettings[SpeedSettingsIdx];// calculates to 450microsecDelay


long stepperPos = 0; //in microsteps
char charBuf[16];

//debounce params
long lastKeyRepeatAt = 0;
long keyRepeatDelay = 400;
long keyDebounce = 125;
int prevKey = KEY_NONE;
        
//menu stuff
int uiState = MAIN;

/* -- Initialize libraries -- */
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);

void setup(){//=================================================void set up========================================================================================
  /* LCD setup */  
  lcd.begin(16, 2);
  lcd.clear();

  lcd.print("SyringePumpV4");


  /* Motor Setup */ 
  pinMode(motorDirPin, OUTPUT);
  pinMode(motorStepPin, OUTPUT);
  

}//================================================================close void set up loop (runs only once)===============================================================

void loop(){//=======================================================void loop=============================================================================================
  //check for LCD updates
  readKey();

}//=====================================================================close void loop==================================================================================


void bolus(int direction){//=======================================bolus() subroutine=========================
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

}//====================================================================end bolus()==============================

void readKey(){//======================================================readKey()================================
	//Some UI niceness here. 
        //When user holds down a key, it will repeat every so often (keyRepeatDelay).
        //But when user presses and releases a key, 
        //the key becomes responsive again after the shorter debounce period (keyDebounce).

	adc_key_in = analogRead(0);
	key = get_key(adc_key_in); // convert into key press

	long currentTime = millis();
        long timeSinceLastPress = (currentTime-lastKeyRepeatAt);
        
        boolean processThisKey = false;
	if (prevKey == key && timeSinceLastPress > keyRepeatDelay){
          processThisKey = true;
        }
        if(prevKey == KEY_NONE && timeSinceLastPress > keyDebounce){
          processThisKey = true;
        }
        if(key == KEY_NONE){
          processThisKey = false;
        }  
        
        prevKey = key;
        
        if(processThisKey){
          doKeyAction(key);
  	  lastKeyRepeatAt = currentTime;
        }
}//================================================================ end readKey()======================================

void doKeyAction(unsigned int key){//============================== doKeyAction()=====================================
	if(key == KEY_NONE){
        return;
    }

	if(key == KEY_SELECT){
		if(uiState == MAIN){
			uiState = SPEED_MENU;
		}
		else if(SPEED_MENU){
			uiState = MAIN;
		}
	}

	if(uiState == MAIN){
		if(key == KEY_LEFT){
			bolus(PULL);
		}
		if(key == KEY_RIGHT){
			bolus(PUSH);
		}
		if(key == KEY_UP){
			mLBolus += mLBolusStep;
		}
		if(key == KEY_DOWN){
			if((mLBolus - mLBolusStep) > 0){
			  mLBolus -= mLBolusStep;
			}
			else{
			  mLBolus = 0;
			}
		}
	}
	else if(uiState == SPEED_MENU){
		if(key == KEY_LEFT){
			//nothin'
		}
		if(key == KEY_RIGHT){
			//nothin'
		}
		if(key == KEY_UP){
			if(SpeedSettingsIdx < SpeedSettingsLength-1){
				SpeedSettingsIdx++;
				SpeedSetting = SpeedSettings[SpeedSettingsIdx];
			}
		}
		if(key == KEY_DOWN){
			if(SpeedSettingsIdx > 0){
				SpeedSettingsIdx -= 1;
				SpeedSetting = SpeedSettings[SpeedSettingsIdx];
			}
		}
	}

	updateScreen();
}//===============================================================end doKeyAction()=============================================

void updateScreen(){//============================================updateScreen()================================================
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
}//==================================================end updateScreen()=====================================================


// Convert ADC value to key number
int get_key(unsigned int input){//=========================get_key()======================================================
  int k;
  for (k = 0; k < NUM_KEYS; k++){
    if (input < adc_key_val[k]){
      return k;
    }
  }
  if (k >= NUM_KEYS){
    k = KEY_NONE;     // No valid key pressed
  }
  return k;
}//=======================================================end get_key()=======================================================

String decToString(float decNumber){//====================decToString()======================================================
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
}// ========================================================end decToString()============================================================
