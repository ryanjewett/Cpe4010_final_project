#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Servo.h>

const int RED_LED = 3;             // Pin for Red LED
const int GREEN_LED = 2;           // Pin for Green LED
const int BUTTON_PIN = 4;          // Pin for Button
const int BUZZER_PIN = 5;          // Pin for Buzzer
const int ULTRASONIC_SERVO_PIN = 52; // Pin for Servo controlling ultrasonic sensor
const int SWEEPER_SERVO_PIN = 53;  // Pin for Sweeper Servo
const int TRIGGER_PIN = 6;         // Pin for Ultrasonic Trigger
const int ECHO_PIN = 7;            // Pin for Ultrasonic Echo

LiquidCrystal_I2C lcd(0x27, 16, 2); // Adjust the address (0x27) if needed
Servo ultrasonicServo;
Servo sweeperServo;

bool gameActive = false;           // Flag for game state
bool redLight = false;             // Flag for red light state
unsigned long lastToggleTime = 0;  // Time of last light toggle
unsigned long redStartTime = 0;    // Time when red light started
int remainingTime = 60;            // Countdown timer for game
bool playerDetected = false;       // Flag for player detection during red light
unsigned long debounceDelay = 200; // Debounce delay for the button
unsigned long lastButtonPressTime = 0; // Time of last button press
long previousDistance = -1;

void setup() {
  pinMode(RED_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);  
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(TRIGGER_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  lcd.init();
  lcd.backlight();
  ultrasonicServo.attach(ULTRASONIC_SERVO_PIN);
  sweeperServo.attach(SWEEPER_SERVO_PIN);

  ultrasonicServo.write(160); 
  sweeperServo.write(180);    

  // Display ready message
  lcd.setCursor(0, 0);
  lcd.print("Press Button");
  Serial.begin(9600);
}

void loop() {
  if (digitalRead(BUTTON_PIN) == LOW && (millis() - lastButtonPressTime) > debounceDelay) {
    lastButtonPressTime = millis(); 
    if (!gameActive) {
      gameActive = true;
      remainingTime = 60;  
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Game Started");
      delay(1000);  
    }
  }

  if (gameActive) {
    while(remainingTime > 0) {
      if (redLight) {
        digitalWrite(RED_LED, HIGH);
        digitalWrite(GREEN_LED, LOW);
        tone(BUZZER_PIN, 500);
        delay(200);
        tone(BUZZER_PIN,0);  
        moveUSDown(); 
        for(int x = 0; x < 10; x++){
          playerDetected = checkMovement();
          if (playerDetected) {
            loseGame();
            remainingTime = 0; 
            }
          delay(200);  
          }
        redLight = false;
      } else {
        redLight = true;
        digitalWrite(GREEN_LED, HIGH);
        digitalWrite(RED_LED, LOW);
        tone(BUZZER_PIN, 1000);
        delay(100);
        tone(BUZZER_PIN,0); 
        moveUSUp();  
        delay(1900);  
      }
      remainingTime--;
      updateLCD(); 
    } 
  }
}


void moveUSDown() {
  ultrasonicServo.write(160);  
}


void moveUSUp() {
  ultrasonicServo.write(50);  
}

bool checkMovement() {
  long duration, distance;
  
  
  digitalWrite(TRIGGER_PIN, LOW);  
  delayMicroseconds(2);
  
  
  digitalWrite(TRIGGER_PIN, HIGH);  
  delayMicroseconds(10);
  digitalWrite(TRIGGER_PIN, LOW);
  
  
  duration = pulseIn(ECHO_PIN, HIGH);
  distance = (duration / 2) / 29.1;  // Calculate distance in cm

  if (previousDistance != -1 && distance != previousDistance) {
    if (abs(previousDistance - distance) > 5) {
      previousDistance = distance;  // Update the previous distance
      return true;  // Player has moved
    }
  }
  previousDistance = distance;  // Update the previous distance for next check
  return false;  // No movement detected
}


void loseGame() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("You Lost!");
  sweeperServo.write(110);  
  delay(2000);
  gameActive = false;
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Press Button");
}


void updateLCD() {
  lcd.setCursor(0, 1);
  lcd.print("Time: ");
  lcd.print(remainingTime);
  lcd.print("s");
}
