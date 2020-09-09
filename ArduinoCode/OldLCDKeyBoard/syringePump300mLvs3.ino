//final program  set speed
//syringePump300mLvs3:  7/14/2020 removed from code perceived as extraneous (like serial) working!!!!

//hard coded speed of syringe to 50 mL/min  450 microsecDelay confirmed with timer

//Original program adjuste from Naroom code by Rob Bruton 2019
//MR comments added by Maritza Romero July/2020

//MR this code based on Naroom Open Syringe Pump code :https://github.com/naroom/OpenSyringePump/blob/master/syringePump/syringePump.ino
//MR uses the key LCD_Button Shield VS1 board from sparkfun (obsolete now):https://www.sparkfun.com/products/retired/13293
//MR uses BigEasyDriver board to control stepper motor:https://www.sparkfun.com/products/12859

// Controls a stepper motor via an LCD keypad shield.

// To run, you will need the LCDKeypad library installed - see libraries dir.

//   MR Everything declared in this section is a global variable seen everywhere

#include <LiquidCrystal.h>
#include <LCDKeypad.h>


/* -- Constants -- */
#define SYRINGE_VOLUME_ML 300.0// MR max cc on syringe
#define SYRINGE_BARREL_LENGTH_MM 155.0//MR measured lenght from o to 300 cc on barrel

#define THREADED_ROD_PITCH 1.25  // MR  metric thread 1.25 mm between thread peaks advances 1.25mm/ Rev 
#define STEPS_PER_REVOLUTION 200.0 //MR info provided by the Big Easy Driver board documentatioin
#define MICROSTEPS_PER_STEP 16.0 //MR big easy driver defaults to 16 microsteps not 8 it is 8 if using the easy driver board

#define SPEED_MICROSECONDS_DELAY 450 // (450 = 50mL/min for 300mL syringe) (2250 = 10mL/min for 300mL syringe)

long ustepsPerML = (MICROSTEPS_PER_STEP * STEPS_PER_REVOLUTION * SYRINGE_BARREL_LENGTH_MM) / (SYRINGE_VOLUME_ML * THREADED_ROD_PITCH );

/* -- Pin definitions -- */
int motorDirPin = 2; //MR where motor Direction is connected: int=integer;motorDirPin is name of variable;2 is the pin that was assigned to that variable
int motorStepPin = 3;//MR where Step command is connected

/* -- Keypad states -- */
int adc_key_val[5] ={622, 822, 863, 913, 940};//MR measured voltages returned by button in array form given by Rob depends on what board you use

enum{KEY_SELECT, KEY_RIGHT, KEY_LEFT, KEY_DOWN, KEY_UP, KEY_NONE};//MR order the key values are give key_select=0, Key_right=1...
int NUM_KEYS = 5;
int adc_key_in;
int key = KEY_NONE;//MR reset key on keyboard

/* -- Enums and constants -- */
enum{PUSH,PULL}; //syringe movement direction
enum{MAIN, BOLUS_MENU}; //UI states when you press select

const int mLBolusStepsLength = 4;//MR number of  mLBolusSteps
float mLBolusSteps[4] = {0.500, 1.000, 5.000, 10.000};  // MR float data type eg: 3.5789874 has a decimal point; long similar without the decimal
//  array of four options to be selected of mLBolusStep ; where each mLBolusStep moves  by .5ml or 1.0ml or ml or 10 ml .

/* -- Default Parameters -- */
float mLBolus = 10.00; //default bolus size
float mLUsed = 0.0;
int mLBolusStepIdx = 3; //MR 10.0 mL increments at first Index of the array mLBolusSteps  where mLBolusSteps[0]==0.5;mLBolusSteps[3]==10.0
float mLBolusStep = mLBolusSteps[mLBolusStepIdx];//MR same as mLBolusStep[3]==10.0

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

// MR ===========================================SET UP loop runs only once===================================================================================

void setup(){
  /* LCD setup */  
  lcd.begin(16, 2);
  lcd.clear();

  lcd.print("SyringePumpV3 MR");//orig  prog


  /* Motor Setup */ 
  pinMode(motorDirPin, OUTPUT);
  pinMode(motorStepPin, OUTPUT);
  
    
}//  MR end void set up loop

