#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <vector>
#include <HardwareSerial.h>

#define RX_PIN 16
#define TX_PIN 17

#define SERVICE_UUID "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

BLECharacteristic *pCharacteristic;

HardwareSerial SerialUART(1);

class UARTCallback : public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *characteristic) {
        String rxValueArduino = characteristic->getValue();
        if (rxValueArduino.length() > 0) {
            std::string rxValue(rxValueArduino.c_str());

            // Ausgabe 1: Direkt nach BLE-Empfang
            Serial.print("BLE Received Value (String): ");
            Serial.println(rxValueArduino); // Ausgabe als String
            Serial.print("BLE Received Value (HEX): ");
            std::vector<uint8_t> data(rxValue.begin(), rxValue.end());
            for (int i = 0; i < data.size(); i++) {
                Serial.print(data[i], HEX);
                Serial.print(" ");
            }
            Serial.println();

            // Ausgabe 2: Nach Weiterleitung an SerialUART (zur Überprüfung der Weiterleitung)
            Serial.print("Sending to SerialUART: ");
            for (int i = 0; i < data.size(); i++) {
                Serial.print(data[i], HEX);
                Serial.print(" ");
            }
            Serial.println();

            SerialUART.write(data.data(), data.size());
        }
    }
};

void setup() {
  Serial.begin(115200);
  SerialUART.begin(115200, SERIAL_8N1, RX_PIN, TX_PIN);

  Serial.println("Starting BLE work!");

  BLEDevice::init("ESP32-UART-BLE");
  BLEServer *pServer = BLEDevice::createServer();
  BLEService *pService = pServer->createService(SERVICE_UUID);

  pCharacteristic = pService->createCharacteristic(
      CHARACTERISTIC_UUID,
      BLECharacteristic::PROPERTY_READ |
          BLECharacteristic::PROPERTY_WRITE |
          BLECharacteristic::PROPERTY_NOTIFY);

  pCharacteristic->setCallbacks(new UARTCallback());
  pCharacteristic->addDescriptor(new BLE2902());
  pService->start();

  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);
  pAdvertising->setMinPreferred(0x12);
  pAdvertising->start();
  Serial.println("Characteristic defined!");

  // Adresse ausgeben
    Serial.print("BLE Address: ");
    Serial.println(BLEDevice::getAddress().toString().c_str());
}

void loop() {
    if (SerialUART.available() > 0) {
        String receivedString = ""; // String zum Speichern der empfangenen Daten
        while (SerialUART.available() > 0) {
            char c = SerialUART.read();
            if (c == '\0') break; // Stop bei Nullterminator
            receivedString += c;
        }
        Serial.print("Received String from UART: ");
        Serial.println(receivedString);
        pCharacteristic->setValue(receivedString.c_str()); // String senden
        pCharacteristic->notify();
    }
    delay(1);
}