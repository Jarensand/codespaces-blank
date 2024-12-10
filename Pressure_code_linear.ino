#include <Wire.h>
#include <SoftwareSerial.h>
#define ledPin 6
#define motorPin 9
int chad = 2;
SoftwareSerial bt(0,1); /* (Rx,Tx) */ 
double flow; //Water flow L/Min 
int flowsensor = 24; 
volatile int pulseCount = 0; // For flow sensor pulse count
float flowRate = 0.0; // Flow rate in mL/min
float totalVolume = 0.0; // Total volume in mL
unsigned long currentTime;
unsigned long lastTime;
unsigned long pulse_freq;
float Lower_Range_Limit = 0.0;
float Upper_Range_Limit = 65;  // kPa
float fadc;
float uADC;
float Range;
float Pressure;
float Pressure1;
float Pressure2;
float Pressure3;
float Pressurecal;
float Temp;
unsigned char buf[6];
unsigned long dat;

const int enA = 11;   // Enable pin for motor (PWM pin)
const int in1 = 9;   // Input 1 for motor
const int in2 = 10;   // Input 2 for motor

// Interrupt service routine for counting pulses
void pulseCounter() {
    pulseCount++;
}

void setup() {
  pinMode(flowsensor, INPUT);
  Serial.begin(9600);
  attachInterrupt(digitalPinToInterrupt(flowsensor), pulseCounter, RISING); // Setup Interrupt
  currentTime = millis();
  lastTime = currentTime;
  Wire.begin(); // Initialize the I2C communication
  pinMode(ledPin, OUTPUT);
  pinMode(motorPin, OUTPUT);
  bt.begin(9600); // Add a brief delay to allow the serial monitor to initialize
  pinMode(enA, OUTPUT);
  pinMode(in1, OUTPUT);
  pinMode(in2, OUTPUT);

  // Stop the motor initially
  digitalWrite(in1, LOW);
  digitalWrite(in2, LOW);
  analogWrite(enA, 0);
  delay(1000);
}

void loop() {
  delay(990);
  int Pressure3 = Pressure2;
  Wire.beginTransmission(0x6D);
  Wire.write(0x06);
  Wire.endTransmission();
  Wire.requestFrom(0x6D, 6);

  if (Wire.available() >= 6) {
    for (int i = 0; i < 6; i++) {
      buf[i] = Wire.read();
    }
    dat = buf[0];
    dat <<= 8;
    dat += buf[1];
    dat <<= 8;
    dat += buf[2];
    if (dat & 0x800000) 
    {
      fadc = dat - 16777216.0;
    } else {
      fadc = dat;
    }
    uADC = 3.3 * fadc / 8388608.0;
    Range = Upper_Range_Limit - Lower_Range_Limit;
    Pressure1 =  Upper_Range_Limit * (0.8 * (fadc / (8388608 - 0.1)))-5.5;
    Pressure2 =  (65 / 0.8 * (fadc / 8388608 - 0.1))-0.45;
    Pressure = Pressure2 - Pressure3;
    dat = buf[3];
    dat <<= 8;
    dat += buf[4];
    dat <<= 8;
    dat += buf[5];
    if (dat & 0x800000) {
      fadc = dat - 16777216.0;
    } else {
      fadc = dat;
    }
    Temp = 25.0 + fadc / 65536.0;
    
    // Flow sensor calculations
    unsigned long elapsedTime = millis() - lastTime;
    if (elapsedTime >= 1000) {
      detachInterrupt(digitalPinToInterrupt(flowsensor));
      float flowLitersPerMinute = ((float)pulseCount / 5880.0) * 60.0; // Example value for pulses per liter
      flowRate = flowLitersPerMinute * 1000.0; // Convert to mL/min
      totalVolume += (flowRate / 60.0); // Add volume for one second
      pulseCount = 0;
      lastTime = millis();
      attachInterrupt(digitalPinToInterrupt(flowsensor), pulseCounter, RISING);

      Serial.print("Flow Rate: ");
      Serial.print(flowRate);
      Serial.println(" mL/min");
      Serial.print("Total Volume: ");
      Serial.print(totalVolume);
      Serial.println(" mL");
    }

    Serial.print("Pressure (Kpa)");
    Serial.println(Pressure2, 3);
    float volume = flowRate * 50 / 60;
    float comp = volume / Pressure;
    Serial.println(comp, 3);
  }

  if (Serial.available()) {
      String command = Serial.readStringUntil('\n'); // Read input until newline
      command.trim(); // Trim whitespace

      // Parse and execute command
      if (command.startsWith("f")) {
        int speed = command.substring(1).toInt(); // Extract speed value
        setMotor('f', constrain(speed, 0, 100)); // Forward with specified speed
      } else if (command.startsWith("b")) {
        int speed = command.substring(1).toInt(); // Extract speed value
        setMotor('b', constrain(speed, 0, 100)); // Backward with specified speed
      } else if (command == "s") {
        setMotor('s', 0); // Stop the motor
      } else {
        Serial.println("Invalid command!");
      }
  }

  delay(10); // Delay between readings (adjust as needed)
}

void setMotor(char direction, int speed) {
  int pwmValue = map(speed, 0, 100, 0, 255); // Map speed (0-100) to PWM (0-255)
  switch (direction) {
    case 'f': // Forward
      digitalWrite(in1, HIGH);
      digitalWrite(in2, LOW);
      analogWrite(enA, pwmValue);
      Serial.print("Forward at speed: ");
      Serial.println(speed);
      break;
    case 'b': // Backward
      digitalWrite(in1, LOW);
      digitalWrite(in2, HIGH);
      analogWrite(enA, pwmValue);
      Serial.print("Backward at speed: ");
      Serial.println(speed);

      break;
    case 's': // Stop
      digitalWrite(in1, LOW);
      digitalWrite(in2, LOW);
      analogWrite(enA, 0);
      Serial.println("Motor stopped.");
      break;
    default:
      Serial.println("Invalid direction!");
  }
}
