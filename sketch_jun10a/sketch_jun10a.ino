#include <LiquidCrystal_I2C.h>
#include <Wire.h> 
#include <Wire.h>
#include <Adafruit_INA219.h>

Adafruit_INA219 ina219;

#define INTERRUPT_PIN 4
#define PRESSURE_PIN 36

#define PWM_PIN 15
#define PWM_FREQ 5000  // Frequency for PWM signal
#define PWM_RESOLUTION 8  // Resolution for PWM signal in bits
#define PWM_CHANNEL 0  // PWM channel


volatile unsigned long interruptCounter = 0;
unsigned long lastResetTime = 0;
volatile unsigned long lastInterruptTime = 0;
 LiquidCrystal_I2C lcd(0x27,16,2);  // Change the 0x27  i2c address to your i2c address

 // Declare global variables for duty cycle
int dutyCycle[] = {0, 178,178,255,255};  // Corresponds to 0%, 30%, 60%, 80% and 100% duty cycle in 8-bit resolution
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
  
  Serial.begin(115200);

  // Set up PWM on PWM_PIN
  pinMode(PWM_PIN, OUTPUT);
  ledcSetup(PWM_CHANNEL, PWM_FREQ, PWM_RESOLUTION);
  ledcAttachPin(PWM_PIN, PWM_CHANNEL);
  
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
      lcd.setCursor(0,0);
      lcd.print("flow:");
      lcd.print(count);
      lcd.print("       ");

      float current_mA = ina219.getCurrent_mA();
    Serial.print("Current: "); Serial.print(current_mA); Serial.println(" mA");
    lcd.setCursor(9,0);
    // lcd.print("Current: ");
    lcd.print(current_mA);
    lcd.print("mA");
    lcd.print("          ");


      lcd.setCursor(1,1);
      lcd.print("Pressure: ");
      lcd.print(pressureValue);
      lcd.print("          ");

     
      // Change duty cycle of PWM signal
  ledcWrite(PWM_CHANNEL, dutyCycle[dutyIndex]);

  dutyIndex = (dutyIndex + 1) % 5;  // Cycle through duty cycle values

  // delay(1000);  // Wait for 1 second before changing duty cycle

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

