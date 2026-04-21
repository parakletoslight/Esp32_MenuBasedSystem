#include <Arduino.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <Preferences.h>

// --- Pin Definitions ---
#define BTN_UP 25
#define BTN_DOWN 26
#define BTN_ENTER 27
#define POT_PIN 34

#define TFT_CS 5
#define TFT_DC 2
#define TFT_RST 4
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_RST);

// --- NVS (Memory) Setup ---
Preferences preferences;

// --- State Machine ---
enum ScreenState { MAIN_MENU, VIEW_DATA, TEMP_SET, HUM_SET, VOLT_SET };
ScreenState currentState = MAIN_MENU;

// --- System Variables ---
int menuCursor = 1;     // Tracks Main Menu (1-4)
int settingCursor = 0;  // Tracks Settings Menu: 0 = Low Limit, 1 = High Limit

// Dummy current values for View Data screen
int currentTemp = 24; 
int currentHum = 45;
float currentVolt = 3.3;

// Limit Variables (These will be loaded from and saved to memory)
int tempLow, tempHigh;
int humLow, humHigh;
float voltLow, voltHigh;

// --- Debounce Variables ---
unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 200; // 200ms delay to prevent double-clicks
// --- Function Prototypes ---
void drawMainMenu();
void drawViewData();
void handleSettingScreen(String title, int* lowVal, int* highVal, int mapMin, int mapMax, bool up, bool down, bool enter);
void handleFloatSettingScreen(String title, float* lowVal, float* highVal, float mapMin, float mapMax, bool up, bool down, bool enter);
void setup() {
  Serial.begin(115200);
  
  pinMode(BTN_UP, INPUT_PULLUP);
  pinMode(BTN_DOWN, INPUT_PULLUP);
  pinMode(BTN_ENTER, INPUT_PULLUP);
  
  // Initialize Display
  tft.begin();
  tft.setRotation(1); // Landscape mode
  tft.fillScreen(ILI9341_BLACK);
  
  // Initialize Non-Volatile Memory & Load Settings (Challenge 3)
  preferences.begin("sys_settings", false);
  tempLow = preferences.getInt("tempLow", 10);   // Default to 10 if not found
  tempHigh = preferences.getInt("tempHigh", 35);
  humLow = preferences.getInt("humLow", 20);
  humHigh = preferences.getInt("humHigh", 80);
  voltLow = preferences.getFloat("voltLow", 1.0);
  voltHigh = preferences.getFloat("voltHigh", 5.0);

  drawMainMenu();
}

void loop() {
  // Read raw button states
  bool readingUp = digitalRead(BTN_UP) == LOW;
  bool readingDown = digitalRead(BTN_DOWN) == LOW;
  bool readingEnter = digitalRead(BTN_ENTER) == LOW;

  // Debounce Logic (Challenge 2)
  bool upPressed = false, downPressed = false, enterPressed = false;
  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (readingUp) { upPressed = true; lastDebounceTime = millis(); }
    if (readingDown) { downPressed = true; lastDebounceTime = millis(); }
    if (readingEnter) { enterPressed = true; lastDebounceTime = millis(); }
  }

  // --- STATE MACHINE LOGIC ---
  switch (currentState) {
    
    case MAIN_MENU:
      if (upPressed && menuCursor > 1) { menuCursor--; drawMainMenu(); }
      if (downPressed && menuCursor < 4) { menuCursor++; drawMainMenu(); }
      
      if (enterPressed) {
        if (menuCursor == 1) currentState = VIEW_DATA;
        else if (menuCursor == 2) currentState = TEMP_SET;
        else if (menuCursor == 3) currentState = HUM_SET;
        else if (menuCursor == 4) currentState = VOLT_SET;
        
        settingCursor = 0; // Reset setting cursor for the next screen
        tft.fillScreen(ILI9341_BLACK);
        if (currentState == VIEW_DATA) drawViewData();
      }
      break;

    case VIEW_DATA:
      if (enterPressed) {
        currentState = MAIN_MENU;
        drawMainMenu();
      }
      break;

    case TEMP_SET:
      handleSettingScreen("Temperature (C)", &tempLow, &tempHigh, 0, 50, upPressed, downPressed, enterPressed);
      break;

    case HUM_SET:
      handleSettingScreen("Humidity (%)", &humLow, &humHigh, 0, 100, upPressed, downPressed, enterPressed);
      break;

    case VOLT_SET:
      handleFloatSettingScreen("Voltage (V)", &voltLow, &voltHigh, 0.0, 12.0, upPressed, downPressed, enterPressed);
      break;
  }
}

