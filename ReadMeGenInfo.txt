ReadMe
1. final syringe pump program old board multiple selectable speeds in mL/min:syringePump300mL_revisedV2
2. final syringe pump program old board single spleed 50ml/min:syringePump300mLvs3
3. final syringe pump program New board multiple selectable speeds in mL/min:syringePump300mL_revised_NewBoardMR4_3

Orig programs received from Hans and Rob

older program:
  	SyringePump300ml.ino  
  	opening menu that says: Syringe pump v2.0
	has hardcoded single speed using the SPEED_MICROSECOND_DELAY  variable  
		SPEED_MICROSECOND_DELAY : 	a)30   for 400 mL/min ????
						b)450  for 50 mL/min
						c)2250 for 10 mL/min


Newer program:
	SyringePump300ml_revised.ino
	opening menu that says: Syringe pump
	adds variable speed control

Note:
 the pump i got from  Alex had  the old program running on it.  
Rob said the New Program should run on the old hardware

To send prog to arduino coonect straight to usb on computer not trough multi usb hub


Stepper Motor:
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
sample code: http://www.schmalzhaus.com/EasyDriver/Examples/EasyDriverExamples.html

void setup() {                
  pinMode(8, OUTPUT);
  pinMode(9, OUTPUT);
  digitalWrite(8, LOW);
  digitalWrite(9, LOW);
}el

void loop() {
  digitalWrite(9, HIGH);
  delay(1);          
  digitalWrite(9, LOW); 
  delay(1);          
}
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

1.8deg/step so 200 full steps per revolution.  Big Easy Diver board defaults to 16 microsteps so total of 3200 microStep per revolution

Motor Speed:
delay() in ms since the code above uses 1ms High and 1 ms Low you each microstep takes 2ms
delayMicroseconds() in microSec eg 10microSec h ad 10 microSec low  takes 20 microSec per microStep

delay(1):	microStep/2ms*1000ms/sec = 500microStep/sec

delay(10):	microStep/20ms*1000ms/sec= 50microStep/sec

delayMicrosec(50):	microStep/100microsec*1000microsec/msec*1000msec/sec= 10,000microstep/sec

Speed:
syringe vol=300mL
syringe Barrel lenght=155mm
Thread Pitch=1.25mm  this means each ful 360 deg rev the rod avances 1.25mm
Big Easy Driver board defaults to 16 microsteps so full 360 deg rev =200step/rev*16microStep/step=3200 microstep per rev

=
300ml/155mm*1.25mm/rev*rev/3200microstep=0.000756 ml/microStep

syringeVol/syringbarrelLenght *ThredPitch/rev*rev/3200microstep= ml/microStepcalc

flow rate wanted

dealyMicrosec(50):		0.000756ml/microStep*10,000microstep/sec*60sec/min= 453 ml/min fo 50 microsec Delay
see spreadsheet Flowrate calculation syringe pump 2020	


+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

TRIGGERING EXAMPLES:

int triggerPin=9; //not actually connected just a way to create a trigger
pinMode(triggerPin, Input);

Void set up(){}

Void Loop(){
	digitalWrite(triggePin, HIGH);
	//  other code
	if(digitalRead(triggerPin)==HIGH); {
		//do this code so the trigger Pin triggered this code
	}// end if loop
	//otherwise do this
	}//end void loop



+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Function that returns values rather than a void function that does not return anything

Example:
int myMultiplyFunction(int x, int y){  // returns and integer(int); name of function: myMultiplyFunction; values passed: int x, int y
  int result;
  result = x * y;
  return result;// result is an integer returned to code that called the function
}

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

User defined Functions created in this program:
Funtion Name			void or parameter to pass
1. ckTrigger()			void
2.readSerial()			void
3.processSerial()		void
4.bolus(Direction)		pass direction( PUSH or PULL)
5.readKey()			void
6.doKeyAction(key)		Pass key
7.updateScreen()		void
8.getKey(input)			pass input
9.decToString(decNumber)	pass decNumber
	 