#include <Adafruit_GFX.h>
#include <Adafruit_NeoMatrix.h>
#include <Adafruit_NeoPixel.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

#ifndef PSTR
#define PSTR
#endif

#define PIN 12

Adafruit_NeoMatrix matrix = Adafruit_NeoMatrix(32, 8, PIN,
                                               NEO_MATRIX_BOTTOM + NEO_MATRIX_RIGHT + NEO_MATRIX_COLUMNS + NEO_MATRIX_ZIGZAG);

String msg = "";

int x_pos = matrix.width();
int y_size = matrix.height();
int msg_length = msg.length();
int x_size = matrix.width();
int pass = 0;
String received = "";
bool hasRun = false;
int duration = 70;
uint16_t textColor = matrix.Color(0, 0, 255);  // Default color: blue
bool multiColor = false;

BLEServer* pServer = NULL;
BLECharacteristic* pCharacteristicTX = NULL;
BLECharacteristic* pCharacteristicRX = NULL;
bool deviceConnected = false;
bool oldDeviceConnected = false;
bool displayOn = true;
uint32_t value = 0;

#define SERVICE_UUID "8cdd366e-7eb4-442d-973f-61e2fd4b56f0"
#define CHARACTERISTIC_UUID_RX "dc994613-74f5-4c4f-b671-5a8d297f737a"
#define CHARACTERISTIC_UUID_TX "cc5e8e3a-f8e6-4889-8a18-9069272be2a5"

class MyServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) {
    deviceConnected = true;
  };

  void onDisconnect(BLEServer* pServer) {
    deviceConnected = false;
  }
};


void sendDisplayStatus() {
  // Extract RGB values from textColor
  uint8_t red = (textColor >> 11) & 0x1F;
  uint8_t green = (textColor >> 5) & 0x3F;
  uint8_t blue = textColor & 0x1F;

  // Scale the values to 8-bit (0-255) range
  red = map(red, 0, 31, 0, 255);
  green = map(green, 0, 63, 0, 255);
  blue = map(blue, 0, 31, 0, 255);
  String status = "Message: " + msg + "\n" + "Color: " + String(red) + "," + String(green) + "," + String(blue) + "\n" + "Speed: " + String(duration) + "\n" + "Brightness: " + String(matrix.getBrightness()) + "\n" + "Display On: " + String(displayOn ? "Yes" : "No") + "\n" + "Multicolor: " + String(multiColor ? "On" : "Off") + "\n";

  // Debug print to verify the status string
  Serial.println("Sending status:");
  Serial.println(status);

  // Convert the status string to a C-style string
  const char* statusCharArray = status.c_str();

  // Set the value and notify
  pCharacteristicTX->setValue((uint8_t*)statusCharArray, strlen(statusCharArray));
  pCharacteristicTX->notify();
}
class MyCallbacks : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic* pCharacteristic) {
    std::string rxValue = pCharacteristic->getValue();
    if (rxValue.length() > 0) {
      Serial.println("===start=receive===");
      Serial.println("Received value: " + String(rxValue.c_str()));
      received = String(rxValue.c_str());
    }
    // Check if the received value is a speed command
    if (received.startsWith("speed:")) {
      int newDuration = received.substring(6).toInt();
      if (newDuration > 0 && newDuration <= 1000) {
        duration = newDuration;
        Serial.println("Set duration to: " + String(duration));
      }
    }
    // Check if the received value is a brightness command
    else if (received.startsWith("brightness:")) {
      int brightness = received.substring(11).toInt();
      if (brightness >= 0 && brightness <= 255) {
        matrix.setBrightness(brightness);
        Serial.println("Set brightness to: " + String(brightness));
      }
    }
    // Check if the received value is a color command
    else if (received.startsWith("color:")) {
      int r = received.substring(6, 9).toInt();
      int g = received.substring(10, 13).toInt();
      int b = received.substring(14, 17).toInt();
      textColor = matrix.Color(r, g, b);
      Serial.println("Set text color to: " + String(r) + ", " + String(g) + ", " + String(b));
    }
    // Check if the received value is a multicolor command
    else if (received == "multicolor:on") {
      multiColor = true;
      Serial.println("Multicolor text enabled");
    } else if (received == "multicolor:off") {
      multiColor = false;
      Serial.println("Multicolor text disabled");
    }
    // Check if the received value is "off" or "on"
    else if (received == "off") {
      displayOn = false;
      matrix.fillScreen(0);  // Turn off all LEDs
      matrix.show();
      Serial.println("Display turned off");
    } else if (received == "on") {
      displayOn = true;
      Serial.println("Display turned on");
    } else if (received == "reset") {
      ESP.restart();
    } else {
      msg = String(rxValue.c_str());
      x_pos = matrix.width();
    }
    // Send notification with current display status
    sendDisplayStatus();
  }
};

// Color definitions
#define BLACK 0x0000
#define BLUE 0x001F
#define RED 0xF800
#define GREEN 0x07E0
#define CYAN 0x07FF
#define MAGENTA 0xF81F
#define YELLOW 0xFFE0
#define WHITE 0xFFFF

