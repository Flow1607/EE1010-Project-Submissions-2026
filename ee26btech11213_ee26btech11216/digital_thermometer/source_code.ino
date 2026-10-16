#include <LiquidCrystal.h>

// Confirmed LCD pin mapping[cite: 2]
LiquidCrystal lcd(7,6, 5, 4, 3, 2);

const int PT100_PIN = A0; //[cite: 2]'
const float R_REF = 120.0; //[cite: 2]
const float V_REF = 5.0; //[cite: 2]

// Quadratic regression coefficients: T = a*v^2 + b*v + c[cite: 1]
const float A_COEFF = 82.25613492; //[cite: 1]
const float B_COEFF = -156.49781957; //[cite: 1]
const float C_COEFF = -78.32855762; //[cite: 1]

float voltage = 0.0; //[cite: 2]
float temperature = 0.0; //[cite: 2]

void setup() {
  lcd.begin(16, 2); //[cite: 2]
  lcd.clear(); //[cite: 2]

  Serial.begin(9600); //[cite: 2]
  Serial.println("Digital Thermometer"); //[cite: 2]
  Serial.println("-------------------"); //[cite: 2]
}

void loop() {
  // Average 25 ADC samples to suppress electrical noise and display jitter
  long adcSum = 0;
  const int SAMPLES = 25;
  for (int i = 0; i < SAMPLES; i++) {
    adcSum += analogRead(PT100_PIN);
    delay(4);
  }
  float avgAdc = (float)adcSum / SAMPLES;

  // Calculate voltage across PT100[cite: 2]
  voltage = avgAdc * (V_REF / 1023.0);

  // Polynomial temperature equation: T = a*v^2 + b*v + c[cite: 1]
  temperature = (A_COEFF * voltage * voltage) + (B_COEFF * voltage) + C_COEFF;

  // Display Line 1: Voltage with 6 decimal places (e.g., "V:2.418293V")
  lcd.setCursor(0, 0); //[cite: 2]
  lcd.print("V:");
  lcd.print(voltage, 6);
  lcd.print("V   ");

  // Display Line 2: Temperature with 6 decimal places (e.g., "T:25.123456C")
  lcd.setCursor(0, 1); //[cite: 2]
  lcd.print("T:");
  lcd.print(temperature, 6);
  lcd.print("C  ");

  // Output to Serial Monitor with 6 decimal places
  Serial.print("V: ");
  Serial.print(voltage, 6);
  Serial.print(" V | Temp: ");
  Serial.print(temperature, 6);
  Serial.println(" C");

  delay(500);
}