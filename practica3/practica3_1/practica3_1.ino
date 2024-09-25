#include "DHT.h" // Librería para el sensor DHT

#define DHTPIN 16     // Pin al que está conectado el sensor
#define DHTTYPE DHT11 // Tipo de sensor

DHT dht(DHTPIN, DHTTYPE); // Inicialización del sensor

void setup() {
  Serial.begin(9600);             // Iniciar comunicación serial
  Serial.println(F("DHTxx test!"));// Mensaje de inicio
  dht.begin();                    // Iniciar el sensor
}

void loop() {
  delay(2000);                      // Espera de 2 segundos entre lecturas

  float h = dht.readHumidity();      // Leer humedad
  float t = dht.readTemperature();   // Leer temperatura

  // Verificar errores en la lectura
  if (isnan(h) || isnan(t)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }

  // Imprimir humedad y temperatura
  Serial.print(F("Humedad: "));
  Serial.print(h);
  Serial.print(F("%  Temperatura: "));
  Serial.print(t);
  Serial.println(F("°C"));
}
