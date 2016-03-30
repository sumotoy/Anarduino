/*
 * MCP7940RTC.h - library for MCP7940x Real Time Clock
 * 
 * Copyright (c) Anarduino.com
 * This library uses Arduino Time.h library functions
 *
 * The library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 * 
 * HISTORY:
 * ---------------------------------------------------------------------
 * 2011-10 - Initial hack...
 * 2013-12 - Added functions: setTimeRTC, getDateStr, loadDT
 * 
 * 
 * NOTE: this library is quite far from complete, if any of you have the time
 *	     and motivation to make a proper library, please also share it for the
 *	     benefit of all.   Thank you! 
 * 
 * TODO: There is much to do...  
 *  - add support for timer1
 *  - clean up the old rif-raff
 *  - deal with calibration and lots of other features
 *       
 */
#include "Arduino.h"
#include <Time.h>
#include <Wire.h>
#include "MCP7940RTC.h"
typedef uint8_t byte;

#define MCP7940_CTRL_ID 0x6F 
byte dt[7];

MCP7940RTC::MCP7940RTC() {
  byte b[8];
  Wire.begin();
  Wire.beginTransmission(MCP7940_CTRL_ID);
  Wire.write((byte)0x00);
  Wire.endTransmission();
  // Check status of oscillator, start it if not already running.
  Wire.requestFrom(MCP7940_CTRL_ID, 1);
  b[0] = Wire.read();
  if((b[0] & 0x80)==0) {      // Start oscillator
    // Start the oscillator
    //Serial.println("starting osc.");
    Wire.beginTransmission(MCP7940_CTRL_ID);
    b[1] = b[0] | 0x80;
    b[0] = 0;
    Wire.write(&b[0],2);
    Wire.endTransmission();
  }
  // Check status of VBAT, and ensure disabled
  // Check the status of VBAT, and disable it
  Wire.beginTransmission(MCP7940_CTRL_ID);
  Wire.write((byte)0x03);
  Wire.endTransmission();
  Wire.requestFrom(MCP7940_CTRL_ID, 1);
  b[0] = Wire.read();
  if((b[0] & 0x08)>0) {
    //Disable the VBAT mode
    //Serial.println("disabling vbat");
    Wire.beginTransmission(MCP7940_CTRL_ID);
    b[1] = b[0] |= 0x08;
    b[0] = 3;
    Wire.write(&b[0],2);
    Wire.endTransmission();
  }
  
}
  
// Load DateTime buffer, dt
void MCP7940RTC::loadDT() { 
    Wire.beginTransmission(MCP7940_CTRL_ID);
    Wire.write((uint8_t)0x00);			
	Wire.endTransmission();
	delay(1);
	Wire.requestFrom(MCP7940_CTRL_ID, 7);
	dt[0] = bcd2dec(Wire.read() & 0x7f);	// seconds
	dt[1] = bcd2dec(Wire.read() & 0x7f); 	// minutes
	dt[2] = bcd2dec(Wire.read() & 0x3f); 	// hours (assumes 24hr clock) TODO: read chip register status, determine properly
	dt[3] = bcd2dec(Wire.read() & 0x07);	// day of week, 1-7
	dt[4] = bcd2dec(Wire.read() & 0x3f);	// day of month, 01-31
	dt[5] = bcd2dec(Wire.read() & 0x1f);	// month, 01-12 TODO: Handle leap year from chip register
	dt[6] = bcd2dec(Wire.read());			// year, 00-99
}

// Set RTC time from time, t
void MCP7940RTC::setTimeRTC(time_t t) {
	tmElements_t tm;
	breakTime(t, tm);
	byte b[7];
    Wire.beginTransmission(MCP7940_CTRL_ID);
    Wire.write((uint8_t)0x00);			
	Wire.endTransmission();
	delay(1);
	Wire.requestFrom(MCP7940_CTRL_ID, 7);
	// load raw data, mask off date/time data elements
	b[0] = (Wire.read() & 0x80);	
	b[1] = (Wire.read() & 0x80);
	b[2] = (Wire.read() & 0xc0);
	b[3] = (Wire.read() & 0xf8);
	b[4] = (Wire.read() & 0xc0);
	b[5] = (Wire.read() & 0xe0);
	b[6] = 0;
	// now overlay the new time from tm
	b[0] |= dec2bcd(tm.Second & 0x7f);  // stop the oscillator and write the data);
	b[1] |= dec2bcd(tm.Minute);
	b[2] |= dec2bcd(tm.Hour);
	b[3] |= dec2bcd(tm.Wday);
	b[4] |= dec2bcd(tm.Day);
	b[5] |= dec2bcd(tm.Month);
	b[6] |= dec2bcd(tmYearToY2k(tm.Year));
    Wire.beginTransmission(MCP7940_CTRL_ID);
    Wire.write((uint8_t)0x00);			
	for(int i=0; i<7; i++) {
		Wire.write(b[i]);
		delay(1);
	}
	Wire.endTransmission();
	delay(10);
	// Now restart the oscillator
    Wire.beginTransmission(MCP7940_CTRL_ID);
    Wire.write(b[0] | 0x80);			// restart the oscillator
    delay(1);
    Wire.endTransmission();			
}

