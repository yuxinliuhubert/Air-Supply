#define INTERRUPT_PIN 4
#define PRESSURE_PIN 36

volatile unsigned long interruptCounter = 0;
unsigned long lastResetTime = 0;
volatile unsigned long lastInterruptTime = 0;

void IRAM_ATTR handleInterrupt() {
  interruptCounter++;
  // lastInterruptTime = millis();  // Record the time of the last pulse
}

void setup() {
  pinMode(INTERRUPT_PIN, INPUT_PULLDOWN);
  attachInterrupt(digitalPinToInterrupt(INTERRUPT_PIN), handleInterrupt, RISING);
  pinMode(PRESSURE_PIN, INPUT_PULLDOWN);
  
  Serial.begin(115200);
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

      Serial.println(count);

    // // Check if a pulse has been received in the last second
    // if (timeSinceLastPulse < 1000) {
    //   Serial.print("Interrupt count in the last second: ");
    //   Serial.println(count);
    // } else {
    //   Serial.println("No flow detected in the last second.");
    // }

    // Read and print the pressure transducer value
    int pressureValue = analogRead(PRESSURE_PIN);
    Serial.print("Pressure transducer value: ");
    Serial.println(pressureValue);
    delay(30);
  }
}

