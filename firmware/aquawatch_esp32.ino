/*
 * AquaWatch EWS - ESP32 -> Cloud sender (reference sketch)
 *
 * Posts raw sensor values to the deployed dashboard every 10 seconds.
 * The server calibrates the raw values and runs the ML risk model.
 *
 * EDIT: WIFI_SSID, WIFI_PASS, and the three sensor pins/reads for your wiring.
 */
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>

const char* WIFI_SSID = "YOUR_WIFI_NAME";
const char* WIFI_PASS = "YOUR_WIFI_PASSWORD";

// Deployed server. Must be https:// and NO port number.
const char* SERVER_URL = "https://aquawatch-ews.onrender.com/api/sensor-data";
const char* DISTRICT   = "Nagaon";

const int TURBIDITY_PIN = 34;   // ADC1 pin (analog)
const int PH_PIN        = 35;   // ADC1 pin (analog)
// Temperature: replace readTemperatureC() with your DS18B20/DHT code.

float readTemperatureC() {
  return 26.0;  // TODO: real sensor
}

void connectWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) { delay(500); Serial.print("."); }
  Serial.println("\nWiFi connected: " + WiFi.localIP().toString());
}

void setup() {
  Serial.begin(115200);
  analogReadResolution(12);  // 0-4095, matches server validation
  connectWiFi();
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) connectWiFi();

  int turbidityRaw = analogRead(TURBIDITY_PIN);
  int phRaw        = analogRead(PH_PIN);
  float tempC      = readTemperatureC();

  String payload = "{\"turbidity_raw\":" + String(turbidityRaw) +
                   ",\"ph_raw\":" + String(phRaw) +
                   ",\"temperature_c\":" + String(tempC, 2) +
                   ",\"district\":\"" + DISTRICT + "\"}";

  WiFiClientSecure client;
  client.setInsecure();            // skip cert pinning (fine for demo)
  HTTPClient http;
  http.setTimeout(60000);          // free Render instance may need ~50s to wake
  if (http.begin(client, SERVER_URL)) {
    http.addHeader("Content-Type", "application/json");
    int code = http.POST(payload);
    Serial.printf("POST %s -> HTTP %d\n", payload.c_str(), code);
    http.end();
  }
  delay(10000);
}
