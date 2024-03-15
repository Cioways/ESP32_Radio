#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <Wire.h>
#include <TEA5767Radio.h>
#include <RTClib.h>

// Definition der Pins für das TFT-Display
#define TFT_CS     15
#define TFT_RST    4
#define TFT_DC     2
#define TFT_SDI    23
#define TFT_CLK    18
#define TFT_LED    32
//Button Pins
#define BUTTON_PIN1 34
#define BUTTON_PIN2 12
//RTC Pins
#define SDA_PIN 27
#define SCL_PIN 17

RTC_DS3231 rtc;
void setRTCFromComputerTime();

//Zeit Variablen
uint8_t lastSecond = 60; // Initialisierung auf einen Wert, der sicher nicht dem aktuellen Sekundenwert entspricht
uint8_t lastMinute = 60; // Initialisierung auf einen Wert, der sicher nicht dem aktuellen Minutenwert entspricht
uint8_t lastHour = 24;   // Initialisierung auf einen Wert, der sicher nicht dem aktuellen Stundenwert entspricht
String lastTimeString;  // Initialisierung als leerer String
unsigned long previousMillis = 0;
const long interval = 1000;
void showTime(uint8_t hour, uint8_t minute, uint8_t second);


// Globale Variablen für die Positionen der Rechtecke und Buttons
int rectX, rectY, rect2X, rect2Y, button1X, button2X, button3X, button4X, buttonWidth;

// Definition der Breite und Höhe des TFT-Displays
#define TFT_WIDTH  320
#define TFT_HEIGHT 240
// Definition der Breite und Höhe der Rechtecke und der Buttons sowie der Abstände
#define RECT_WIDTH  310
#define RECT_HEIGHT 50
#define RECT2_HEIGHT 110
#define RECT_SPACING 5
#define BORDER_WIDTH 5
#define BUTTON_HEIGHT 60
#define BUTTON_SPACING 10

//TFT Display initialisieren
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_RST);


// Definition der TEA5767 Radio-Adresse
#define TEA5767_ADDR 0x60
class TEA5767Radio radio(TEA5767_ADDR);


