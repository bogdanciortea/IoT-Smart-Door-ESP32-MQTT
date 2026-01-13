#include <WiFi.h>
#include <PubSubClient.h>
#include <ESP32Servo.h>
#include <LiquidCrystal_I2C.h>

// --- DATE CONEXIUNE ---
const char* ssid = "-----------"; 
const char* password = "----------------";

// --- DATE MQTT ---
const char* mqtt_broker = "usa-mea-iot.------------";
const char* mqtt_user = "usa-mea-iot";
const char* mqtt_pass = "--------------------------";

// --- PINI COMPONENTE ---
const int PIN_TRIG = 12; // Exemplu pini pentru D1 R32
const int PIN_ECHO = 13;
const int PIN_SERVO = 14;

WiFiClient espClient;
PubSubClient client(espClient);
Servo pârghieServo;
LiquidCrystal_I2C lcd(0x27, 16, 2);

void setup() {
  Serial.begin(115200);
  lcd.init();
  lcd.backlight();
  
  pârghieServo.attach(PIN_SERVO);
  pinMode(PIN_TRIG, OUTPUT);
  pinMode(PIN_ECHO, INPUT);

  WiFi.begin(ssid, password);
  client.setServer(mqtt_broker, 1883);
  client.setCallback(callback); // Functia pentru comenzi de pe telefon
}

// Functia care primeste comenzi de la telefon (MQTT Subscriber)
void callback(char* topic, byte* payload, unsigned int length) {
  String mesaj;
  for (int i = 0; i < length; i++) mesaj += (char)payload[i];
  
  if (mesaj == "OPEN") {
    deschideUsa();
  } else if (mesaj == "CLOSE") {
    inchideUsa();
  }
}

void deschideUsa() {
  pârghieServo.write(90); // Ridica parghia
  client.publish("casa/usa/stare", "DESCHISA"); // MQTT Publisher
  lcd.setCursor(0,0); lcd.print("Stare: DESCHISA");
}

void inchideUsa() {
  pârghieServo.write(0); // Coboara parghia
  client.publish("casa/usa/stare", "INCHISA");
  lcd.setCursor(0,0); lcd.print("Stare: INCHISA ");
}

void loop() {
  if (!client.connected()) {
    conectaMqtt();
  }
  client.loop();
  
  // Masurare distanta (Logica originala)
  long durata, distanta;
  digitalWrite(PIN_TRIG, LOW); delayMicroseconds(2);
  digitalWrite(PIN_TRIG, HIGH); delayMicroseconds(10);
  digitalWrite(PIN_TRIG, LOW);
  durata = pulseIn(PIN_ECHO, HIGH);
  distanta = (durata/2) / 29.1;

  if (distanta < 15 && distanta > 0) {
    deschideUsa();
    delay(3000); // Tine deschis 3 secunde
    inchideUsa();
  }
  delay(500);
}

void conectaMqtt() {
  while (!client.connected()) {
    if (client.connect("ESP32_Usa", mqtt_user, mqtt_pass)) {
      client.subscribe("casa/usa/comenzi");
    } else {
      delay(5000);
    }
  }
}