//MR==========================================================begin Void loop()  repeats over and over======================================================================

void loop(){
  //check for LCD updates
  readKey();// MR call readKey function
  
}//  MR end Void loop()==========================================end voild loop=============================================================================================

//MR===================Subroutine aka Functions Section in Arduino ============================================================================

//==========================bolus function========================================

void bolus(int direction){//Move stepper. Will not return until stepper is done moving.  Pass direction either PUsh or PUll       
  
	//change units to steps
	long steps = (mLBolus * ustepsPerML);
	if(direction == PUSH){
                digitalWrite(motorDirPin, HIGH);
		steps = mLBolus * ustepsPerML;
		mLUsed += mLBolus; //equiv to mLUsed= mLUsed + mLBolus;
	}//MR  end if direction=Push loop
 
	else if(direction == PULL){
                digitalWrite(motorDirPin, LOW);
		if((mLUsed-mLBolus) > 0){
			mLUsed -= mLBolus; //mLUsed=mLUsed-mLBolus;
		}// MR end if (mLused-mLBolus) >0 loop
    
		else{
			mLUsed = 0;
		}//MR end else
    
	}// MR end else if (dir=Pull )loop 	

  float usDelay = SPEED_MICROSECONDS_DELAY; //can go down to 20 or 30
  

      for(long i=0; i < steps; i++){ // this loop is what actually mores the  motor at a rate of msDelay ck out Flowrate Calcuataion syringe pump.xls to see flow rate vs usec delay
        digitalWrite(motorStepPin, HIGH); 
        delayMicroseconds(usDelay); 
    
        digitalWrite(motorStepPin, LOW); 
        delayMicroseconds(usDelay); 
      } // MR end for( long) loop

}//======================================== MR end bolus function=======================
//
//======================================MR ReadKey function=============================

void readKey(){
  
	      //Some UI niceness here. 
        //When user holds down a key, it will repeat every so often (keyRepeatDelay).
        //But when user presses and releases a key, 
        //the key becomes responsive again after the shorter debounce period (keyDebounce).

	  adc_key_in = analogRead(0);// MR all keys  read by A0 reads voltage
	  key = get_key(adc_key_in); // MR call function get_key() pass it adc_key_in voltage; if valid key returns k=# of key or Key_None

	  long currentTime = millis();//MR long data type eg: 1,455,489
    long timeSinceLastPress = (currentTime-lastKeyRepeatAt);        
    boolean processThisKey = false;//MR dont process the key
   //
   // this section ck debounce parameters keydebounce=125 msec or keyRepeatDelay=400msec  
      
	  if (prevKey == key && timeSinceLastPress > keyRepeatDelay){
          processThisKey = true;
    }// MR end if prevKey=key&&
          
    if(prevKey == KEY_NONE && timeSinceLastPress > keyDebounce){
          processThisKey = true;
    }// MR end if prevKey==Key non &&...
    
    if(key == KEY_NONE){
          processThisKey = false;
    } // MR end if key== key none
        
    prevKey = key;// MR  key
        
    if(processThisKey){// MR if (condition){//statements}  evaluates condition and if true performs the statements
          doKeyAction(key);// call doKeyAction subroutine and pass key#
  	      lastKeyRepeatAt = currentTime;
    }// MR end if (processThisKey  loop
    
}//===========================MR end readKey function=================================
//
//===========================MR doKeyAction function===================================
void doKeyAction(unsigned int key){
  
	if(key == KEY_NONE){
        return;
    }// MR end if key=KeyNone

	if(key == KEY_SELECT){
		  if(uiState == MAIN){// MR  uiState==MAin set in Default section 
			        uiState = BOLUS_MENU;// select key pused and uiState==Main if second press of  select key not performed
		  }//MR  end if uiState=Main
     
		  else if(BOLUS_MENU){
			        uiState = MAIN;
		  }// MR end else if
    
	}// MR end if key=key_select

	if(uiState == MAIN){
		  if(key == KEY_LEFT){
			        bolus(PULL);
		  }
     
		  if(key == KEY_RIGHT){
			        bolus(PUSH);
		  }
     
		  if(key == KEY_UP){
			mLBolus += mLBolusStep;//MR syntax equivalent to mBolus= mLBolus+mLBolusStep;
		  }
     
		  if(key == KEY_DOWN){
			    if((mLBolus - mLBolusStep) > 0){
			       mLBolus -= mLBolusStep;//MR syntax equivalent to mLBolus=mLBolus -mLBolusStep
			    }// end if MLBolus- loop
          
			    else{ 
			       mLBolus = 0;
			   }// MR end else loop
        
		}// MR end if Key==Keydown loop
   
	}//MR end if uiState=Main loop
  
	else if(uiState == BOLUS_MENU){
	  	if(key == KEY_LEFT){
		    	//nothin'
		  }//MR end key=key left
    
		  if(key == KEY_RIGHT){
		    	//nothin'
	   	}//MR end key=key right loop
    
		  if(key == KEY_UP){
			    if(mLBolusStepIdx < mLBolusStepsLength-1){  //MR mLBolusStepIdx=3 and mLBolusStepLength=4 set in default section
				      mLBolusStepIdx++;//MR x++  :increment x by one and returns the old value of x  So if x=2 and y=x++ then x=2+1=3 and y=2
				      mLBolusStep = mLBolusSteps[mLBolusStepIdx];
			    }//MR end if mlBolusStepIDx loop
      
		  }// MR end if key=key up loop
   
		  if(key == KEY_DOWN){  
		    	if(mLBolusStepIdx > 0){
		      		mLBolusStepIdx -= 1;  //x -= y; // equivalent to the expression x = x - y;so mLBolusStepIdx=mLBolusStepIdx-1
			      	mLBolusStep = mLBolusSteps[mLBolusStepIdx];
		    	}// end if mLBolusStepIdx> loop
     
		   }//MR end if key= key down loop
   
	}//MR end  else if uiState==bolus menu loop

	updateScreen();  //MR call function updateScreen
}//========================== MR end DoKeyAction function========================================
//
//===========================MR updateScreen fucntion=============================================

