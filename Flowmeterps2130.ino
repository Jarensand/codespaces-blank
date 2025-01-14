// FM-PS2130 Flow Sensor Example Code
// Connect the sensor's signal pin to pin 2 on the Arduino
// (Pin 2 is used because it supports interrupts on most Arduino boards)

#define FLOW_SENSOR_PIN 2   // The pin connected to the flow sensor signal wire
#define CALIBRATION_FACTOR 6000 // Calibration factor from the sensor datasheet (adjust if needed)

volatile uint32_t pulseCount = 0; // Counter for the number of pulses
float flowRate = 0.0;             // Flow rate in liters per minute
float totalLiters = 0.0;          // Total liters passed
unsigned long lastTime = 0;       // Time of the last calculation

void setup() {
  Serial.begin(9600);          // Start serial communication for debugging
  pinMode(FLOW_SENSOR_PIN, INPUT_PULLUP); // Set the sensor pin as an input with an internal pullup
  attachInterrupt(digitalPinToInterrupt(FLOW_SENSOR_PIN), pulseCounter, RISING); // Attach interrupt to count pulses
  
  Serial.println("FM-PS2130 Flow Sensor Initialized");
}

void loop() {
  unsigned long currentTime = millis(); // Current time in milliseconds
  unsigned long elapsedTime = currentTime - lastTime; // Time since the last calculation

  if (elapsedTime >= 1000) { // Update every 1 second
    detachInterrupt(digitalPinToInterrupt(FLOW_SENSOR_PIN)); // Disable interrupts during calculation
    
    // Calculate flow rate in liters per minute
    flowRate = (pulseCount / (float)elapsedTime) * 1000.0 / CALIBRATION_FACTOR;

    // Calculate total liters passed
    totalLiters += (flowRate / 60.0); // Convert flow rate to liters per second
    
    Serial.print("Flow rate: ");
    Serial.print(flowRate, 4); // Display flow rate with 2 decimal places
    Serial.println(" L/min");

    Serial.print("Total volume: ");
    Serial.print(totalLiters, 4); // Display total volume with 2 decimal places
    Serial.println(" L");

    pulseCount = 0; // Reset pulse count
    lastTime = currentTime; // Update last time
    
    attachInterrupt(digitalPinToInterrupt(FLOW_SENSOR_PIN), pulseCounter, RISING); // Re-enable interrupts
  }
}

// Interrupt Service Routine (ISR)
void pulseCounter() {
  pulseCount++; // Increment the pulse count
}
