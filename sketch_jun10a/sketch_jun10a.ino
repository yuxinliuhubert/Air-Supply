#include <LiquidCrystal_I2C.h>
#include <Wire.h> 
#include <Adafruit_INA219.h>

Adafruit_INA219 ina219;

#define INTERRUPT_PIN 14
#define PRESSURE_PIN 27

#define PWM_PIN 23
#define PWM_FREQ 5000  // Frequency for PWM signal
#define PWM_RESOLUTION 8  // Resolution for PWM signal in bits
#define PWM_CHANNEL 0  // PWM channel

#define a 0.0232
#define b -10.9232
volatile unsigned long interruptCounter = 0;
unsigned long lastResetTime = 0;
volatile unsigned long lastInterruptTime = 0;
 LiquidCrystal_I2C lcd(0x27,16,2);  // Change the 0x27  i2c address to your i2c address
LiquidCrystal_I2C lcd2(0x26,16,2);
 // Declare global variables for duty cycle
int dutyCycle[] = {0,178};  // Corresponds to 0%, 30%, 60%, 80% and 100% duty cycle in 8-bit resolution
int dutyIndex = 0;  // Initialize index for dutyCycle array

void IRAM_ATTR handleInterrupt() {
  interruptCounter++;
  // lastInterruptTime = millis();  // Record the time of the last pulse
}

void setup() {
  pinMode(INTERRUPT_PIN, INPUT_PULLDOWN);
  attachInterrupt(digitalPinToInterrupt(INTERRUPT_PIN), handleInterrupt, RISING);
  pinMode(PRESSURE_PIN, INPUT_PULLDOWN);

  if (! ina219.begin()) {
    Serial.println("Failed to find INA219 chip");
    while (1) { delay(10); }
  }
  Wire.begin(21,22);
  Serial.begin(115200);

  // Set up PWM on PWM_PIN
  pinMode(PWM_PIN, OUTPUT);
  ledcSetup(PWM_CHANNEL, PWM_FREQ, PWM_RESOLUTION);
  ledcAttachPin(PWM_PIN, PWM_CHANNEL);
  ledcWrite(PWM_CHANNEL, 0);
  
  // lcd.begin(16,2);
  // lcd.begin(16, 2);
  lcd.init();
  lcd.backlight();

  lcd2.init();
  lcd2.backlight();


  lcd.clear();
  lcd2.clear();

  lcd.setCursor(1,0);
  // lcd.print("Hello, world!");
  lcd.print("Air Supply");
  lcd.setCursor(3,1);
  lcd.print("Screen 1");

  lcd2.setCursor(1,0);
  lcd2.print("Air Supply");
  lcd2.setCursor(3,1);
  lcd2.print("Screen 2");
  delay(1000);

  lcd.clear();
  lcd2.clear();


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
    double pressureValue = analogRead(PRESSURE_PIN)*a+b;
    Serial.print("Pressure transducer value: ");
    Serial.println(pressureValue);

    Serial.print("flow rate:");
      Serial.println(count);
      // lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("flow rate: ");
      lcd.print(count);
      lcd.print("       ");

      lcd.setCursor(0,1);
      lcd.print("Pressure: ");
      lcd.print(pressureValue);
      lcd.print("          ");

      float current_mA = ina219.getCurrent_mA();
    Serial.print("Current: "); Serial.print(current_mA); Serial.println(" mA"); 
    lcd2.setCursor(0,0);
    lcd2.print("I: ");
    lcd2.print(current_mA);
    lcd2.print(" mA");
    lcd2.print("          ");

    lcd2.setCursor(0,1);
    lcd2.print("Use Serial Monitor");
    lcd2.print(current_mA);
    lcd2.print("mA");
    lcd2.print("          ");

    Serial.println("Type in number between 0 (close) and 255 (open) to control valve!");
    Serial.println();


     
      // Change duty cycle of PWM signal
  // ledcWrite(PWM_CHANNEL, dutyCycle[dutyIndex]);

  // dutyIndex = (dutyIndex + 1) % 2;  // Cycle through duty cycle values

  // delay(1000);  // Wait for 1 second before changing duty cycle

    // // Check if a pulse has been received in the last second
    // if (timeSinceLastPulse < 1000) {
    //   Serial.print("Interrupt count in the last second: ");
    //   Serial.println(count);
    // } else {
    //   Serial.println("No flow detected in the last second.");
    // }

    // delay(30);
  } else {
      if(Serial.available() > 0) {
      String inputString = Serial.readStringUntil('\n');
      inputString.trim();
      int inputInt = inputString.toInt();

      if(inputInt >= 0 && inputInt <= 255) {
        ledcWrite(PWM_CHANNEL, inputInt);
        Serial.print("Updated duty cycle: ");
        Serial.println(inputInt);
      }
      else {
        Serial.println("Invalid input! Please enter a number between 0 and 255.");
      }
      }
  }
}

