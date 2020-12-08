#include <Arduino.h>
#include <DHT.h>

//ESP8266 ESP-12E
#define pwmOutputPin1 D0
#define tachInputPin1 D1
#define pwmOutputPin2 D2
#define tachInputPin2 D3
// #define tempInputPin1 D4
#define DHTPIN 8
#define DHTTYPE DHT11
#define calculationPeriod 1000 // Number of milliseconds over which to count interrupts

volatile int interruptCounter1; // Counter for hall sensor in fan1
volatile int interruptCounter2; // Counter for hall sensor in fan2
unsigned long previousMillis;
int RPM1;
int RPM2;
int pwmDuty;
int pwmRamp;
bool pwmState;
float h;
float t;

DHT dht(DHTPIN, DHTTYPE);

void ICACHE_RAM_ATTR handleInterrupt1() {
    // This is the function called by the interrupt
    interruptCounter1++;
}

void ICACHE_RAM_ATTR handleInterrupt2() {
    // This is the function called by the interrupt
    interruptCounter2++;
}

void computeFanSpeed1(int count1) {
    // interruptCounter counts 2 pulses per revolution of the fan over a one second period
    RPM1 = count1 / 2 * 60;
}
void computeFanSpeed2(int count2) {
    // interruptCounter counts 2 pulses per revolution of the fan over a one second period
    RPM2 = count2 / 2 * 60;
}

void displayFanSpeed() {
    Serial.print(RPM1, DEC);
    Serial.print(" RPM1\t");
    Serial.print(RPM2, DEC);
    Serial.print(" RPM2\t");
    Serial.print(F("Humidity: "));
    Serial.print(h);
    Serial.print(F("%  Temperature: "));
    Serial.print(t);
    Serial.print(F("°C\r\n"));
}

void setup()
{
    Serial.begin(9600);
    dht.begin();
    
    previousMillis = 0;
    interruptCounter1 = 0;
    interruptCounter2 = 0;
    RPM1 = 0;
    RPM2 = 0;
    pwmRamp = 0;
    pwmDuty = 1023; // Start at 100% speed
    pwmState = false;
    h = 0.0;
    t = 0.0;

    pinMode(pwmOutputPin1, OUTPUT);
    pinMode(tachInputPin1, INPUT_PULLUP);
    pinMode(pwmOutputPin2, OUTPUT);
    pinMode(tachInputPin2, INPUT_PULLUP);
    // analogWriteFreq(25000);  
    analogWrite(pwmOutputPin1, pwmDuty);
    analogWrite(pwmOutputPin2, pwmDuty);
    attachInterrupt(digitalPinToInterrupt(tachInputPin1), handleInterrupt1, FALLING);
    attachInterrupt(digitalPinToInterrupt(tachInputPin2), handleInterrupt2, FALLING);

}

void loop()
{
  // Process counters once every second
  if ((millis() - previousMillis) > calculationPeriod) {

    previousMillis = millis();
    int count1 = interruptCounter1;
    int count2 = interruptCounter2;
    interruptCounter1 = 0;
    interruptCounter2 = 0;

    computeFanSpeed1(count1);
    computeFanSpeed2(count2);
    
    h = dht.readHumidity();
    t = dht.readTemperature();

    displayFanSpeed();
    
    pwmRamp++;

    if (pwmRamp >= 10)
    {
      if (pwmState)
      {
        pwmDuty = 1023;
        pwmState = false;
      }
      else
      {
        pwmDuty = 511;
        pwmState = true;
      }
      analogWrite(pwmOutputPin1, pwmDuty);
      analogWrite(pwmOutputPin2, pwmDuty);
      pwmRamp = 0;
    }

  }

  yield();
}