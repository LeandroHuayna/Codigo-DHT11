#include "DHT.h"

// -----------------------------
// CONFIGURACIÓN DEL DHT11
// -----------------------------
#define DHTPIN 25       // Pin de datos del DHT11 (cámbialo según tu conexión)
#define DHTTYPE DHT11  // Tipo de sensor
DHT dht(DHTPIN, DHTTYPE);

// -----------------------------
// CONFIGURACIÓN DEL RELÉ
// -----------------------------
int pinRele = 26;   // Pin del ESP32 conectado al IN del módulo relé

void setup() {
  Serial.begin(115200);
  dht.begin();

  pinMode(pinRele, OUTPUT);

  // Relé inicialmente apagado
  digitalWrite(pinRele, HIGH); // Asumiendo relé activo en LOW
}

void loop() {
  // Leer temperatura y humedad
  float temp = dht.readTemperature();
  float hum = dht.readHumidity();

  // Verificar que la lectura es válida
  if (isnan(temp) || isnan(hum)) {
    Serial.println("❌ Error al leer el DHT11");
    return;
  }

  Serial.print("🌡 Temperatura: ");
  Serial.print(temp);
  Serial.print(" °C  |  💧 Humedad: ");
  Serial.print(hum);
  Serial.println(" %");

  // Control del ventilador según la temperatura
  if (temp > 24 ) {
    digitalWrite(pinRele, HIGH);   // Encender ventilador
    Serial.println("💨 Ventilador ENCENDIDO");
  } else {
    digitalWrite(pinRele, LOW);  // Apagar ventilador
    Serial.println("🛑 Ventilador APAGADO");
  }

  delay(2000); // Leer cada 2 segundos
}