void updateScreen(){
	//build strings for upper and lower lines of screen
	String s1; //upper line
	String s2; //lower line
	
	if(uiState == MAIN){
		s1 = String("Used ") + decToString(mLUsed) + String(" mL");
		s2 = (String("Bolus ") + decToString(mLBolus) + String(" mL"));		
	}// MR end if uiStat=main loop
  
	else if(uiState == BOLUS_MENU){
		s1 = String("Menu> BolusStep");
		s2 = decToString(mLBolusStep);
	}// MR end elseif loop

	//do actual screen update
	lcd.clear();

	s2.toCharArray(charBuf, 16);
	lcd.setCursor(0, 1);  //line=2, x=0
	lcd.print(charBuf);
	
	s1.toCharArray(charBuf, 16);
	lcd.setCursor(0, 0);  //line=1, x=0
	lcd.print(charBuf);
}//================================MR close update screen function======================
//
//=============================== getKey function retunrs an integer key number========
//
// from enum{KEY_SELECT, KEY_RIGHT, KEY_LEFT, KEY_DOWN, KEY_UP, KEY_NONE};// key_select==0 and key_None==5
// so valid key numbers are 0 thru 4
//
// Convert ADC value to key number k

int get_key(unsigned int input){
  
  int k;
  for (k = 0; k < NUM_KEYS; k++){
    if (input < adc_key_val[k]){
      return k;//  MR k is a valid key number return it
    }// MR end if loop of for loop
  }// MR end for loop
  
  if (k >= NUM_KEYS){
    k = KEY_NONE;     // No valid key pressed
  }// MR end if k> loop
  
  return k;
  
}// =============================MR end of get_key function return k========================
//
//==============================MR decToString function returns a string from a decimal===================

String decToString(float decNumber){       //not a general use converter! Just good for the numbers we're working with here. You passed it a decNumber
	int wholePart = decNumber; //truncate
	int decPart = round(abs(decNumber*1000)-abs(wholePart*1000)); //3 decimal places
        String strZeros = String("");
        
        if(decPart < 10){
          strZeros = String("00");
        }  // MR end if decPart<10
        
        else if(decPart < 100){
          strZeros = String("0");
        }// MR  end else if decPart
        
	return String(wholePart) + String('.') + strZeros + String(decPart);// recombine and return the number from the whole and decimal part
  
}//===========================MR end  decToSting ================================================
