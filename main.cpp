#include <Arduino.h>
#include <SPI.h>
#include <MFRC522.h>
#include <TM1637.h>

// --- Pin Definitions ---
#define BUTTON1_PIN 2
#define BUTTON2_PIN 3
#define CLK 4
#define DIO 5
#define SS_PIN 10
#define RST_PIN 9

MFRC522 mfrc522(SS_PIN, RST_PIN);
TM1637 tm1637(CLK, DIO);

// --- State Machine ---
enum State { IDLE, READY, RUNNING, FINISHED };
State currentState = IDLE;

unsigned long startTime = 0;
unsigned long finalTime = 0;
int currentPlayer = 0; // Stores current player (0 = Anonymous)

// Variables for blinking effect
unsigned long blinkTimer = 0;
bool displayVisible = true;

// Button states for edge detection (debounce/single press)
bool lastBtn1State = LOW;
bool lastBtn2State = LOW;

// --- NFC UIDs ---
String tag1 = "53 E4 D7 1D";
String tag2 = "AA BB CC DD"; 
String tag3 = "11 22 33 44"; 

// ---------------------------------------------------------
// Helper functions for the Display
// ---------------------------------------------------------

void displayTime(unsigned long timeMs) {
  int seconds = (timeMs / 1000) % 100;
  int hundredths = (timeMs / 10) % 100;
  
  int8_t dispData[4];
  dispData[0] = (seconds / 10 == 0) ? 0x7f : (seconds / 10); // Blank leading zero
  dispData[1] = seconds % 10;
  dispData[2] = hundredths / 10;
  dispData[3] = hundredths % 10;
  
  tm1637.point(POINT_ON); 
  tm1637.display(dispData);
}

void displayUser(int id) {
  tm1637.clearDisplay();
  tm1637.start();
  tm1637.writeByte(ADDR_AUTO);
  tm1637.stop();
  tm1637.start();
  tm1637.writeByte(STARTADDR);
  
  tm1637.writeByte(0x3E); // Character 'U'
  tm1637.writeByte(0x00); // Space
  
  if (id > 9) {
      tm1637.writeByte(tm1637.coding(id / 10)); 
      tm1637.writeByte(tm1637.coding(id % 10)); 
  } else {
      tm1637.writeByte(0x00);                   
      tm1637.writeByte(tm1637.coding(id));      
  }
  
  tm1637.stop();
  tm1637.start();
  tm1637.writeByte(tm1637.Cmd_DispCtrl); 
  tm1637.stop();
}

// ---------------------------------------------------------
// Main Program
// ---------------------------------------------------------

void setup() {
  Serial.begin(115200);
  pinMode(BUTTON1_PIN, INPUT);
  pinMode(BUTTON2_PIN, INPUT);

  tm1637.init();
  tm1637.set(BRIGHT_TYPICAL);
  tm1637.clearDisplay();

  SPI.begin();
  mfrc522.PCD_Init();
}

void loop() {
  bool btn1State = digitalRead(BUTTON1_PIN);
  bool btn2State = digitalRead(BUTTON2_PIN);

  switch (currentState) {
    
    case IDLE:
      // Option A: Scan NFC Tag -> Display User & wait for button
      if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
        String content = "";
        for (byte i = 0; i < mfrc522.uid.size; i++) {
          content += String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
          content += String(mfrc522.uid.uidByte[i], HEX);
        }
        content.toUpperCase();
        String uid = content.substring(1);

        if (uid == tag1) { currentPlayer = 1; }
        else if (uid == tag2) { currentPlayer = 2; }
        else if (uid == tag3) { currentPlayer = 3; }
        else { currentPlayer = 9; } 

        displayUser(currentPlayer);
        currentState = READY;
        delay(1000); 
      }
      // Option B: Anonymous Mode via Button 1 -> Direct Start!
      else if (btn1State == HIGH && lastBtn1State == LOW) {
        currentPlayer = 0; 
        
        // Direct countdown without displaying 'U0'
        tm1637.point(POINT_OFF);
        for (int i = 3; i > 0; i--) {
          int8_t countdownData[] = {0x7f, 0x7f, 0x7f, (int8_t)i};
          tm1637.display(countdownData);
          delay(1000);
        }
        
        tm1637.clearDisplay();
        startTime = millis();
        currentState = RUNNING;
      }
      break;

    case READY:
      // Starts the run for scanned NFC users
      if (btn1State == HIGH && lastBtn1State == LOW) {
        tm1637.point(POINT_OFF);
        for (int i = 3; i > 0; i--) {
          int8_t countdownData[] = {0x7f, 0x7f, 0x7f, (int8_t)i};
          tm1637.display(countdownData);
          delay(1000);
        }
        
        tm1637.clearDisplay();
        startTime = millis();
        currentState = RUNNING;
      }
      break;

    case RUNNING:
      { 
        unsigned long currentMillis = millis() - startTime;
        displayTime(currentMillis);

        // Stop the timer with Button 2
        if (btn2State == HIGH && lastBtn2State == LOW) {
          finalTime = currentMillis;
          
          // Prepare for the blinking end screen
          blinkTimer = millis(); 
          displayVisible = true;
          
          currentState = FINISHED;
        }
      }
      break;

    case FINISHED:
      // 1. Non-blocking blinking of the final time every 500ms
      if (millis() - blinkTimer >= 500) {
        blinkTimer = millis();
        displayVisible = !displayVisible; // Toggle visibility state
        
        if (displayVisible) {
          displayTime(finalTime);
        } else {
          tm1637.point(POINT_OFF);
          tm1637.clearDisplay();
        }
      }

      // 2. Check if either button is pressed to reset to IDLE
      if ((btn1State == HIGH && lastBtn1State == LOW) || 
          (btn2State == HIGH && lastBtn2State == LOW)) {
        
        currentPlayer = 0; // Reset for next run
        currentState = IDLE; 
        
        tm1637.point(POINT_OFF);
        tm1637.clearDisplay();
        delay(200); // Brief pause to prevent accidental immediate restart
      }
      break;
  }

  // Save button states for the next loop iteration (edge detection)
  lastBtn1State = btn1State;
  lastBtn2State = btn2State;
}
