/*
 * Anardino library use example for MCP7940N RTC using in MiniWireless class products.
 *
 * Synopsis:
 *  - Set specific time in RTC
 *  - synchronize local arduino time
 *  - set initial alarm 10-seconds later
 *  - loop on increasing relative alarm time intervals
 *  - print date/time output along the way
 *  -- see screenshot for sample view of output
 *
*/ 
 
#include <Time.h>
#include <Wire.h>
#include <MCP7940RTC.h>
#define RTC_INT 3                     // arduino hardware interrupt pin, (i.e, D3 for miniWireless)

char buf[32];
MCP7940RTC *pRTC;    // setup rtc pointer
time_t tt;
int gotInterrupt=0;
int nextIntr=5;
char *p;

void setup() {
  pinMode(RTC_INT, INPUT_PULLUP);     // INPUT also sufficient, if we have our own on-board discrete pullup
  memset(buf,0,sizeof(buf));
  Serial.begin(115200);
  Serial.println("Startup...");
  delay(20);
  
  pRTC = new MCP7940RTC();            // initialize rtc pointer
  setNewTimeRTC(2013, 12, 23, 11, 32, 10);  // Set RTC time, (year, month, day, hour, minute, second)
  p = &buf[0];
  pRTC->getDateStr(p);  
  Serial.println(p);                  // Print current date, utc-seconds.
  
  setTime(11,32,10,23,12,2013);       // Set current Anarduino date/time to same as what we've just sent to RTC
  tt = now() + 10;                    // setup time_t to 10-seconds in future
  pRTC->setAlarm0(tt);                // set alarm in 10-sec
  attachInterrupt(1,rtcIntr,FALLING);
  Serial.print("tt=");
  Serial.println(tt);
}

void loop() {
  if(gotInterrupt>0) {
    nextIntr *= 2;
    p = &buf[0];
    pRTC->getDateStr(p);  
    Serial.println(p);
    Serial.print("Got Interrupt, next interrupt in ");
    Serial.print(nextIntr);
    Serial.println(" sec.");
    gotInterrupt=0;  // reset flag    
    tt = now() + nextIntr +1; // we add a second to account for latency throughout...
    pRTC->setAlarm0(tt);  // set new alarm increasingly further out in time.
  }
  delay(20);  
}

void rtcIntr() {  // interrupt service routine, just set flag
  gotInterrupt=1;
}

void setNewTimeRTC(int yr, int mo, int dy, int hr, int mn, int sec) {
  tmElements_t tm1;
  tm1.Year=(yr-1970);//(2013-1970);
  tm1.Month=mo;
  tm1.Day=dy;
  tm1.Hour=hr;
  tm1.Minute=mn;
  tm1.Second=sec;
  time_t t = makeTime(tm1);
  pRTC->setTimeRTC(t);
}

