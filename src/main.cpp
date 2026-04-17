#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include "pot_module.h"  // Подключаем модуль потенциометров


// Используем стандартные UUID для UART сервиса
#define SERVICE_UUID        "6E400001-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_RX_UUID "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_TX_UUID "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"

#define LED_PIN             8

BLEServer* pServer = NULL;
BLECharacteristic* pTxCharacteristic = NULL;
bool deviceConnected = false;
unsigned long lastBlinkTime = 0;
bool ledState = false;
unsigned long connectionTime = 0;
bool greetingSent = false;

// Класс для обработки подключений
class MyServerCallbacks: public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) {
    deviceConnected = true;
    greetingSent = false;
    connectionTime = millis();
    digitalWrite(LED_PIN, LOW);  // LED гаснет при подключении
    //Serial.println("Client connected");
  }
  
  void onDisconnect(BLEServer* pServer) {
    deviceConnected = false;
    greetingSent = false;
    digitalWrite(LED_PIN, HIGH);
    //Serial.println("Client disconnected");
    // Перезапускаем рекламу
    pServer->startAdvertising();
  }
};

// Класс для обработки входящих сообщений (RX)
class MyCallbacks: public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pCharacteristic) {
    String value = pCharacteristic->getValue();
    
    if (value.length() > 0) {
      Serial.print("Received: ");
      Serial.println(value);
      
      // ПРОВЕРКА НА КОМАНДУ ДЛЯ ПОТЕНЦИОМЕТРОВ
      // Формат команды: 
      // - "$32 45"           (длительность по умолчанию 150 мс)
      // - "$32 45 300"       (длительность 300 мс)
      if (value[0] == '$') {
        int valX = 0, valY = 0, duration = 150;  // duration по умолчанию 150
        
        // Пробуем распарсить 2 или 3 числа
        int parsed = sscanf(value.c_str(), "$%d %d %d", &valX, &valY, &duration);
        
        if (parsed >= 2) {  // Минимум 2 числа есть
          // Устанавливаем оба потенциометра
          setBothPots(valX, valY);
          
          // Имитируем нажатие с указанной длительностью
          triggerOptoClick(duration);
          
          // Отправляем подтверждение
          String response;
          if (parsed == 3) {
            response = "++Pots set to: " + String(valX) + " " + String(valY) + 
                       ", click: " + String(duration) + "ms";
          } else {
            response = "++Pots set to: " + String(valX) + " " + String(valY) + 
                       ", click: 150ms (default)";
          }
          pTxCharacteristic->setValue(response);
          pTxCharacteristic->notify();
        } else {
          // Ошибка парсинга
          String response = "++Error: Use format $value1 value2 [duration]";
          pTxCharacteristic->setValue(response);
          pTxCharacteristic->notify();
        }
      } 
      else {
        // Обычный ответ с "++" (ваша существующая логика)
        String response = "++" + value;
        pTxCharacteristic->setValue(response);
        pTxCharacteristic->notify();
      }
    }
  }
};

void setup() {
  //Serial.begin(115200);
  //delay(100);

  // ИНИЦИАЛИЗАЦИЯ МОДУЛЯ ПОТЕНЦИОМЕТРОВ
  initPotModule();
  
  
  // Настройка LED
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);  // Начинаем с выключенного (инвертировано)
  
  // Инициализация BLE
  BLEDevice::init("esp32c");
  
  // Создание сервера
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());
  
  // Создание сервиса
  BLEService *pService = pServer->createService(SERVICE_UUID);
  
  // Создание характеристики для TX
  pTxCharacteristic = pService->createCharacteristic(
                    CHARACTERISTIC_TX_UUID,
                    BLECharacteristic::PROPERTY_NOTIFY
                  );
  pTxCharacteristic->addDescriptor(new BLE2902());
  
  // Создание характеристики для RX
  BLECharacteristic *pRxCharacteristic = pService->createCharacteristic(
                    CHARACTERISTIC_RX_UUID,
                    BLECharacteristic::PROPERTY_WRITE
                  );
  pRxCharacteristic->setCallbacks(new MyCallbacks());
  
  // Запуск сервиса
  pService->start();
  
  // Настройка рекламы
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);
  pAdvertising->setMaxPreferred(0x12);
  
  // Старт рекламы
  BLEDevice::startAdvertising();
  
  //Serial.println("BLE UART Server ready");
  //Serial.println("Device name: esp32");
}

void loop() {
  // Отправка приветствия после подключения
  if (deviceConnected && !greetingSent) {
    // Ждем 500ms после подключения перед отправкой
    if (millis() - connectionTime > 1000) {
      String greeting = "Connect - OK";
      pTxCharacteristic->setValue(greeting);
      pTxCharacteristic->notify();
      greetingSent = true;
      //Serial.println("Greeting sent: Connect - OK");
    }
  }
  
  // Мигание LED если нет подключения
  if (!deviceConnected) {
    unsigned long currentMillis = millis();
    if (currentMillis - lastBlinkTime >= 500) {
      lastBlinkTime = currentMillis;
      ledState = !ledState;
      digitalWrite(LED_PIN, ledState);
    }
  }
  
  delay(10);
}