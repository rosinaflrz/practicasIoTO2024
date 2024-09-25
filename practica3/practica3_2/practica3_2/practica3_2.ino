// Prácticas IoT - Práctica realizada por alumna: Rosina Flores
#include <Wire.h>                 // Librería para la comunicación I2C
#include <Adafruit_GFX.h>          // Librería gráfica para manejar displays
#include <Adafruit_SSD1306.h>      // Librería para el display OLED SSD1306
#include <Adafruit_Sensor.h>       // Librería base para sensores de Adafruit
#include <DHT.h>                   // Librería para el sensor DHT

#define SCREEN_WIDTH 128           // Ancho del display OLED en píxeles
#define SCREEN_HEIGHT 64           // Alto del display OLED en píxeles

// Declaración del display OLED conectado por I2C (pines SDA, SCL)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

#define DHTPIN 4                   // Pin digital conectado al sensor DHT

// Definir el tipo de sensor DHT que estamos utilizando
#define DHTTYPE DHT11              // Usamos el sensor DHT11
//#define DHTTYPE DHT22            // Descomentar si se usa un DHT22
//#define DHTTYPE DHT21            // Descomentar si se usa un DHT21

DHT dht(DHTPIN, DHTTYPE);          // Inicializamos el objeto del sensor DHT

void setup() {
  Serial.begin(115200);            // Inicializar la comunicación serial para depuración

  dht.begin();                     // Inicializar el sensor DHT

  // Inicializar el display OLED y verificar si está conectado correctamente
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed")); // Mostrar error si no se inicializa
    for(;;);                     // Bucle infinito si falla el OLED
  }
  delay(5000);                     // Esperar 2 segundos para estabilización
  display.clearDisplay();           // Limpiar el contenido previo en la pantalla
  display.setTextColor(WHITE);      // Establecer el color del texto en blanco
}

void loop() {
  delay(5000);                     // Esperar 5 segundos entre lecturas

  // Leer temperatura y humedad desde el sensor DHT
  float t = dht.readTemperature();  // Leer temperatura en grados Celsius
  float h = dht.readHumidity();     // Leer humedad en porcentaje

  // Mostrar los valores de temperatura y humedad en el monitor serial
  Serial.println(t);
  Serial.println(h);

  // Verificar si las lecturas son válidas
  if (isnan(h) || isnan(t)) {
    Serial.println("Fallo al leer datos del sensor DHT1!");  // Mostrar error si falla la lectura
  }

  // Limpiar el display antes de actualizarlo
  display.clearDisplay();
  
  // Mostrar la temperatura en el display OLED
  display.setTextSize(1);
  display.setCursor(0, 0);          // Posicionar el cursor en la parte superior izquierda
  display.print("Temperatura: ");
  display.setTextSize(2);
  display.setCursor(0, 10);         // Ajustar la posición del cursor
  display.print(t);                 // Mostrar el valor de la temperatura
  display.print(" ");
  display.setTextSize(1);
  display.cp437(true);              // Habilitar el conjunto de caracteres CP437 para el símbolo de grados
  display.write(167);               // Mostrar el símbolo de grados
  display.setTextSize(2);
  display.print("C");               // Mostrar la unidad Celsius

  // Mostrar la humedad en el display OLED
  display.setTextSize(1);
  display.setCursor(0, 35);         // Posicionar el cursor más abajo para mostrar la humedad
  display.print("Humedad: ");
  display.setTextSize(2);
  display.setCursor(0, 45);
  display.print(h);                 // Mostrar el valor de la humedad
  display.print(" %");              // Añadir el símbolo de porcentaje
  
  display.display();                // Actualizar la pantalla con los nuevos valores
}