// --- DISPLAY FUNCTIONS ---

void drawMainMenu() {
  tft.fillScreen(ILI9341_BLACK);
  tft.setTextColor(ILI9341_WHITE); // Text color, Background color (prevents flicker)
  tft.setTextSize(2);
  
  tft.setCursor(10, 20);
  tft.println("--- SYSTEM MENU ---");
  
  tft.setCursor(10, 60);
  tft.print(menuCursor == 1 ? "-> " : "   "); tft.println("1. View All Data");
  tft.setCursor(10, 90);
  tft.print(menuCursor == 2 ? "-> " : "   "); tft.println("2. Temp Settings");
  tft.setCursor(10, 120);
  tft.print(menuCursor == 3 ? "-> " : "   "); tft.println("3. Hum Settings");
  tft.setCursor(10, 150);
  tft.print(menuCursor == 4 ? "-> " : "   "); tft.println("4. Volt Settings");
}

void drawViewData() {
  tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
  tft.setTextSize(2);
  
  tft.setCursor(10, 20); tft.println("--- CURRENT DATA ---");
  
  tft.setCursor(10, 60); tft.print("Temp:     "); tft.print(currentTemp); tft.println(" C  ");
  tft.setCursor(10, 90); tft.print("Humidity: "); tft.print(currentHum); tft.println(" %  ");
  tft.setCursor(10, 120); tft.print("Voltage:  "); tft.print(currentVolt); tft.println(" V  ");

  tft.setCursor(10, 180); tft.setTextSize(1);
  tft.println("Press ENTER to return");
}

// Handles the logic and drawing for integer settings (Temp, Hum)
void handleSettingScreen(String title, int* lowVal, int* highVal, int mapMin, int mapMax, bool up, bool down, bool enter) {
  // Read Potentiometer and map to realistic limits
  int potValue = map(analogRead(POT_PIN), 0, 4095, mapMin, mapMax);
  
  // Navigation changes which limit we are adjusting
  if (up || down) settingCursor = !settingCursor; // Toggle between 0 and 1
  
  // Assign pot value to the selected limit dynamically
  if (settingCursor == 0) *lowVal = potValue;
  else *highVal = potValue;

  // Render the screen
  tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
  tft.setTextSize(2);
  tft.setCursor(10, 20); tft.print("Set "); tft.println(title);
  
  tft.setCursor(10, 80);
  tft.print(settingCursor == 0 ? "-> " : "   "); 
  tft.print("Low Limit:  "); tft.print(*lowVal); tft.println("   "); // Extra spaces clear old artifacts

  tft.setCursor(10, 120);
  tft.print(settingCursor == 1 ? "-> " : "   "); 
  tft.print("High Limit: "); tft.print(*highVal); tft.println("   ");

  tft.setCursor(10, 180); tft.setTextSize(1);
  tft.println("Press ENTER to Save & Return");

  // Save to memory and exit
  if (enter) {
    if (title == "Temperature (C)") {
      preferences.putInt("tempLow", *lowVal);
      preferences.putInt("tempHigh", *highVal);
    } else {
      preferences.putInt("humLow", *lowVal);
      preferences.putInt("humHigh", *highVal);
    }
    currentState = MAIN_MENU;
    drawMainMenu();
  }
}

// Handles logic and drawing for float settings (Voltage)
void handleFloatSettingScreen(String title, float* lowVal, float* highVal, float mapMin, float mapMax, bool up, bool down, bool enter) {
  // Map ADC to a float
  float potValue = (analogRead(POT_PIN) / 4095.0) * (mapMax - mapMin) + mapMin;
  
  if (up || down) settingCursor = !settingCursor;
  
  if (settingCursor == 0) *lowVal = potValue;
  else *highVal = potValue;

  tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
  tft.setTextSize(2);
  tft.setCursor(10, 20); tft.print("Set "); tft.println(title);
  
  tft.setCursor(10, 80);
  tft.print(settingCursor == 0 ? "-> " : "   "); 
  tft.print("Low Limit:  "); tft.print(*lowVal, 1); tft.println(" V  "); 

  tft.setCursor(10, 120);
  tft.print(settingCursor == 1 ? "-> " : "   "); 
  tft.print("High Limit: "); tft.print(*highVal, 1); tft.println(" V  ");

  tft.setCursor(10, 180); tft.setTextSize(1);
  tft.println("Press ENTER to Save & Return");

  if (enter) {
    preferences.putFloat("voltLow", *lowVal);
    preferences.putFloat("voltHigh", *highVal);
    currentState = MAIN_MENU;
    drawMainMenu();
  }
}