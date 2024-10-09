// Envio de temperatura a otro ESP32 por medio del protocolo Bluetooth
// Asegurate de cambiar el nombre del device name de tu dispositivo linea 13
// Codigo clase IoT - Rosina Flores

// Incluir librerias requeridas para Bluetooth y el sensor DHT11
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <DHT.h>

// ==== PARÁMETROS CONFIGURABLES ====

// Configuración del nombre del dispositivo
#define DEVICE_NAME "ESPRosina"  // Cambiar este nombre para cada estudiante
// Configuración del tiempo de parpadeo del led cuando se reciba la alarta del receptor (en milisegundos)
const unsigned long BLINK_DURATION = 200000; // 20 segundos
const unsigned long BLINK_INTERVAL = 500;   // Intervalo de parpadeo en milisegundos

// ===============================================

// Configuración de pines y tipo de sensor
#define DHTPIN 4     // Pin digital conectado al sensor DHT
#define DHTTYPE DHT11   // Tipo de sensor DHT (DHT11 o DHT22)
#define LED_PIN 5    // Pin para el LED de alerta

// Inicialización del sensor DHT
DHT dht(DHTPIN, DHTTYPE);

// Variables para el manejo de BLE
BLEServer* pServer = NULL;
BLECharacteristic* pCharacteristic = NULL;
BLECharacteristic* pResponseCharacteristic = NULL;
bool deviceConnected = false;
bool oldDeviceConnected = false;
bool alertReceived = false;

// UUIDs para el servicio BLE y características
#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"
#define RESPONSE_CHARACTERISTIC_UUID "47a9744c-2d2c-4b8f-ba98-7a13c7c1234e"

// Umbral de temperatura (en grados Celsius)
const float TEMP_THRESHOLD = 25.0;

// Clase para manejar eventos de conexión y desconexión BLE
class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
    }
};

// Clase para manejar la recepción de datos por BLE
class MyCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
      uint8_t* pData = pCharacteristic->getData();
      size_t dataLength = pCharacteristic->getValue().length();

      if (dataLength > 0) {
        Serial.println("*********");
        Serial.print("Received Value: ");
        for (int i = 0; i < dataLength; i++) {
          Serial.print((char)pData[i]);
        }
        Serial.println();
        Serial.println("*********");

        // Verificar si se recibió una alerta
        if (dataLength == 5 && 
            pData[0] == 'A' && 
            pData[1] == 'L' && 
            pData[2] == 'E' && 
            pData[3] == 'R' && 
            pData[4] == 'T') {
          alertReceived = true;
        }
      }
    }
};

void setup() {
  Serial.begin(115200);
  dht.begin();
  pinMode(LED_PIN, OUTPUT);
  
  // Inicializar el dispositivo BLE con el nombre personalizado
  BLEDevice::init(DEVICE_NAME);
  
  // Crear el servidor BLE
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Crear el servicio BLE
  BLEService *pService = pServer->createService(SERVICE_UUID);

  // Crear la característica para enviar datos de temperatura
  pCharacteristic = pService->createCharacteristic(
                      CHARACTERISTIC_UUID,
                      BLECharacteristic::PROPERTY_READ   |
                      BLECharacteristic::PROPERTY_WRITE  |
                      BLECharacteristic::PROPERTY_NOTIFY |
                      BLECharacteristic::PROPERTY_INDICATE
                    );

  // Crear la característica para recibir respuestas
  pResponseCharacteristic = pService->createCharacteristic(
                      RESPONSE_CHARACTERISTIC_UUID,
                      BLECharacteristic::PROPERTY_READ   |
                      BLECharacteristic::PROPERTY_WRITE  |
                      BLECharacteristic::PROPERTY_NOTIFY |
                      BLECharacteristic::PROPERTY_INDICATE
                    );

  pResponseCharacteristic->setCallbacks(new MyCallbacks());

  // Agregar descriptor a las características
  pCharacteristic->addDescriptor(new BLE2902());
  pResponseCharacteristic->addDescriptor(new BLE2902());

  // Iniciar el servicio
  pService->start();

  // Iniciar advertising
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(false);
  pAdvertising->setMinPreferred(0x0);
  BLEDevice::startAdvertising();
  
  Serial.print(DEVICE_NAME);
  Serial.println(" está esperando una conexión BLE para notificar...");
}

void loop() {
  // Leer la temperatura del sensor DHT
  float temperature = dht.readTemperature();
  
  if (isnan(temperature)) {
    Serial.println("Error al leer del sensor DHT!");
    return;
  }

  Serial.print("Temperatura: ");
  Serial.print(temperature);
  Serial.println(" *C");

  // Si hay un dispositivo conectado, enviar la temperatura
  if (deviceConnected) {
    char txString[8];
    dtostrf(temperature, 1, 2, txString);
    pCharacteristic->setValue((uint8_t*)txString, strlen(txString));
    pCharacteristic->notify();
  }

  // Si se recibió una alerta, hacer parpadear el LED por el tiempo definido 
  // en los parametros configurables
  if (alertReceived) {
    unsigned long startTime = millis(); // Registra el tiempo de inicio
    while (millis() - startTime < BLINK_DURATION) {
      digitalWrite(LED_PIN, HIGH);  // Enciende el LED
      delay(BLINK_INTERVAL / 2);    // Espera la mitad del intervalo
      digitalWrite(LED_PIN, LOW);   // Apaga el LED
      delay(BLINK_INTERVAL / 2);    // Espera la otra mitad del intervalo
    }
    alertReceived = false;  // Resetear la alerta cuando termine el parpadeo
  }

  // Manejar la desconexión y reconexión
  if (!deviceConnected && oldDeviceConnected) {
    delay(500); // Dar tiempo al bluetooth para estar listo
    pServer->startAdvertising(); // Reiniciar advertising
    Serial.println("Iniciar advertising");
    oldDeviceConnected = deviceConnected;
  }
  if (deviceConnected && !oldDeviceConnected) {
    oldDeviceConnected = deviceConnected;
  }
  
  delay(2500); // Esperar 2.5 segundos antes de la próxima lectura
}