// arrays holding color config data for scrolling text and for graphics drawings
const uint16_t textColors[] = { matrix.Color(255, 0, 0), matrix.Color(0, 255, 0), matrix.Color(0, 0, 255), matrix.Color(255, 255, 0), matrix.Color(137, 112, 219) };
const uint16_t drawingColors[] = { GREEN, BLUE, CYAN, WHITE, RED, YELLOW, MAGENTA };

void setup() {
  Serial.begin(115200);

  // Create the BLE Device
  BLEDevice::init("ESP32");
  BLEDevice::setMTU(517);  // Set MTU size to 517 bytes

  // Create the BLE Server
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Create the BLE Service
  BLEService* pService = pServer->createService(SERVICE_UUID);

  // Create a BLE Characteristic
  pCharacteristicTX = pService->createCharacteristic(
    CHARACTERISTIC_UUID_TX,
    BLECharacteristic::PROPERTY_NOTIFY);
  BLE2902* pBLE2902;
  pBLE2902 = new BLE2902();
  pBLE2902->setNotifications(true);
  pCharacteristicTX->addDescriptor(new BLE2902());

  // Create a BLE Characteristic for RX
  pCharacteristicRX = pService->createCharacteristic(
    CHARACTERISTIC_UUID_RX,
    BLECharacteristic::PROPERTY_WRITE);
  pCharacteristicRX->setCallbacks(new MyCallbacks());

  pService->start();
  // Start advertising
  BLEAdvertising* pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(false);
  pAdvertising->setMinPreferred(0x00);  // set value to 0x00 to not advertise this parameter
  BLEDevice::startAdvertising();
  Serial.println("Waiting a client connection to notify...");
  // Request MTU size
  pServer->getAdvertising()->setMinPreferred(0x06);  

  matrix.begin();
  matrix.setTextWrap(false);
  matrix.setBrightness(40);
  matrix.setTextColor(textColors[0]);
  Serial.println("NeoMatrix pixel grid test");
  Serial.println((String) "Matrix Pixel Width x Height: " + x_size + " x " + y_size);

  // Display startup message
  msg = "Welcome";
  drawBars();
}


void loop() {

  // notify changed value
  if (deviceConnected) {
    sendDisplayStatus();
    delay(3);  // bluetooth stack will go into congestion, if too many packets are sent.
  }

  // disconnecting
  if (!deviceConnected && oldDeviceConnected) {
    delay(500);                   // give the bluetooth stack the chance to get things ready
    pServer->startAdvertising();  // restart advertising
    Serial.println("start advertising");
    oldDeviceConnected = deviceConnected;
  }
  // connecting
  if (deviceConnected && !oldDeviceConnected) {
    // do stuff here on connecting
    oldDeviceConnected = deviceConnected;
  }

  if (displayOn) {
    displayMessage(msg, duration);
  }
}

void displayMessage(String message, int duration) {
  matrix.fillScreen(0);
  matrix.setCursor(x_pos, 0);
  msg = message;
  msg_length = msg.length();
  // Set text color based on message content
  if (msg == "Unavailable" || msg == "unavailable") {
    matrix.setTextColor(RED);
  } else if (msg == "Available" || msg == "available") {
    matrix.setTextColor(GREEN);
  } else {
    matrix.setTextColor(textColor);
  }
  matrix.print(msg);

  // if(--x_pos < -(msg_length * 6)) {
  //   x_pos = matrix.width();

  if (msg_length * 6 > x_size) {  // Only scroll if message is wider than the screen
    for (int i = 0; i < msg_length; i++) {
      matrix.setCursor(x_pos + i * 6, 0);
      if (multiColor) {
        matrix.setTextColor(textColors[i % 5]);
      }
      matrix.print(msg[i]);
    }
    if (--x_pos < -(msg_length * 6)) {
      x_pos = matrix.width();
    }
  } else {
    x_pos = (x_size - msg_length * 6) / 2;  // Center the text if it's shorter than the screen width
    for (int i = 0; i < msg_length; i++) {
      matrix.setCursor(x_pos + i * 6, 0);
      if (multiColor) {
        matrix.setTextColor(textColors[i % 3]);
      }
      matrix.print(msg[i]);
    }
  }
  matrix.show();
  delay(duration);
}


void testPixels() {
  int rnd;
  matrix.fillScreen(0);
  for (int i = 0; i < x_size; i++) {
    for (int j = 0; j < y_size; j++) {
      rnd = random(0, 7);
      matrix.drawPixel(i, j, drawingColors[rnd]);
      matrix.show();
      delay(75);
    }
  }
}

void drawBars() {
  int rnd;
  for (int i = 0; i < 10; i++) {
    matrix.fillScreen(0);
    for (int x = 0; x < 32; x++) {  // Loop through all 32 columns
      rnd = random(1, 7);
      matrix.drawLine(x, 7, x, rnd, GREEN);
      matrix.drawPixel(x, rnd - 1, RED);
    }
    matrix.show();
    delay(300);
  }
}