#include <Wire.h>
#include <BH1750.h>

BH1750 lightSensor;
const int buttonPin = 3;
const int ledPins[] = {4, 5, 6, 7, 8, 9, 10, 11, 12, 13};

int currentMode = 1; // 0: Chế độ 5 đèn, 1: Chế độ 10 đèn
int clickCount = 0;
unsigned long lastPressTime = 0;
bool buttonWasPressed = false;

float lightValue = 0;
unsigned long lastLightReadTime = 0;
const unsigned long lightReadInterval = 200; // Đọc cảm biến mỗi 200ms

void setup() {
  Serial.begin(9600);
  Wire.begin();
  lightSensor.begin();
  
  pinMode(buttonPin, INPUT_PULLUP); // Nút bấm nối GND
  for (int i = 0; i < 10; i++) {
    pinMode(ledPins[i], OUTPUT);
  }
}

void loop() {
  // ---------------------------------------------------------
  // 1. ĐỌC CẢM BIẾN (Non-blocking - Không dùng delay)
  // ---------------------------------------------------------
  if (millis() - lastLightReadTime >= lightReadInterval) {
    lightValue = lightSensor.readLightLevel();
    lastLightReadTime = millis();
  }

  // ---------------------------------------------------------
  // 2. LOGIC QUÉT NÚT BẤM (Siêu nhạy)
  // ---------------------------------------------------------
  bool buttonIsPressed = (digitalRead(buttonPin) == LOW); 

  if (buttonIsPressed && !buttonWasPressed) { 
    unsigned long currentTime = millis();
    // Bấm lần 2 cách lần 1 dưới 400ms -> Nhấp đúp
    if (currentTime - lastPressTime < 400) {  
      clickCount++;
    } else {
      clickCount = 1; 
    }
    lastPressTime = currentTime;
    delay(20); // Lọc nhiễu lò xo nút bấm (debounce)
  }
  buttonWasPressed = buttonIsPressed; 

  // Chốt kết quả sau 400ms thả tay
  if (clickCount > 0 && (millis() - lastPressTime) > 400) {
    if (clickCount == 1) {
      currentMode = 0; 
      Serial.println("\n>>> CHUYEN CHE DO 1: Toi da 5 den <<<");
    } else if (clickCount >= 2) {
      currentMode = 1; 
      Serial.println("\n>>> CHUYEN CHE DO 2: Toi da 10 den <<<");
    }
    clickCount = 0; 
  }

  // ---------------------------------------------------------
  // 3. TÍNH TOÁN SỐ ĐÈN SÁNG
  // ---------------------------------------------------------
  int maxAllowed = (currentMode == 0) ? 5 : 10;
  
  // Ánh sáng lux: 0 (tối) -> maxAllowed đèn | 1000 (rất sáng) -> 0 đèn
  int numLedsOn = map((int)lightValue, 0, 1000, maxAllowed, 0);
  numLedsOn = constrain(numLedsOn, 0, maxAllowed);

  // In ra Serial Monitor mỗi 500ms để không trôi màn hình
  static unsigned long lastPrintTime = 0;
  if (millis() - lastPrintTime > 500) {
      Serial.print("Anh sang (lux): "); Serial.print(lightValue);
      Serial.print(" | Che do: "); Serial.print(currentMode == 0 ? "5 den" : "10 den");
      Serial.print(" | So den sang: "); Serial.println(numLedsOn);
      lastPrintTime = millis();
  }

  // ---------------------------------------------------------
  // 4. ĐIỀU KHIỂN ĐÈN LED
  // ---------------------------------------------------------
  for (int i = 0; i < 10; i++) {
    if (i < numLedsOn) {
      digitalWrite(ledPins[i], HIGH);
    } else {
      digitalWrite(ledPins[i], LOW);
    }
  }
}
