
/******************************************************
*
* Created: Mar 29, 2016
* Author: Tlee
*
*******************************************************/

#include <Wire.h>

#define LED_PIN   13
#define BUZZ_PIN  15
#define BLINK(pin)  ((digitalWrite(pin, !digitalRead(pin))))

//ROL = x, PIT = y, YAW = z
#define ROL 0
#define PIT 1
#define YAW 2

#define SAMPLES 4000

bool calibrated = false;
bool gyroInitState = false;

double gyroRaw[3];
double gyroRate[3];
double gyroCal[3];

//------------------------------------------------------
//  func: Init GYRO
//  inp: none
//  out: none return but GYRO is read to work
//  note: this also inits I2C for data transmition
//------------------------------------------------------
void gyroInit()
{
	Serial.println();
	Serial.println("-gyroInit :: I2C init");
	Wire.begin();
	digitalWrite(LED_PIN,HIGH);
	delay(1500);

	Serial.println("-gyroInit :: turn on the gyro and enable all axises");
	Wire.beginTransmission(0x69);
	Wire.write(0x20);
	Wire.write(0x0F);
	Wire.endTransmission();

	Wire.beginTransmission(0x69);
	Wire.write(0x23);
	Wire.write(0x90);
	Wire.endTransmission();
	gyroInitState = true;
	Serial.println("-gyroInit :: init OK");
}

//------------------------------------------------------
//  func: gyroCalibration
//  inp: gobal data gyroRaw data is ready.
//  out: it takes SAMPLES and then calculates the average
//    value for each axis.
//  note: this also inits I2C for data transmition
//------------------------------------------------------
void gyroCalibration()
{
	if (gyroInitState == false) {
		Serial.println();
		Serial.println("-gyroCalibration :: Not yet init Gyro");
		gyroInit();
	}

	Serial.println();
	Serial.print("-gyroCalibration :: Collecting samples: ");
	Serial.println(SAMPLES);

	for (int i=0; i<SAMPLES ; i++) {
		gyroReadRaw();
		gyroCal[ROL] += gyroRaw[ROL];
		gyroCal[PIT] += gyroRaw[PIT];
		gyroCal[YAW] += gyroRaw[YAW];
		delay(1);
		if(i % 100 == 0) BLINK(LED_PIN);
	}

	gyroCal[ROL] /= SAMPLES;
	gyroCal[PIT] /= SAMPLES;
	gyroCal[YAW] /= SAMPLES;

	calibrated = true;
	Serial.println("-gyroCalibration :: gyroCalibration OK");
}

//------------------------------------------------------
//  func: gyroReadRaw
//  inp: none
//  out: Raw data of L3G are returned
//------------------------------------------------------
void gyroReadRaw()
{
	byte highByte, lowByte;
	Wire.beginTransmission(0x69);
	Wire.write(168);
	Wire.endTransmission();

	Wire.requestFrom(0x69, 6);
	while(Wire.available() < 6);

	lowByte = Wire.read();
	highByte = Wire.read();
	gyroRaw[ROL] = ((highByte<<8)|lowByte);

	lowByte = Wire.read();
	highByte = Wire.read();
	gyroRaw[PIT] = ((highByte<<8)|lowByte);
	gyroRaw[PIT] = - gyroRaw[PIT];

	lowByte = Wire.read();
	highByte = Wire.read();
	gyroRaw[YAW] = ((highByte<<8)|lowByte);
	gyroRaw[YAW] = -gyroRaw[YAW];

	if(calibrated) {
		gyroRaw[ROL] -= gyroCal[ROL];
		gyroRaw[PIT] -= gyroCal[PIT];
		gyroRaw[YAW] -= gyroCal[YAW];
	}
}

//------------------------------------------------------
//  func: gyroCaculate
//  inp: gyroRaw is ready
//  out: gyroRate are returned
//------------------------------------------------------
void gyroCaculate()
{
	gyroRate[ROL] = (gyroRate[ROL] * 0.8) + ((gyroRaw[ROL] / 57.14286) * 0.2);
	gyroRate[PIT] = (gyroRate[PIT] * 0.8) + ((gyroRaw[PIT] / 57.14286) * 0.2);
	gyroRate[YAW] = (gyroRate[YAW] * 0.8) + ((gyroRaw[YAW] / 57.14286) * 0.2);
}

//------------------------------------------------------
//  func: System setup
//  inp: none
//  out: System is ready
//------------------------------------------------------
void setup()
{
	Serial.begin(9600);
	Serial.println();
  Serial.println();
	Serial.println("------ L3G Demo /Initialize starting ------");
	gyroInit();
	gyroCalibration();
	Serial.println("------ L3G Demo /Initialized OK ------");
}

//------------------------------------------------------
//  func: main loop
//------------------------------------------------------
void loop()
{
	gyroReadRaw();
	gyroCaculate();

	//for easy reading, I just print X-Axis
	Serial.print("X-Axis of Rotation :");
	Serial.println(gyroRate[ROL]);

	//TODO:
	//	User can print more as their wish.
	//	Serial.print("Y-Axis of Rotation :");
	//	Serial.println(gyroRate[PIT]);
	//	Serial.print("Z-Axis of Rotation :");
	//	Serial.println(zGyro);
}
