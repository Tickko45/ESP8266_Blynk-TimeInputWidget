#define BLYNK_TEMPLATE_ID "TMPL6VPccPdNx"
#define BLYNK_TEMPLATE_NAME "TimeInputWidget"
#define BLYNK_AUTH_TOKEN "8AD1l2pHUAZ3hQejrhEVDVRNMqEsv4Vh"

#define BLYNK_FIRMWARE_VERSION        "0.1.0"

#define BLYNK_PRINT Serial

#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>

char auth[] = BLYNK_AUTH_TOKEN;
char ssid[] = "Tickko_Home WiFi"; //WiFi Name
char pass[] = "0631361663"; //Password

#include <TimeLib.h>

BlynkTimer timer;

bool led_set[2];
long timer_start_set[2] = {0xFFFF, 0xFFFF};
long timer_stop_set[2] = {0xFFFF, 0xFFFF};
unsigned char weekday_set[2];

long rtc_sec;
unsigned char day_of_week;

bool led_status[2];
bool update_blynk_status[2];
bool led_timer_on_set[2];

#define LED_PIN D4

// #########################################################################################################
// LED 1
BLYNK_WRITE(V0)
{
  int val = param.asInt();

  if ( led_timer_on_set[0] == 0 ){
    led_set[0] = val;
    Serial.println("SW1 : OFF");
  }else{
    update_blynk_status[0] = 1;
    Serial.println("SW1 : ON");}
}

// #########################################################################################################
// LED 2
BLYNK_WRITE(V1)
{
  int val = param.asInt();

  if ( led_timer_on_set[1] == 0 ){
    led_set[1] = val;
    Serial.println("SW2 : OFF");
  }else{
    update_blynk_status[1] = 1; 
    Serial.println("SW2 : ON");} 
}

// #########################################################################################################
// Timer 1
BLYNK_WRITE(V2) 
{
  unsigned char week_day;
 
  TimeInputParam t(param);

  if (t.hasStartTime() && t.hasStopTime() ) 
  {
    timer_start_set[0] = (t.getStartHour() * 60 * 60) + (t.getStartMinute() * 60) + t.getStartSecond();
    timer_stop_set[0] = (t.getStopHour() * 60 * 60) + (t.getStopMinute() * 60) + t.getStopSecond();
    
    Serial.println(String("Start Time: ") +
                   t.getStartHour() + ":" +
                   t.getStartMinute() + ":" +
                   t.getStartSecond());
                   
    Serial.println(String("Stop Time: ") +
                   t.getStopHour() + ":" +
                   t.getStopMinute() + ":" +
                   t.getStopSecond());
                   
    for (int i = 1; i <= 7; i++) 
    {
      if (t.isWeekdaySelected(i)) 
      {
        week_day |= (0x01 << (i-1));
        Serial.println(String("Day ") + i + " is selected");
      }
      else
      {
        week_day &= (~(0x01 << (i-1)));
      }
    } 

    weekday_set[0] = week_day;
  }
  else
  {
    timer_start_set[0] = 0xFFFF;
    timer_stop_set[0] = 0xFFFF;
  }
}

// #########################################################################################################
// Timer 2
BLYNK_WRITE(V3) 
{
unsigned char week_day;
 
  TimeInputParam t(param);

  if (t.hasStartTime() && t.hasStopTime() ) 
  {
    timer_start_set[1] = (t.getStartHour() * 60 * 60) + (t.getStartMinute() * 60) + t.getStartSecond();
    timer_stop_set[1] = (t.getStopHour() * 60 * 60) + (t.getStopMinute() * 60) + t.getStopSecond();
    
    Serial.println(String("Start Time: ") +
                   t.getStartHour() + ":" +
                   t.getStartMinute() + ":" +
                   t.getStartSecond());
                   
    Serial.println(String("Stop Time: ") +
                   t.getStopHour() + ":" +
                   t.getStopMinute() + ":" +
                   t.getStopSecond());
                   
    for (int i = 1; i <= 7; i++) 
    {
      if (t.isWeekdaySelected(i)) 
      {
        week_day |= (0x01 << (i-1));
        Serial.println(String("Day ") + i + " is selected");
      }
      else
      {
        week_day &= (~(0x01 << (i-1)));
      }
    } 

    weekday_set[1] = week_day;
  }
  else
  {
    timer_start_set[1] = 0xFFFF;
    timer_stop_set[1] = 0xFFFF;
  }
}

// #########################################################################################################
BLYNK_WRITE(InternalPinRTC) 
{
  const unsigned long DEFAULT_TIME = 1357041600; // Jan 1 2013
  unsigned long blynkTime = param.asLong(); 
  
  if (blynkTime >= DEFAULT_TIME) 
  {
    setTime(blynkTime);

    day_of_week = weekday();
  
    if ( day_of_week == 1 )
      day_of_week = 7;
    else
      day_of_week -= 1; 
    
    rtc_sec = (hour()*60*60) + (minute()*60) + second();
   
    Serial.println(blynkTime);
    Serial.println(String("RTC Server: ") + hour() + ":" + minute() + ":" + second());
    Serial.println(String("Day of Week: ") + weekday()); 
  }
}

// #########################################################################################################
BLYNK_CONNECTED() 
{
  Blynk.sendInternal("rtc", "sync"); 
}

// #########################################################################################################
void checkTime() 
{
  Blynk.sendInternal("rtc", "sync"); 
}

// #########################################################################################################
void led_mng()
{
  bool time_set_overflow;
  bool led_status_buf[2];
  
  for (int i=0; i<2; i++)
  {
    led_status_buf[i] = led_status[i];
    time_set_overflow = 0;
    
    if ( timer_start_set[i] != 0xFFFF && timer_stop_set[i] != 0xFFFF)
    {
      if ( timer_stop_set[i] < timer_start_set[i] ) time_set_overflow = 1;

      if ((((time_set_overflow == 0 && (rtc_sec >= timer_start_set[i]) && (rtc_sec < timer_stop_set[i])) ||
        (time_set_overflow  && ((rtc_sec >= timer_start_set[i]) || (rtc_sec < timer_stop_set[i])))) && 
        (weekday_set[i] == 0x00 || (weekday_set[i] & (0x01 << (day_of_week - 1) )))) )
        {
          led_timer_on_set[i] = 1;
        }
        else
          led_timer_on_set[i] = 0;
    }
    else
      led_timer_on_set[i] = 0;

    if ( led_timer_on_set[i] )
    {
      led_status[i] = 1;
      led_set[i] = 0;
    }
    else
    {
      led_status[i] = led_set[i];
    }

    if ( led_status_buf[i] != led_status[i] )
      update_blynk_status[i] = 1;  
  }
    // HARDWARE CONTROL
  digitalWrite(LED_PIN, !led_status[0]);  
  digitalWrite(D1, !led_status[1]);  
}

// #########################################################################################################
void blynk_update()
{
  if ( update_blynk_status[0] )
  {
      update_blynk_status[0] = 0;
      Blynk.virtualWrite(V0, led_status[0]);
  }  

  if ( update_blynk_status[1] )
  {
      update_blynk_status[1] = 0;
      Blynk.virtualWrite(V1, led_status[1]);
  } 
}

// #########################################################################################################
void setup()
{
  Serial.begin(115200);
  delay(100);

  pinMode(LED_PIN, OUTPUT);
  pinMode(D1, OUTPUT);
  Blynk.begin(auth, ssid, pass);
  Serial.println("Blynk Ready!!!");
  timer.setInterval(1000L, checkTime);
}

// #########################################################################################################
void loop() 
{
  timer.run();
  led_mng();
  blynk_update();
}
