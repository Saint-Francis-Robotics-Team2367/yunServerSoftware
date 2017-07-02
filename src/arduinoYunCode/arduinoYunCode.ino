#include <Servo.h>
#include <string.h>

//uncomment to enable hardware serial debug messages
#define DEBUG 0 

#define DELIMS "#:!?"

#define LED_FAST_DELAY 50
#define LED_SLOW_DELAY 300
#define MOTOR_WATCHDOG_DELAY 200
#define MOTOR_IDLE 93
#define MOTOR_MAX 190
#define MOTOR_MIN 0
#define L_STICK_DEADZONE 17
#define R_STICK_DEADZONE 17
#define MIN_STRING 10

#define MTR1_SIG 5  
#define MTR1_POS 6
#define MTR1_GND 7

#define MTR2_SIG 2
#define MTR2_POS 3
#define MTR2_GND 4

//uncomment next line to invert left axis
//#define INVERT_LEFT_AXIS 1

//uncomment next line to inver right axis
//#define INVERT_RIGHT_AXIS 1

char data_read[32];
int ledDelay = LED_SLOW_DELAY;
bool ledState =true;
bool sockStat = true;
bool updateOutputs = false;
Servo mtr1, mtr2;
long ledTimeout, motorWatchdog;

byte oneVal;
byte LStickX;
byte RStickY;

void setup() {
	Serial1.begin(115200); // Set the baud.
	while (!Serial1)
	{
	}
	Serial.begin(115200);
	pinMode(MTR1_SIG, OUTPUT);
	pinMode(MTR1_POS, OUTPUT);
	pinMode(MTR1_GND, OUTPUT);

	pinMode(MTR2_SIG, OUTPUT);
	pinMode(MTR2_POS, OUTPUT);
	pinMode(MTR2_GND, OUTPUT);

	digitalWrite(MTR1_POS, HIGH);
	digitalWrite(MTR1_GND, LOW);
	digitalWrite(MTR2_POS, HIGH);
	digitalWrite(MTR2_GND, LOW);

	mtr1.attach(MTR1_SIG);
	mtr2.attach(MTR2_SIG);
	mtr1.write(MOTOR_IDLE);
	mtr2.write(MOTOR_IDLE);

	digitalWrite(13, LOW); //have LED start in off state
	ledTimeout = millis() + ledDelay;
	motorWatchdog = millis();
}

void loop() {
	if (Serial1.available())
	{
		memset(data_read, 0, 32);
		Serial1.readBytesUntil('?', data_read, 32);

		//check to weed out garbage data on start-up, if reasonable data size between delimiters then accept packet 
		if(strstr(data_read, "ROB#"))
		{
			ledDelay = LED_SLOW_DELAY;
			updateOutputs = true;
		}	
	}


	if (updateOutputs)
	{
		updateOutputs = false;
		strtok(data_read, "#");
		char *lStick = strtok(NULL, DELIMS);
		char *rStick = strtok(NULL, DELIMS);
		LStickX   = map(atoi(lStick), 0, 255, MOTOR_MIN, MOTOR_MAX);
		RStickY = map(atoi(rStick), 0, 255, MOTOR_MIN, MOTOR_MAX);

		#ifdef DEBUG
			Serial.print(LStickX);
			Serial.print(" ");
			Serial.println(RStickY);
		#endif


		int lftMtr = MOTOR_IDLE;
		int rghtMtr = MOTOR_IDLE;

    if(LStickX > MOTOR_IDLE + L_STICK_DEADZONE)
    {
       #ifdef INVERT_LEFT_AXIS
        rghtMtr = map(LStickX, MOTOR_IDLE + L_STICK_DEADZONE,  MOTOR_MAX,  MOTOR_IDLE,  MOTOR_MIN); //right motor is inverted
        lftMtr =  map(LStickX, MOTOR_IDLE + L_STICK_DEADZONE,  MOTOR_MAX,  MOTOR_IDLE,  MOTOR_MAX);   
      #else
        lftMtr =  map(LStickX, MOTOR_IDLE + L_STICK_DEADZONE,  MOTOR_MAX,  MOTOR_IDLE,  MOTOR_MIN); //left motor is inverted
        rghtMtr = map(LStickX, MOTOR_IDLE + L_STICK_DEADZONE,  MOTOR_MAX,  MOTOR_IDLE,  MOTOR_MAX);  
      #endif
    }
    else if(LStickX < MOTOR_IDLE - L_STICK_DEADZONE)
    {
       #ifdef INVERT_LEFT_AXIS
        lftMtr =  map(LStickX, MOTOR_IDLE - L_STICK_DEADZONE,  MOTOR_MIN,  MOTOR_IDLE,  MOTOR_MIN); //left motor is inverted
        rghtMtr = map(LStickX, MOTOR_IDLE - L_STICK_DEADZONE,  MOTOR_MIN,  MOTOR_IDLE,  MOTOR_MAX);  

      #else
        rghtMtr = map(LStickX, MOTOR_IDLE - L_STICK_DEADZONE,  MOTOR_MIN,  MOTOR_IDLE,  MOTOR_MIN); //right motor is inverted
        lftMtr =  map(LStickX, MOTOR_IDLE - L_STICK_DEADZONE,  MOTOR_MIN,  MOTOR_IDLE,  MOTOR_MAX);   
      #endif
    }

    if(RStickY > MOTOR_IDLE + R_STICK_DEADZONE)
    {
      int turnRate = map(RStickY, MOTOR_IDLE + R_STICK_DEADZONE, MOTOR_MAX, MOTOR_IDLE, MOTOR_MAX);
      #ifdef INVERT_RIGHT_AXIS
        rghtMtr -= turnRate - MOTOR_IDLE;
        lftMtr -= turnRate - MOTOR_IDLE;
      #else
        lftMtr += turnRate - MOTOR_IDLE;
        rghtMtr += turnRate - MOTOR_IDLE;
      #endif
    }
    else if(RStickY < MOTOR_IDLE - R_STICK_DEADZONE)
    {
      int turnRate = map(RStickY, MOTOR_IDLE - R_STICK_DEADZONE, MOTOR_MIN, MOTOR_IDLE, MOTOR_MIN);
      #ifdef INVERT_RIGHT_AXIS
        rghtMtr -= turnRate - MOTOR_IDLE;
        lftMtr -= turnRate - MOTOR_IDLE;
      #else
        lftMtr += turnRate - MOTOR_IDLE;
        rghtMtr += turnRate - MOTOR_IDLE;
      #endif
    }

		mtr1.write(lftMtr);
		mtr2.write(rghtMtr);
		motorWatchdog = millis();
	}

	if (motorWatchdog + MOTOR_WATCHDOG_DELAY < millis())
	{
		mtr1.write(MOTOR_IDLE);
		mtr2.write(MOTOR_IDLE);
		motorWatchdog = millis();
		ledTimeout = LED_FAST_DELAY;
		#ifdef DEBUG
		  Serial.println("watchdog not feed");
		#endif
	}

	if (ledTimeout < millis() && sockStat)
	{
		ledTimeout = millis() + ledDelay;
		if (ledState)
		{
			digitalWrite(13, LOW);
			ledState = false;
		}
		else
		{
			digitalWrite(13, HIGH);
			ledState = true;
		}
	}
}
