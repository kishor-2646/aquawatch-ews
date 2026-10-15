/*
 * AquaWatch EWS - ESP32 (Turbidity GPIO34, pH GPIO35, DS18B20 GPIO32)
 * Sends RAW ADC values to the deployed cloud dashboard over HTTPS.
 *
 * EDIT ONLY: WIFI_SSID, WIFI_PASSWORD
 * Phone hotspot must be 2.4 GHz (ESP32 does not support 5 GHz).
 */
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// ---------------- Wi-Fi ----------------
const char* WIFI_SSID     = "Kishor_2646";
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";

// ---------------- Cloud server ----------------
// https:// (NOT http://) and no port number.
const char* SERVER_URL = "https://aquawatch-ews.onrender.com/api/sensor-data";
const char* DISTRICT   = "Nagaon";

// ---------------- Pins ----------------
const int TURBIDITY_PIN = 34;
const int PH_PIN        = 35;
const int TEMP_PIN      = 32;

OneWire oneWire(TEMP_PIN);
DallasTemperature temperatureSensor(&oneWire);

float readAverageADC(int pin, int samples = 20) {
  long total = 0;
  for (int i = 0; i < samples; i++) {
    total += analogRead(pin);
    delay(10);
  }
  return total / (float)samples;
}

void connectWiFi() {
  Serial.print("Connecting to Wi-Fi");
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 30) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  Serial.println();

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("Wi-Fi connected!");
    Serial.print("ESP32 IP address: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("Wi-Fi connection FAILED.");
  }
}

void setup() {
  Serial.begin(115200);
  analogReadResolution(12);
  temperatureSensor.begin();

  Serial.println();
  Serial.println("=================================");
  Serial.println("AquaWatch ESP32 (cloud mode)");
  Serial.println("=================================");

  connectWiFi();
}

void loop() {
  float turbidityRaw = readAverageADC(TURBIDITY_PIN);
  float phRaw        = readAverageADC(PH_PIN);

  temperatureSensor.requestTemperatures();
  float temperatureC = temperatureSensor.getTempCByIndex(0);

  Serial.println();
  Serial.println("========== SENSOR DATA ==========");
  Serial.print("Turbidity Raw : "); Serial.println(turbidityRaw, 1);
  Serial.print("pH Raw        : "); Serial.println(phRaw, 1);
  Serial.print("Temperature   : ");
  if (temperatureC == DEVICE_DISCONNECTED_C) {
    Serial.println("NOT DETECTED");
  } else {
    Serial.print(temperatureC, 2);
    Serial.println(" C");
  }
  Serial.println("=================================");

  // Don't send garbage: skip the upload if the temperature probe is unplugged.
  if (temperatureC == DEVICE_DISCONNECTED_C) {
    Serial.println("Skipping upload: DS18B20 not detected (check wiring + 4.7k pull-up).");
    delay(5000);
    return;
  }

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Wi-Fi disconnected. Reconnecting...");
    connectWiFi();
    delay(2000);
    return;
  }

  WiFiClientSecure client;
  client.setInsecure();          // demo: skip certificate pinning

  HTTPClient http;
  http.setTimeout(60000);        // free Render server can take ~50 s to wake up
  http.setReuse(false);

  if (!http.begin(client, SERVER_URL)) {
    Serial.println("http.begin() failed.");
    delay(5000);
    return;
  }
  http.addHeader("Content-Type", "application/json");

  String json = "{";
  json += "\"turbidity_raw\":" + String(turbidityRaw, 1);
  json += ",\"ph_raw\":"       + String(phRaw, 1);
  json += ",\"temperature_c\":" + String(temperatureC, 2);
  json += ",\"district\":\""   + String(DISTRICT) + "\"";
  json += "}";

  Serial.println("Sending JSON:");
  Serial.println(json);

  int responseCode = http.POST(json);
  Serial.print("HTTP response: ");
  Serial.println(responseCode);

  if (responseCode > 0) {
    Serial.println("Server response:");
    Serial.println(http.getString());
  } else {
    Serial.print("HTTP request failed: ");
    Serial.println(http.errorToString(responseCode));
  }
  http.end();

  delay(10000);                  // send every ~10 s
}