time_t MCP7940RTC::getTimeRTC() {
	loadDT();
	setTime((int)dt[2],(int)dt[1],(int)dt[0],(int)dt[4],(int)dt[5],(int)dt[6]);
	return now();
}

// Get Date string in format: YYYY-MM-DD HH:MM:SS W  - from RTC registers (W= Weekday)
void MCP7940RTC::getDateStr(char *dateStr) {
	loadDT();
	sprintf(dateStr,"%04d-%02d-%02d %02d:%02d:%02d %d", dt[6]+2000, dt[5], dt[4], dt[2], dt[1], dt[0], dt[3]);
}

// PUBLIC FUNCTIONS
time_t MCP7940RTC::get() { // Aquire data from buffer and convert to time_t
  tmElements_t tm;
  read(tm);
  return(makeTime(tm));
}

void  MCP7940RTC::set(time_t t) {
  tmElements_t tm;
  breakTime(t, tm);
  tm.Second &= 0x7f;  // stop the oscillator and write the data
  write(tm);
  uint8_t s = (tm.Second | 0x80); // assert oscillator start bit
  // Start the oscillator
  Wire.beginTransmission(MCP7940_CTRL_ID);
  Wire.write((uint8_t)0x00);			
  Wire.write((dec2bcd(tm.Second) | 0x80));							// Seconds
  Wire.endTransmission();
}

void MCP7940RTC::clearAlarm0() {
  Wire.beginTransmission(MCP7940_CTRL_ID);
  Wire.write((uint8_t)0x07);
  Wire.write((uint8_t)0);
  Wire.endTransmission();
}

void MCP7940RTC::setAlarm0(time_t t) {
  tmElements_t tm;
  breakTime(t, tm);
  Wire.beginTransmission(MCP7940_CTRL_ID);
  Wire.write((uint8_t)0x0a);			
  Wire.write((uint8_t)dec2bcd(tm.Second) & 0x7f);		// Seconds
  Wire.write((uint8_t)dec2bcd(tm.Minute) & 0x7f);		// Minutes
  Wire.write((uint8_t)dec2bcd(tm.Hour) & 0x3f);			// Hour
  Wire.write((uint8_t)(dec2bcd(tm.Wday) & 0x07) | 0x70);// wDay
  Wire.write((uint8_t)dec2bcd(tm.Day) & 0x3f);			// Day
  Wire.write((uint8_t)dec2bcd(tm.Month) & 0x1f);		// Hour
  Wire.endTransmission();
  delay(10);
// enable alarm 0
  Wire.beginTransmission(MCP7940_CTRL_ID);
  Wire.write((uint8_t)0x07);
  Wire.write((uint8_t)0x10);
  Wire.endTransmission();
}

// Aquire data from the RTC chip in BCD format
void MCP7940RTC::read(tmElements_t &tm) {
  uint8_t d[8];
  uint8_t dmask[] = { 0x7f, 0x7f, 0x3f, 0x07, 0x3f, 0x1f, 0xff };
  memset(d,0,8);
  Wire.beginTransmission(MCP7940_CTRL_ID);
  Wire.write((uint8_t)0x00); 
  Wire.endTransmission();

  // request the 7 data fields   (secs, min, hr, dow, date, mth, yr)
  Wire.requestFrom(MCP7940_CTRL_ID, tmNbrFields);
  tm.Second = bcd2dec((Wire.read() & 0x7f));   
  tm.Minute = bcd2dec((Wire.read() & 0x7f));
  byte b1 = Wire.read();
  byte b2 = b1 & 0x1f;
  if((b1 & 0x40)>0) 
	b2 |= 0x20;
  tm.Hour = bcd2dec(b1 & 0x3f);
  //tm.Hour =   bcd2dec(Wire.read() & 0x3f);  // mask assumes 24hr clock
  tm.Wday = bcd2dec((Wire.read() & 0x07));
  tm.Day = bcd2dec(Wire.read() & 0x3f );
  tm.Month = bcd2dec(Wire.read() & 0x1f);
  //tm.Year = bcd2dec(Wire.read()) + 30;//
  tm.Year = y2kYearToTm((bcd2dec(Wire.read())));
}

