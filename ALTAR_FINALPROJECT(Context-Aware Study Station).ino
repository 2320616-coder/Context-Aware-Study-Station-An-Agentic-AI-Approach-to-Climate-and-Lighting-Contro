#include <DHT.h>
#include <LiquidCrystal_I2C.h>

#define DHTPIN 3
#define DHTTYPE DHT11
#define LDR_PIN A0
#define TRIG_PIN 6   
#define ECHO_PIN 5   
#define FAN_RELAY 2
#define LED_RELAY 4

DHT dht(DHTPIN, DHTTYPE);
LiquidCrystal_I2C lcd(0x27, 16, 2);

int vacantCounter = 0; 
String currentStatus = "VACANT";
const int VACANT_THRESHOLD = 5; 
unsigned long lastLCDUpdate = 0; // Para hindi mabaliw ang LCD

void setup() {
  Serial.begin(9600);
  dht.begin();
  lcd.init(); 
  lcd.backlight();
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(FAN_RELAY, OUTPUT);
  pinMode(LED_RELAY, OUTPUT);
  digitalWrite(FAN_RELAY, HIGH); 
  digitalWrite(LED_RELAY, HIGH);
  
  lcd.setCursor(0,0);
  lcd.print("System Ready...");
  delay(1000);
}

void loop() {
  // 1. SENSING
  digitalWrite(TRIG_PIN, LOW); delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH); delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  long duration = pulseIn(ECHO_PIN, HIGH, 25000); 
  int distance = duration * 0.034 / 2;

  // 2. OCCUPANCY LOGIC (60cm Range)
  if (distance > 2 && distance < 60) {
    currentStatus = "OCCUPIED";
    vacantCounter = 0; 
  } else {
    vacantCounter++;
    if (vacantCounter >= VACANT_THRESHOLD) {
      currentStatus = "VACANT";
    }
  }

  float t = dht.readTemperature();
  int ldrVal = analogRead(LDR_PIN);

  // 3. SEND DATA TO PYTHON
  Serial.print(t); Serial.print(",");
  Serial.print(ldrVal); Serial.print(",");
  Serial.println(currentStatus);

  // 4. LCD UPDATE (Every 500ms only to prevent blanking)
  if (millis() - lastLCDUpdate >= 500) {
    lcd.setCursor(0,0);
    lcd.print("T:"); lcd.print(t,0); lcd.print("C L:"); lcd.print(ldrVal);
    lcd.print("    ");
    lcd.setCursor(0,1);
    lcd.print("ST: "); lcd.print(currentStatus);
    lcd.print("      ");
    lastLCDUpdate = millis();
  }

  // 5. IMMEDIATE COMMAND RECEIVE
  if (Serial.available() > 0) {
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();
    if (cmd.indexOf("F_ON") >= 0) digitalWrite(FAN_RELAY, LOW);
    else if (cmd.indexOf("F_OFF") >= 0) digitalWrite(FAN_RELAY, HIGH);
    if (cmd.indexOf("L_ON") >= 0) digitalWrite(LED_RELAY, LOW);
    else if (cmd.indexOf("L_OFF") >= 0) digitalWrite(LED_RELAY, HIGH);
  }

  delay(50); // Minimal delay for overall stability
}