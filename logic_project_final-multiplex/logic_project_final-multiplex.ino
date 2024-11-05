int dataPin = 6;
int latchPin = 7;
int clockPin = 8;
int resetButtonPin = 11;
int clearPin = 2;
int extraButton = 12;

int buttonPinA = 9; 
int buttonPinB = 10; 

byte LEDsAllOff = 0x00;
byte LEDsAllOn = 0xFF;
byte LEDs2 = 0b10101010;
byte LEDs3 = 0b01010101;
byte LEDs4 = 0b11101111;

int buttonAState = LOW;
int lastButtonAState = LOW;

int buttonBState = LOW;
int lastButtonBState = LOW;

int resetButtonState = 0;
int lastResetButtonState = 0;

int countA = 0;
int countB = 0;
int maxCount = 15;
bool displayCountA = true; // Flag to alternate between countA and countB
int delayTime = 1000;

// 7-segment display pins
int segmentPins[] = {5, A1, A2, A3, A4, A5, 3}; // A1 to A5 for a to g, 3 for f, 5 for a
int decimalPointPin = 4; // Pin for the decimal point
int poten = A0;
int interval = 1;

// Segment patterns for digits 0-F
byte digitPatterns[] = {
  0b00000000, // 0
  0b00000110, // 1
  0b0111011, // 2
  0b00101111, // 3
  0b01100110, // 4
  0b01101101, // 5
  0b01111101, // 6
  0b00000111, // 7
  0b01111111, // 8
  0b01101111,  // 9
  0b01110111, // A
  0b01111100, // b
  0b01011001, // C
  0b00111110, // d
  0b01111001, // E
  0b01110001  // F
};

void setup() {
  Serial.begin(9600);
  pinMode(dataPin, OUTPUT);
  pinMode(latchPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(resetButtonPin, INPUT_PULLUP);

  pinMode(buttonPinA, INPUT_PULLUP);
  pinMode(buttonPinB, INPUT_PULLUP);

  pinMode(clearPin, OUTPUT);
  digitalWrite(clearPin, HIGH); // Ensure SRCLR is high to enable shift register
  pinMode(decimalPointPin, OUTPUT); // Initialize pin for decimal point


  // Initialize 7-segment display pins
  for (int i = 0; i < 7; i++) {
    pinMode(segmentPins[i], OUTPUT);
  }
}

void updateShiftRegister(int valueA, int valueB) {
  byte combinedValue = (valueA << 4) | (valueB & 0x0F);
  digitalWrite(latchPin, LOW);
  shiftOut(dataPin, clockPin, LSBFIRST, combinedValue);
  digitalWrite(latchPin, HIGH);
}

void clearShiftRegister() {
  digitalWrite(clearPin, LOW);  
  delay(1);                     
  digitalWrite(clearPin, HIGH); 
  updateShiftRegister(0, 0);  
}


void displayDigit(int digit, bool decimalPoint) {
  byte pattern = digitPatterns[digit];
  for (int i = 0; i < 7; i++) {
    digitalWrite(segmentPins[i], (pattern >> i) & 0x01); // Turn on the current segment
    delay(interval); // Small delay to allow the segment to be visible
    digitalWrite(segmentPins[i], LOW); // Turn off all segments 
  }
}


void loop() {
  int potValue = analogRead(poten);
  // // Map the potentiometer value to a suitable delay range (e.g., 1 to 100 milliseconds)
  interval = map(potValue, 0, 1023, 1, 100);



  buttonAState = digitalRead(buttonPinA);
  buttonBState = digitalRead(buttonPinB);
  resetButtonState = digitalRead(resetButtonPin);

  // Check if button A is pressed
  if (buttonAState == HIGH && lastButtonAState == LOW && countA + countB < maxCount) {

    displayCountA = displayCountA;
    countA++;
    updateShiftRegister(countA, countB);
    delay(200); // Debounce delay
  
  }
  lastButtonAState = buttonAState;

  //check if button B is pressed
  if (buttonBState == HIGH && lastButtonBState == LOW && countA + countB < maxCount) {

    displayCountA = !displayCountA;
    countB++;
    updateShiftRegister(countA, countB);
    delay(200);
    }
  lastButtonBState = buttonBState;

  // Check if the reset button is pressed
  if (resetButtonState == HIGH && lastResetButtonState == LOW) {
    countA = 0;
    countB = 0;
    clearShiftRegister();
    displayDigit(0 ,false); // Display 0 on reset
    delay(delayTime);
  }

  if (displayCountA) {
    displayDigit(countA, false);
  } else {
    displayDigit(countB, true);
  }
}
