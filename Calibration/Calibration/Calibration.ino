#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include <Adafruit_INA219.h>

// 0 if PT, 1 if flow sensor.
volatile int calibrationCase = 0;

// MODIFY HERE ACCORDING TO CALIBRATION VALUES
// reset them to a = 1, b = 0 if recalibrate!
volatile float PT_a = 1;
volatile float PT_b = 0;
volatile float FL_a = 1;
volatile float FL_b = 0;


volatile int timerInterval = 10;
Adafruit_INA219 ina219;
#define INTERRUPT_PIN 14
#define PRESSURE_PIN 27
#define PWM_PIN 23
#define PWM_FREQ 5000     // Frequency for PWM signal
#define PWM_RESOLUTION 8  // Resolution for PWM signal in bits
#define PWM_CHANNEL 0     // PWM channel

volatile unsigned long interruptCounter = 0;
unsigned long lastResetTime = 0;
volatile unsigned long lastInterruptTime = 0;
LiquidCrystal_I2C lcd(0x27, 16, 2);  // Change the 0x27  i2c address to your i2c address
LiquidCrystal_I2C lcd2(0x26, 16, 2);


void IRAM_ATTR handleInterrupt() {
  interruptCounter++;
  // lastInterruptTime = millis();  // Record the time of the last pulse
}

void lcdPrint(float current_mA, float pressureValue, float flowRate) {
  lcd.setCursor(0, 0);
  lcd.print("flow rate: ");
  lcd.print(flowRate);
  lcd.print("       ");
  lcd.setCursor(0, 1);
  lcd.print("Pressure: ");
  lcd.print(pressureValue);
  lcd.print("          ");
  lcd2.setCursor(0, 0);
  lcd2.print("I: ");
  lcd2.print(current_mA);
  lcd2.print(" mA");
  lcd2.print("          ");
  lcd2.setCursor(0, 1);
  lcd2.print("Use Serial Monitor");
  lcd2.print(current_mA);
  lcd2.print("mA");
  lcd2.print("          ");
  return;
}

void setup() {
  pinMode(INTERRUPT_PIN, INPUT_PULLDOWN);
  attachInterrupt(digitalPinToInterrupt(INTERRUPT_PIN), handleInterrupt, RISING);
  pinMode(PRESSURE_PIN, INPUT_PULLDOWN);

  if (!ina219.begin()) {
    Serial.println("Failed to find INA219 chip");
    // while (1) { delay(10); }
  }
  Wire.begin(21, 22);
  Serial.begin(115200);


  // Set up PWM on PWM_PIN
  pinMode(PWM_PIN, OUTPUT);
  ledcSetup(PWM_CHANNEL, PWM_FREQ, PWM_RESOLUTION);
  ledcAttachPin(PWM_PIN, PWM_CHANNEL);
  ledcWrite(PWM_CHANNEL, 255);

  if (calibrationCase == 1) {
    timerInterval = 1000;

  }

  // // lcd.begin(16,2);
  // // lcd.begin(16, 2);
  // lcd.init();
  // lcd.backlight();

  // lcd2.init();
  // lcd2.backlight();


  // lcd.clear();
  // lcd2.clear();

  // lcd.setCursor(1, 0);
  // // lcd.print("Hello, world!");
  // lcd.print("Air Supply");
  // lcd.setCursor(3, 1);
  // lcd.print("Screen 1");

  // lcd2.setCursor(1, 0);
  // lcd2.print("Air Supply");
  // lcd2.setCursor(3, 1);
  // lcd2.print("Screen 2");
  // delay(1000);

  // lcd.clear();
  // lcd2.clear();
}

void loop() {
  unsigned long currentTime = millis();

  // If 1 second has passed since the last reset
  if (currentTime - lastResetTime >= timerInterval) {
    lastResetTime = currentTime;

    if (calibrationCase == 0) {
      timerInterval = 10;
         // Read and print the pressure transducer value
    float pressureValue = analogRead(PRESSURE_PIN) * PT_a + PT_b;
    // Read and calculate flow rate
 

      Serial.println(pressureValue);
    } else if (calibrationCase == 1) {
      timerInterval = 1000;

      noInterrupts();
     // get current meter value
      float count = interruptCounter;
      interruptCounter = 0;
      unsigned long timeSinceLastPulse = currentTime - lastInterruptTime;
      interrupts();
      
      float flowRate = count * FL_a + FL_b;

      Serial.println(flowRate);
    }

    // lcdPrint(current_mA, pressureValue, flowRate);

  } else {
   if (Serial.available() > 0) {
     String inputString = Serial.readStringUntil('\n');
     inputString.trim();
     int inputInt = inputString.toInt();

     if (inputInt == 0) {
       calibrationCase = 0;
     } else if (inputInt == 1) {
       calibrationCase = 1;
     }
   }
  }
}