//######################################Setup################################################################
void setup() {
  Serial.begin(9600); // Starte die serielle Kommunikation mit einer Baudrate von 9600 bps

  //RTC-------------------------------------------------------------------------------------
  Wire.begin(SDA_PIN, SCL_PIN);
   if (!rtc.begin()) {
    Serial.println("RTC konnte nicht gefunden werden!");
    while (1);
  }

  if (rtc.lostPower()) {
    Serial.println("RTC hat die Zeit verloren. Bitte neu einstellen!");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__))); // Setze die RTC-Zeit auf die Kompilierzeit des Sketches
  }

  rtc.adjust(DateTime(2024, 3, 13, 22, 37));

  

  setRTCFromComputerTime();
  //------------------------------------------------------------------------------------------

  //TFT Helligkeit Ansteuern
  pinMode(TFT_LED, OUTPUT);
  digitalWrite(TFT_LED, HIGH);

  //TFT einschalten, Rotation auf 3 Setzen und Hintergrund auf Schwarz Setzen
  tft.begin();
  tft.setRotation(3);
  tft.fillScreen(ILI9341_BLACK);

  // Berechnung der Positionen der Rechtecke und Buttons
  rectX = (TFT_WIDTH - RECT_WIDTH) / 2;
  rectY = 5;
  rect2X = (TFT_WIDTH - RECT_WIDTH) / 2;
  rect2Y = rectY + RECT_HEIGHT + RECT_SPACING;

  // Zeichnen des Oberen Rechtecks wo die Uhrzeit hinkommt
  tft.fillRect(rectX - BORDER_WIDTH, rectY - BORDER_WIDTH, RECT_WIDTH + 2 * BORDER_WIDTH, RECT_HEIGHT + 2 * BORDER_WIDTH, ILI9341_NAVY);
  tft.fillRect(rectX, rectY, RECT_WIDTH, RECT_HEIGHT, ILI9341_BLACK);

  // Zeichnen des Mittleren Rechtecks
  tft.fillRect(rect2X - BORDER_WIDTH, rect2Y - BORDER_WIDTH, RECT_WIDTH + 2 * BORDER_WIDTH, RECT2_HEIGHT + 2 * BORDER_WIDTH, ILI9341_NAVY);
  tft.fillRect(rect2X, rect2Y, RECT_WIDTH, RECT2_HEIGHT, ILI9341_GREEN);


  //---------------Buttons 1-4 Zeichnen------------------------------------------------------------
  int buttonY = rect2Y + RECT2_HEIGHT + BUTTON_SPACING;
  buttonWidth = 70;
  button1X = BORDER_WIDTH;
  button2X = button1X + buttonWidth + BUTTON_SPACING;
  button3X = button2X + buttonWidth + BUTTON_SPACING;
  button4X = button3X + buttonWidth + BUTTON_SPACING;
  tft.fillRect(button1X, buttonY, buttonWidth, BUTTON_HEIGHT, ILI9341_RED);
  tft.fillRect(button2X, buttonY, buttonWidth, BUTTON_HEIGHT, ILI9341_WHITE);
  tft.fillRect(button3X, buttonY, buttonWidth, BUTTON_HEIGHT, ILI9341_GREEN);
  tft.fillRect(button4X, buttonY, buttonWidth, BUTTON_HEIGHT, ILI9341_YELLOW);
  //-----------------------------------------------------------------------------------------------
  //---------------------------Radio Hagen Logo----------------------------------------------------
  tft.fillRect(button1X + 5, buttonY + 50, 5, 5, ILI9341_WHITE);
  tft.fillRect(button1X + 20, buttonY + 50, 5, 10, ILI9341_WHITE); 
  tft.fillRect(button1X + 35, buttonY + 50, 5, 5, ILI9341_WHITE);
  tft.fillRect(button1X + 50, buttonY + 50, 5, 10, ILI9341_WHITE);
  tft.fillRect(button1X + 63, buttonY + 50, 5, 5, ILI9341_WHITE);
  tft.setTextSize(2);
  tft.setCursor(button1X + 5, buttonY + 7);
  tft.println("radio");
  tft.setCursor(button1X + 5, buttonY + 28);
  tft.println("hagen");
  //-----------------------------------------------------------------------------------------------
  //--------------------------------WDR4 Logo------------------------------------------------------
  tft.drawLine(button2X + 52, buttonY + 45, button2X + 58, buttonY + 15, ILI9341_ORANGE);
  tft.drawLine(button2X + 53, buttonY + 45, button2X + 59, buttonY + 15, ILI9341_ORANGE); 
  tft.drawLine(button2X + 54, buttonY + 45, button2X + 60, buttonY + 15, ILI9341_ORANGE);
  tft.drawLine(button2X + 55, buttonY + 45, button2X + 61, buttonY + 15, ILI9341_ORANGE);
  tft.drawLine(button2X + 56, buttonY + 45, button2X + 62, buttonY + 15, ILI9341_ORANGE);
  tft.drawLine(button2X + 57, buttonY + 45, button2X + 63, buttonY + 15, ILI9341_ORANGE);
  tft.drawLine(button2X + 58, buttonY + 45, button2X + 64, buttonY + 15, ILI9341_ORANGE);
  tft.drawLine(button2X + 59, buttonY + 45, button2X + 65, buttonY + 15, ILI9341_ORANGE);
  tft.drawLine(button2X + 60, buttonY + 45, button2X + 66, buttonY + 15, ILI9341_ORANGE);
  tft.drawLine(button2X + 61, buttonY + 45, button2X + 67, buttonY + 15, ILI9341_ORANGE);
  tft.drawLine(button2X + 62, buttonY + 45, button2X + 68, buttonY + 15, ILI9341_ORANGE);
  tft.setCursor(button2X + 1, buttonY + 20);
  tft.setTextColor(ILI9341_NAVY);
  tft.setTextSize(3);
  tft.println("WDR");
  tft.setCursor(button2X + 56, buttonY + 20);
  tft.setTextSize(2);
  tft.setTextColor(ILI9341_WHITE);
  tft.println("4");
  //-----------------------------------------------------------------------------------------------
 
   
  //-------------------------------Text im 2. Rechteck---------------------------------------------
  //Text Größe auf 1 Setzen und Hintergrundfarbe auf Schwarz
  tft.setTextSize(1);
  tft.setTextColor(ILI9341_BLACK);

  //Erster Text
  tft.setCursor(rect2X + 20, rect2Y + 5);
  tft.println("Aktueller Sender");

  //Zweiter Text
  tft.setTextSize(3);
  tft.setCursor(rect2X + 65, rect2Y + 35);
  tft.println("107.7");

  //Dritter Text
  tft.setTextSize(2);
  tft.setCursor(rect2X + 25, rect2Y + 80);
  tft.println("RATH Bahntechnik");
  
  //Radio Frequenz Setzen
  Wire.begin();
  radio.setFrequency(107.7); 

  //Taster auf Pulldown Modus setzen. 
  pinMode(BUTTON_PIN1, INPUT_PULLDOWN);
  pinMode(BUTTON_PIN2, INPUT_PULLDOWN);
}
//##################################################################################################

