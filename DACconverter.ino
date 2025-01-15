#include <Wire.h>

// DAC I2C Address (default: 0x60 for Gravity I2C 12-Bit DAC Module)
#define DAC_I2C_ADDRESS 0x60 

void setup() {
  Wire.begin();                // Initialize I2C communication
  Serial.begin(9600);          // Initialize serial communication
  Serial.println("Gravity I2C DAC Module Control");
  Serial.println("Enter a percentage (0 to 100) to set the output voltage:");
}

void loop() {
  // Check if data is available from the serial monitor
  if (Serial.available() > 0) {
    String input = Serial.readStringUntil('\n'); // Read user input
    input.trim(); // Remove any trailing newline or spaces

    int percentage = input.toInt(); // Convert input to integer

    // Validate input percentage
    if (percentage >= 0 && percentage <= 100) {
      float voltage = (percentage / 100.0) * 5.0; // Convert percentage to voltage
      setDACOutput(voltage);                      // Set DAC output
      Serial.print("Output voltage set to: ");
      Serial.print(voltage, 2); // Print voltage with 2 decimal places
      Serial.println(" V");
    } else {
      Serial.println("Invalid input. Enter a value between 0 and 100.");
    }
  }
}

// Function to set the DAC output voltage
void setDACOutput(float voltage) {
  // Convert voltage to a 12-bit value (0-4095)
  int dacValue = (voltage / 5.0) * 4095;
  
  // Prepare data to send to the DAC
  byte highByte = (dacValue >> 8) & 0xFF; // Extract the high 4 bits
  byte lowByte = dacValue & 0xFF;        // Extract the low 8 bits
  
  // Send the data to the DAC module via I2C
  Wire.beginTransmission(DAC_I2C_ADDRESS);
  Wire.write(highByte); // High byte first
  Wire.write(lowByte);  // Low byte second
  Wire.endTransmission();
}
