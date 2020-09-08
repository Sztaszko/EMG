//
//    FILE: HX711.h
//  AUTHOR: Rob Tillaart
// VERSION: 0.2.0
// PURPOSE: Library for Loadcells for UNO
//     URL: https://github.com/RobTillaart/HX711
//
// HISTORY: see HX711.cpp
//
// NOTES
// Superset of interface of HX711 class of Bogde
// float iso long as float has 23 bits mantisse.

#include "Arduino.h"

#define HX711_LIB_VERSION  "0.2.0"

class HX711
{
public:
  HX711();
  ~HX711();

  // fixed gain 128 for now
  void begin(uint8_t dataPin, uint8_t clockPin);

  void reset();

  // checks if loadcell is ready to read.
  bool is_ready();
  
  // wait until ready, 
  // check every ms
  void wait_ready(uint32_t ms = 0);
  // max # retries
  bool wait_ready_retry(uint8_t retries = 3, uint32_t ms = 0);
  // max timeout
  bool wait_ready_timeout(uint32_t timeout = 1000, uint32_t ms = 0);

  // raw read
  float read();
  // multiple raw reads
  float read_average(uint8_t times = 10);
  // corrected for offset
  float get_value(uint8_t times = 1) { return read_average(times) - _offset; };
  // converted to proper units.
  float get_units(uint8_t times = 1);

  // TARE
  // call tare to calibrate zero
  void tare(uint8_t times = 10)      { _offset = read_average(times); };
  float get_tare()                   { return -_offset * _scale; };
  bool tare_set()                    { return _offset == 0; };

  // CORE "CONSTANTS" -> read datasheet
  // GAIN values: 128, 64 32  [only 128 tested & verified]
  void set_gain(uint8_t gain = 128)  { _gain = gain; };
  uint8_t get_gain()                 { return _gain; };
  // SCALE > 0
  void set_scale(float scale = 1.0)  { _scale = 1 / scale; };
  float get_scale()                  { return 1 / _scale; };
  // OFFSET > 0
  void set_offset(long offset = 0)   { _offset = offset; };
  long get_offset()                  { return _offset; };

  // CALIBRATION
  // clear the scale
  // call tare() to set the zero offset
  // put a known weight on the scale 
  // call callibrate_scale(weight) 
  // scale is calculated.
  void callibrate_scale(uint16_t weight, uint8_t times = 10);

  // POWER MANAGEMENT
  void power_down();
  void power_up();

  // TIME OF LAST READ
  uint32_t last_read()               { return _lastRead; };

  // PRICING  (idem calories?)
  float get_price(uint8_t times = 1) { return get_units(times) * _price; };
  void  set_unit_price(float price)  { _price = price; };
  float get_unit_price()             { return _price; };

private:
  uint8_t  _dataPin;
  uint8_t  _clockPin;

  uint8_t  _gain = 128;     // default channel A
  long     _offset = 0;
  float    _scale = 1;
  uint32_t _lastRead = 0;
  float    _price = 0;
};

HX711::HX711()
{
  reset();
}

HX711::~HX711() {}

void HX711::begin(uint8_t dataPin, uint8_t clockPin)
{
  _dataPin = dataPin;
  _clockPin = clockPin;

  pinMode(_dataPin, INPUT);
  pinMode(_clockPin, OUTPUT);
  digitalWrite(_clockPin, LOW);

  reset();
}

void HX711::reset()
{
  _offset = 0;
  _scale = 1;
  _gain = 128;
}

bool HX711::is_ready()
{
  return digitalRead(_dataPin) == LOW;
}

float HX711::read() 
{
  // this waiting takes most time...
  while (digitalRead(_dataPin) == HIGH) yield();
  
  union
  {
    long value = 0;
    uint8_t data[4];
  } v;

  noInterrupts();

  // Pulse the clock pin 24 times to read the data.
  v.data[2] = shiftIn(_dataPin, _clockPin, MSBFIRST);
  v.data[1] = shiftIn(_dataPin, _clockPin, MSBFIRST);
  v.data[0] = shiftIn(_dataPin, _clockPin, MSBFIRST);

  // TABLE 3 page 4 datasheet
  // only default verified, so other values not supported yet
  uint8_t m = 1;   // default gain == 128
  if (_gain == 64) m = 3;
  if (_gain == 32) m = 2;

  while (m > 0)
  {
    digitalWrite(_clockPin, HIGH);
    digitalWrite(_clockPin, LOW);
    m--;
  }

  interrupts();

  // SIGN extend
  if (v.data[2] & 0x80) v.data[3] = 0xFF;

  _lastRead = millis();
  return 1.0 * v.value;
}

// assumes tare() has been set.
void HX711::callibrate_scale(uint16_t weight, uint8_t times)
{
  _scale = (1.0 * weight) / (read_average(times) - _offset);
}

void HX711::wait_ready(uint32_t ms) 
{
  while (!is_ready())
  {
    delay(ms);
  }
}

bool HX711::wait_ready_retry(uint8_t retries, uint32_t ms) 
{
  while (retries--)
  {
    if (is_ready()) return true;
    delay(ms);
  }
  return false;
}

bool HX711::wait_ready_timeout(uint32_t timeout, uint32_t ms)
{
  uint32_t start = millis();
  while (millis() - start < timeout) 
  {
    if (is_ready()) return true;
    delay(ms);
  }
  return false;
}

float HX711::read_average(uint8_t times) 
{
  float sum = 0;
  for (uint8_t i = 0; i < times; i++) 
  {
    sum += read();
    yield();
  }
  return sum / times;
}

float HX711::get_units(uint8_t times)
{
  float units = get_value(times) * _scale;
  return units;
};

void HX711::power_down() 
{
  digitalWrite(_clockPin, LOW);
  digitalWrite(_clockPin, HIGH);
}

void HX711::power_up() 
{
  digitalWrite(_clockPin, LOW);
}
#define calibration_factor -87690.0
#define DOUT  3
#define CLK  2

HX711 scale;

void setup() {
  Serial.begin(9600);
  scale.begin(DOUT,CLK);
  scale.tare();
  Serial.println("Put 2kg on the scale to calibrate (5s):");
  delay(5000);
  scale.callibrate_scale(2);
  Serial.println("\nCallibrating");
}

uint32_t last_time = 0;

void loop() {
  if(scale.is_ready()){
  //Serial.print(scale.last_read());
  //Serial.print(",");
  //Serial.print(scale.last_read()-last_time);
  //last_time = scale.last_read();
  //Serial.print(",");
  Serial.println(scale.get_scale());
  
  }
  delay(10);
}
