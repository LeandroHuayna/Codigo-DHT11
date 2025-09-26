#if defined(ESP32)
  #include <WiFi.h>   // Librería WiFi para ESP32
#elif defined(ESP8266)
  #include <ESP8266WiFi.h> // Librería WiFi para ESP8266
#endif

#include <Firebase_ESP_Client.h>  // Librería de Firebase
#include "DHT.h"                  // Librería del sensor DHT11

// Archivos de ayuda para Firebase (muestran errores y estado de conexión)
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"

// -----------------------------
// CONFIGURACIÓN SENSOR DHT11
// -----------------------------
#define DHTPIN 25       // Pin donde está conectado el DHT11
#define DHTTYPE DHT11   // Definimos el tipo de sensor
DHT dht(DHTPIN, DHTTYPE);

// -----------------------------
// CONFIGURACIÓN DEL RELÉ
// -----------------------------
int pinRele = 26;        // Pin que controla el relé
#define UMBRAL_TEMP 24   // Temperatura límite para encender el ventilador

// -----------------------------
// CONFIGURACIÓN WIFI Y FIREBASE
// -----------------------------
#define WIFI_SSID "POCO X6 5G"        // Nombre de tu red WiFi
#define WIFI_PASSWORD "luisfer456"    // Contraseña de tu WiFi

#define API_KEY "AIzaSyDsEb_YAGcr9S9fKjzmJKuA7RdNuJtQZJg"   // API Key de Firebase
#define DATABASE_URL "https://huerto-unandes-default-rtdb.firebaseio.com/" // URL de tu BD

// Usuario de Firebase (Authentication con email y contraseña)
#define USER_EMAIL "josueleonardomanriquez@gmail.com"
#define USER_PASSWORD "12341234"

// Objetos para manejar la conexión con Firebase
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

// Variables de control para el envío de datos
unsigned long sendDataPrevMillis = 0;
const long timerDelay = 5000; // Tiempo entre envíos (5s)

void setup() {
  Serial.begin(115200);
  dht.begin();  // Inicializamos el DHT11

  pinMode(pinRele, OUTPUT);
  digitalWrite(pinRele, HIGH); // Relé apagado al inicio

  // Conectar al WiFi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Conectando a WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\n✅ WiFi conectado");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());

  // Configuración de Firebase
  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;
  config.token_status_callback = tokenStatusCallback;

  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
  Serial.println("✅ Firebase inicializado");
}

void loop() {
  // Leer valores de temperatura y humedad
  float temp = dht.readTemperature();
  float hum = dht.readHumidity();

  // Si hay error en la lectura, no seguimos
  if (isnan(temp) || isnan(hum)) {
    Serial.println("❌ Error al leer el DHT11");
    return;
  }

  // Mostrar en el monitor serie
  Serial.print("🌡 Temperatura: ");
  Serial.print(temp);
  Serial.print(" °C  |  💧 Humedad: ");
  Serial.print(hum);
  Serial.println(" %");

  // Encender o apagar ventilador según temperatura
  if (temp > UMBRAL_TEMP) {
    digitalWrite(pinRele, HIGH);   // Relé activo → Ventilador ON
    Serial.println("💨 Ventilador ENCENDIDO");
  } else {
    digitalWrite(pinRele, LOW);    // Relé inactivo → Ventilador OFF
    Serial.println("🛑 Ventilador APAGADO");
  }

  // Enviar datos a Firebase cada 5 segundos
  if (Firebase.ready() && (millis() - sendDataPrevMillis > timerDelay || sendDataPrevMillis == 0)) {
    sendDataPrevMillis = millis();

    // Enviar humedad
    if (Firebase.RTDB.setFloat(&fbdo, "DHT11/Humedad", hum)) {
      Serial.println("✅ Humedad enviada");
    } else {
      Serial.println("❌ Error humedad: " + fbdo.errorReason());
    }

    // Enviar temperatura
    if (Firebase.RTDB.setFloat(&fbdo, "DHT11/Temperatura", temp)) {
      Serial.println("✅ Temperatura enviada");
    } else {
      Serial.println("❌ Error temperatura: " + fbdo.errorReason());
    }
  }

  delay(2000); // Esperar 2s antes de la siguiente lectura
}