uint8_t MCP7940RTC::getSecond() {
 byte b=0;
  Wire.beginTransmission(MCP7940_CTRL_ID);
  Wire.write((byte)0);
  Wire.endTransmission();
  Wire.requestFrom(MCP7940_CTRL_ID, 1);
  byte b1 = Wire.read();
  b = bcd2dec(b1 & 0x7f);
  //b = (b1 & 0xf) + ((b1 & 0x70)>>4) * 10;
  return b;
}

uint8_t MCP7940RTC::getMinute() {
 byte b=0;
  Wire.beginTransmission(MCP7940_CTRL_ID);
  Wire.write((byte)1);
  Wire.endTransmission();
  Wire.requestFrom(MCP7940_CTRL_ID, 1);
  byte b1 = Wire.read();
  b = bcd2dec(b1 & 0x7f);
  //b = (b1 & 0xf) + ((b1 & 0x70)>>4) * 10;
  return b;
}

uint8_t MCP7940RTC::getHour() {
  //TODO: Check for 24-hr formation vs. am/pm, but for now, treat as 24-hr format due to init setting of same
  byte b=0;
  Wire.beginTransmission(MCP7940_CTRL_ID);
  Wire.write((byte)2);
  Wire.endTransmission();
  Wire.requestFrom(MCP7940_CTRL_ID, 1);
  byte b1 = Wire.read();
  byte b2 = b1 & 0x1f;
  if((b1 & 0x40)>0) b2 |= 0x20;
  b = bcd2dec(b1 & 0x3f);
  return b;
}

uint8_t MCP7940RTC::getDayOfWeek() {
  byte b=0;
  Wire.beginTransmission(MCP7940_CTRL_ID);
  Wire.write((byte)3);
  Wire.endTransmission();
  Wire.requestFrom(MCP7940_CTRL_ID, 1);
  byte b1 = Wire.read();
  byte b2 = b1 & 0x07;
  return b;
}

uint8_t MCP7940RTC::getDay() {
  byte b=0;
  Wire.beginTransmission(MCP7940_CTRL_ID);
  Wire.write((byte)4);
  Wire.endTransmission();
  Wire.requestFrom(MCP7940_CTRL_ID, 1);
  byte b1 = Wire.read();
  b = bcd2dec(b1 & 0x3f);
  return b;
}

uint8_t MCP7940RTC::getMonth() {
  byte b=0;
  Wire.beginTransmission(MCP7940_CTRL_ID);
  Wire.write((byte)5);
  Wire.endTransmission();
  Wire.requestFrom(MCP7940_CTRL_ID, 1);
  byte b1 = Wire.read();
  b = bcd2dec(b1 & 0x1f);
  return b;
}

uint8_t MCP7940RTC::getYear() {
  byte b=0;
  Wire.beginTransmission(MCP7940_CTRL_ID);
  Wire.write((byte)6);
  Wire.endTransmission();
  Wire.requestFrom(MCP7940_CTRL_ID, 1);
  byte b1 = Wire.read();
  return bcd2dec(b1);
}

void MCP7940RTC::write(tmElements_t &tm) {
  Wire.beginTransmission(MCP7940_CTRL_ID);
  Wire.write((uint8_t)0x00); // reset register pointer  
  Wire.write((uint8_t)dec2bcd(tm.Second) & 0x7f) ;   // Seconds
  Wire.write((uint8_t)dec2bcd(tm.Minute) & 0x7f);    // Minutes
  Wire.write((uint8_t)dec2bcd(tm.Hour) & 0x3f);     // sets 24 hour format
  Wire.write((uint8_t)dec2bcd(tm.Wday) & 0x07);   
  Wire.write((uint8_t)dec2bcd(tm.Day) & 0x3f);
  Wire.write((uint8_t)dec2bcd(tm.Month) & 0x1f);
  Wire.write((uint8_t)dec2bcd(tm.Year));//tmYearToY2k(tm.Year))); 
  Wire.endTransmission();  
}
// PRIVATE FUNCTIONS

// Convert Decimal to Binary Coded Decimal (BCD)
uint8_t MCP7940RTC::dec2bcd(uint8_t num) {
  return ((num/10 * 16) + (num % 10));
}

// Convert Binary Coded Decimal (BCD) to Decimal
uint8_t MCP7940RTC::bcd2dec(uint8_t num) {
  return ((num/16 * 10) + (num % 16));
}

//MCP7940RTC RTC = MCP7940RTC(); // create an instance for the user

