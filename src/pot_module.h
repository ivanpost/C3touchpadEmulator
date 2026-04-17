#ifndef POT_MODULE_H
#define POT_MODULE_H

#include <Arduino.h>

// Определение пинов
#define PIN_UD      4
#define PIN_CS1     2
#define PIN_CS2     3
#define PIN_OPTO    1

// Объявление функций
void initPotModule();
void setPot(int csPin, int steps);
void setBothPots(int valX, int valY);
void triggerOptoClick(int durationMs);  // Теперь с параметром длительности

#endif