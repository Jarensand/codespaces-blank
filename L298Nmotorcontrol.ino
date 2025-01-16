
const int enA = 11;   // Enable pin for motor (PWM pin)
const int in1 = 9;   // Input 1 for motor
const int in2 = 10;   // Input 2 for motor

void setup() {
  Serial.begin(9600);
  pinMode(enA, OUTPUT);
  pinMode(in1, OUTPUT);
  pinMode(in2, OUTPUT);
  // Stop the motor initially
  digitalWrite(in1, LOW);
  digitalWrite(in2, LOW);
  analogWrite(enA, 0);

  Serial.println("L298N Motor Controller Initialized");
  Serial.println("Enter commands in the format: [direction] [speed]");
  Serial.println("Example: f 75");

  delay(1000);
}

void loop() {
  
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