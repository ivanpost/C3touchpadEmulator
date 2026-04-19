#include "pot_module.h"

// Инициализация модуля
void initPotModule() {
    pinMode(PIN_UD, OUTPUT);
    pinMode(PIN_CS1, OUTPUT);
    pinMode(PIN_CS2, OUTPUT);
    pinMode(PIN_OPTO, OUTPUT);
    
    // Неактивное состояние: CS всегда HIGH
    digitalWrite(PIN_CS1, HIGH);
    digitalWrite(PIN_CS2, HIGH);
    digitalWrite(PIN_OPTO, LOW);  // Оптопара разомкнута
    
    // Начальная установка потенциометров в 0
    setBothPots(0, 0);
    
    Serial.println("Pot module initialized");
}

// Установка одного потенциометра
void setPot(int csPin, int steps) {
    steps = constrain(steps, 0, 63);
    
    // 1. СБРОС В НОЛЬ (ВНИЗ)
    digitalWrite(PIN_UD, LOW);  // Направление - вниз
    for (int i = 0; i < 65; i++) {
        digitalWrite(csPin, LOW);
        delayMicroseconds(10);
        digitalWrite(csPin, HIGH);  // Шаг происходит по этому перепаду
        delayMicroseconds(10);
    }
    
    // 2. УСТАНОВКА ЗНАЧЕНИЯ (ВВЕРХ)
    digitalWrite(PIN_UD, HIGH);  // Направление - вверх
    for (int i = 0; i < steps; i++) {
        digitalWrite(csPin, LOW);
        delayMicroseconds(10);
        digitalWrite(csPin, HIGH);  // Шаг вверх
        delayMicroseconds(10);
    }
}

// Установка обоих потенциометров
void setBothPots(int valX, int valY) {
    Serial.print("Setting pots: X=");
    Serial.print(valX);
    Serial.print(", Y=");
    Serial.println(valY);
    
    setPot(PIN_CS1, valX);
    setPot(PIN_CS2, valY);
}

// Имитация нажатия кнопки с заданной длительностью
void triggerOptoClick(int durationMs) {
    // Ограничиваем длительность от 50 до 2000 мс
    durationMs = constrain(durationMs, 100, 9000);
    
    digitalWrite(PIN_OPTO, HIGH);   // Замыкаем
    delay(durationMs);               // Ждем заданное время
    digitalWrite(PIN_OPTO, LOW);     // Размыкаем
    
    Serial.print("Opto click triggered, duration: ");
    Serial.print(durationMs);
    Serial.println(" ms");
}