// Pin Definitions
#define FLOW_SENSOR_PIN 2   // Pin for flow sensor input (interrupt)
#define PRESSURE_SENSOR_PIN A0 // Analog pin for pressure sensor input
#define ENA 9                // Enable pin for motor (PWM pin)
#define IN1 7                // Control pin 1 for motor
#define IN2 8                // Control pin 2 for motor

// Constants
const float FLOW_CALIBRATION_FACTOR = 7.5; // Flow sensor calibration (pulses/mL)
const int PRESSURE_SENSOR_MAX = 1023;     // Max ADC value
const float PRESSURE_SENSOR_MAX_KPA = 100.0; // Max pressure in kPa

// Variables
volatile int pulseCount = 0; // Flow sensor pulse counter
float flowRate = 0.0;        // Flow rate in mL/min
float volume = 0.0;          // Volume in liters
float pressure = 0.0;        // Pressure in kPa
float compliance = 0.0;      // Compliance (mL/min per kPa)
bool overrideMode = false;   // User override mode
int overrideSpeed = 0;       // Override speed (0-100%)

// Interrupt service routine for flow sensor
void pulseCounter() {
  pulseCount++;
}

void setup() {
  // Initialize pins
  pinMode(FLOW_SENSOR_PIN, INPUT_PULLUP);
  pinMode(PRESSURE_SENSOR_PIN, INPUT);
  pinMode(ENA, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);

  // Initialize serial communication
  Serial.begin(9600);
  Serial.println("System Initialized");
  Serial.println("Press 'override' to enable manual control.");
  attachInterrupt(digitalPinToInterrupt(FLOW_SENSOR_PIN), pulseCounter, RISING); // Attach interrupt
}

void loop() {
  if (!overrideMode) {
    enableOverride();
    calculateFlow();
    readPressure();
    calculateCompliance();
    adjustMotorSpeed();
  } else {
    handleOverride();
  }

  delay(1000); // Update every second
}

// Function to calculate flow rate and volume
void calculateFlow() {
  noInterrupts(); // Disable interrupts to safely read pulse count
  int pulses = pulseCount;
  pulseCount = 0;
  interrupts(); // Re-enable interrupts

  flowRate = (pulses / FLOW_CALIBRATION_FACTOR) * 60.0; // mL/min
  volume += (flowRate / 60000.0); // Convert mL/min to liters per second
  Serial.print("Flow Rate: ");
  Serial.print(flowRate);
  Serial.println(" mL/min");
  Serial.print("Volume: ");
  Serial.print(volume, 3);
  Serial.println(" L");
}

// Function to read pressure sensor value
void readPressure() {
  int adcValue = analogRead(PRESSURE_SENSOR_PIN);
  pressure = (adcValue / (float)PRESSURE_SENSOR_MAX) * PRESSURE_SENSOR_MAX_KPA;
  Serial.print("Pressure: ");
  Serial.print(pressure);
  Serial.println(" kPa");
}

// Function to calculate compliance
void calculateCompliance() {
  if (pressure > 0) {
    compliance = flowRate / pressure;
    Serial.print("Compliance: ");
    Serial.print(compliance, 3);
    Serial.println(" mL/min per kPa");
  } else {
    compliance = 0;
    Serial.println("Compliance: Undefined (Pressure is 0)");
  }
}

// Function to adjust motor speed based on compliance
void adjustMotorSpeed() {
  int speed = constrain(map(compliance, 0, 100, 0, 255), 0, 255); // Map compliance to PWM range
  analogWrite(ENA, speed);
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW); // Set motor direction forward
  Serial.print("Motor Speed: ");
  Serial.print((speed / 255.0) * 100);
  Serial.println("%");
}

// Function to handle user override
void handleOverride() {
  if (Serial.available() > 0) {
    String input = Serial.readStringUntil('\n');
    input.trim();

    if (input.equalsIgnoreCase("exit")) {
      overrideMode = false;
      Serial.println("Exiting override mode.");
    } else {
      int speed = input.toInt();
      if (speed >= 0 && speed <= 100) {
        overrideSpeed = speed;
        int pwmValue = map(overrideSpeed, 0, 100, 0, 255);
        analogWrite(ENA, pwmValue);
        digitalWrite(IN1, HIGH);
        digitalWrite(IN2, LOW);
        Serial.print("Motor set to ");
        Serial.print(overrideSpeed);
        Serial.println("% speed (Override).");
      } else {
        Serial.println("Invalid input. Enter a speed (0-100) or 'exit' to return to auto mode.");
      }
    }
  }
}

// Function to enable override mode
void enableOverride() {
  if (Serial.available() > 0) {
    String input = Serial.readStringUntil('\n');
    input.trim();

    if (input.equalsIgnoreCase("override")) {
      overrideMode = true;
      Serial.println("Override mode enabled. Enter motor speed (0-100):");
    }
  }
}