//#########################################loop#####################################################
void loop() {
 
  DateTime now = rtc.now();
  showTime(now.hour(), now.minute(), now.second());
  delay(1000); // Wait for 1 second

  
  int buttonstate1 = digitalRead(BUTTON_PIN1);
  int buttonState2 = digitalRead(BUTTON_PIN2);
  
//------------------------------------Button 1-----------------------------------------------------
  if (buttonstate1 == HIGH) {
    radio.setFrequency(107.3);

    tft.fillRect(rect2X - BORDER_WIDTH, rect2Y - BORDER_WIDTH, RECT_WIDTH + 2 * BORDER_WIDTH, RECT2_HEIGHT + 2 * BORDER_WIDTH, ILI9341_NAVY);
    tft.fillRect(rect2X, rect2Y, RECT_WIDTH, RECT2_HEIGHT, ILI9341_RED);
    tft.setTextSize(1);
    tft.setTextColor(ILI9341_WHITE);

  // Text "Aktueller Sender"
    tft.setCursor(rect2X + 20, rect2Y + 5);
    tft.println("R.A.T.H Bahntechnik");

 

  // Smaller text for the radio station
    tft.setTextSize(2);
    tft.setCursor(rect2X + 25, rect2Y + 80);
    tft.println("RATH Bahntechnik");
  }
 //------------------------------------------------------------------------------------------------
 //-----------------------------Button 2-----------------------------------------------------------
  if (buttonState2 == HIGH) {
    radio.setFrequency(101.3);
    tft.fillRect(rect2X - BORDER_WIDTH, rect2Y - BORDER_WIDTH, RECT_WIDTH + 2 * BORDER_WIDTH, RECT2_HEIGHT + 2 * BORDER_WIDTH, ILI9341_NAVY);
    tft.fillRect(rect2X, rect2Y, RECT_WIDTH, RECT2_HEIGHT, ILI9341_WHITE); 
    tft.setTextSize(1);
    tft.setTextColor(ILI9341_NAVY);

    // Text "Aktueller Sender"
    tft.setTextSize(4);
    tft.setCursor(rect2X + 50, rect2Y + 5);
    tft.println("R.A.T.H Bahntechnik");

    // Bigger text for the frequency
    tft.setTextSize(3);
    tft.setCursor(rect2X + 65, rect2Y + 35);
    tft.println("WDR4");

    // Smaller text for the radio station
    tft.setTextSize(2);
    tft.setCursor(rect2X + 25, rect2Y + 80);
    tft.println("Irgendne Musik");
  }
  
}
//-------------------------------------------------------------------------------------------------
//-----------------------------------UHR Standart Wert---------------------------------------------
//############################Uhrzeit Werte woanders her kriegen, womöglich vorm Auschalten speichern.
void setRTCFromComputerTime(){
  int year = 2024;
  int month = 3;
  int day = 14;
  int hour= 4;
  int minute = 51;
  int second = 30l;
  
  rtc.adjust(DateTime(year, month, day, hour, minute, second));

}
//-------------------------------------------------------------------------------------------------
//----------------------------------UHR ANZEIGE----------------------------------------------------
void showTime(uint8_t hour, uint8_t minute, uint8_t second) {
  // Setze die Textfarbe auf weiß
  tft.setTextColor(ILI9341_GREEN);
  // Setze die Textgröße auf 6
  tft.setTextSize(6);
  
  // Lösche und aktualisiere Stunden, wenn sich die Stunden ändern oder wenn die Minuten 0 sind
  if (hour != lastHour || minute == 0) {
    tft.fillRect(rectX, rectY, 80, 50, ILI9341_BLACK);
    if (hour < 10) {
      tft.setCursor(rectX + 10, rectY + 4);
      tft.print("0");
    }
    tft.setCursor(rectX + 45, rectY + 4);
    tft.print(hour);
    lastHour = hour; // Aktualisiere die letzte Stundenzahl
  }
  
  tft.setCursor(rectX + 80, rectY + 4);
  tft.print(":");
  
  // Lösche und aktualisiere Minuten, wenn sich die Minuten ändern
  if (minute != lastMinute) {
    tft.fillRect(rectX + 115, rectY, 80, 50, ILI9341_BLACK);
    if (minute < 10) {
      tft.setCursor(rectX + 150, rectY);
      tft.print("0");
    }
    tft.setCursor(rectX + 125, rectY + 4);
    tft.print(minute);
    lastMinute = minute; // Aktualisiere die letzte Minutenzahl
  }
    tft.setCursor(rectX + 195, rectY + 4);
    tft.print(":");
    
  // Lösche und aktualisiere Sekunden, wenn sich die Sekunden ändern
  if (second != lastSecond) {
    tft.fillRect(rectX + 225, rectY, 85, 50, ILI9341_BLACK);
  }
    tft.setCursor(rectX + 230, rectY + 4);
    tft.print(second);
    lastSecond = second; // Aktualisiere die letzte Sekundenzahl
}
//-------------------------------------------------------------------------------------------------
