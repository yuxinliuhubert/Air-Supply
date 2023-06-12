#define INTERRUPT_PIN 4
#define PRESSURE_PIN 36
#include <LiquidCrystal_I2C.h>
#include <Wire.h> 
volatile unsigned long interruptCounter = 0;
unsigned long lastResetTime = 0;
volatile unsigned long lastInterruptTime = 0;
 LiquidCrystal_I2C lcd(0x27,16,2);  // Change the 0x27  i2c address to your i2c address

void IRAM_ATTR handleInterrupt() {
  interruptCounter++;
  // lastInterruptTime = millis();  // Record the time of the last pulse
}

void setup() {
  pinMode(INTERRUPT_PIN, INPUT_PULLDOWN);
  attachInterrupt(digitalPinToInterrupt(INTERRUPT_PIN), handleInterrupt, RISING);
  pinMode(PRESSURE_PIN, INPUT_PULLDOWN);
  
  Serial.begin(115200);
  
  // lcd.begin(16,2);
  // lcd.begin(16, 2);
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(1,0);
  lcd.print("Hello, world!");
  lcd.setCursor(3,1);
  lcd.print("Air Supply");
  delay(1000);
  lcd.clear();
}

void loop() {
  unsigned long currentTime = millis();

  // If 1 second has passed since the last reset
  if (currentTime - lastResetTime >= 1000) {
    lastResetTime = currentTime;

    // Temporarily disable interrupts while reading and resetting interruptCounter
    noInterrupts();
    unsigned long count = interruptCounter;
    interruptCounter = 0;
    unsigned long timeSinceLastPulse = currentTime - lastInterruptTime;
    interrupts();

     // Read and print the pressure transducer value
    int pressureValue = analogRead(PRESSURE_PIN);
    Serial.print("Pressure transducer value: ");
    Serial.println(pressureValue);

      Serial.println(count);
      // lcd.clear();
      lcd.setCursor(2,0);
      lcd.print("flow count:");
      lcd.print(count);
      lcd.print("          ");
      lcd.setCursor(2,1);
      lcd.print("Pressure: ");
      lcd.print(pressureValue);
      lcd.print("          ");

    // // Check if a pulse has been received in the last second
    // if (timeSinceLastPulse < 1000) {
    //   Serial.print("Interrupt count in the last second: ");
    //   Serial.println(count);
    // } else {
    //   Serial.println("No flow detected in the last second.");
    // }

    // delay(30);
  }
}

