#ifndef BASE_SENSOR
#define BASE_SENSOR

#include <Arduino.h>

class Sensor {
public:
  virtual ~Sensor() {}
  virtual int getSensorCount()=0;
  virtual bool getValue(const int index, String& value)=0;
  virtual String getName(int index)=0;
};
#endif
