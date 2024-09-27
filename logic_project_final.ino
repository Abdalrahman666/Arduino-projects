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

int delayTime = 1000;

// 7-segment display pins
int segmentPins[] = {A0, A1, A2, A3, A4, A5, 3}; // A0 to A5 for a to g, 3 for f
int decimalPointPin = 4; // Pin for the decimal point

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

  pinMode(segmentPins[6], OUTPUT); // Initialize pin 3 for segment g

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
    digitalWrite(segmentPins[i], (pattern >> i) & 0x01);
  }
  digitalWrite(decimalPointPin, decimalPoint ? HIGH : LOW); // Control the decimal point

}

void loop() {
  buttonAState = digitalRead(buttonPinA);
  buttonBState = digitalRead(buttonPinB);
  resetButtonState = digitalRead(resetButtonPin);

  // Check if the increment button is pressed
  if (buttonAState == HIGH && lastButtonAState == LOW && countA + countB < maxCount) {
    countA++;
    if (countA > 15) { // Reset count if it exceeds 8-bit value
      countA = 0;
    }
    updateShiftRegister(countA, countB);
    displayDigit(countA, false); // Display the last digit of the count
    delay(200); // Debounce delay
  
  }
  else if (buttonAState == HIGH && lastButtonAState == LOW && countA + countB == maxCount) {
    displayDigit(countA, false); // Display the last digit of the count
  }

  lastButtonAState = buttonAState;


  // Check if the decrement button is pressed
  if (buttonBState == HIGH && lastButtonBState == LOW && countA + countB < maxCount) {
    countB++;
    if (countB > 15) { // Reset count if it exceeds 8-bit value
      countB = 0;
    }
    updateShiftRegister(countA, countB);
    displayDigit(countB, true); // Display the last digit of the count
    delay(200);
    }
  else if (buttonBState == HIGH && lastButtonBState == LOW && countA + countB == maxCount){
    displayDigit(countB, true); // Display the last digit of the count
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

  //more fun:

  // lastResetButtonState = resetButtonState;
  //   for (int i = LEDsAllOff; i<LEDsAllOn; i++) {
  //   digitalWrite(latchPin, LOW);
  //   shiftOut(dataPin, clockPin, LSBFIRST, i);
  //   digitalWrite(latchPin, HIGH);
  //   delay(delayTime);
  // }
  // digitalWrite(latchPin, LOW);
  // shiftOut(dataPin, clockPin, LSBFIRST, LEDs2);
  // digitalWrite(latchPin, HIGH);
  // delay(400);

  // digitalWrite(latchPin, LOW);
  // shiftOut(dataPin, clockPin, LSBFIRST, LEDs3);
  // digitalWrite(latchPin, HIGH);
  // delay(400);
  //LEDsAllOff = LEDsAllOff + 1;   //another way to increment i without for loop
  //LEDs4 = LEDs4*2;   //a way to shift numbers to the right by division shifting to the left is simply multiplication
  //LEDs4 = LEDs4/128 + LEDs4*2; //for circular shifting
}
