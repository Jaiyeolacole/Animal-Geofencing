#include <TinyGPS++.h>
#include <SoftwareSerial.h>

// GPS Module Connections
static const int RXPin = 2, TXPin = 3;
static const uint32_t GPSBaud = 9600;

// SIM800L Module Connections
static const int SIM800L_RX = 4, SIM800L_TX = 5;

// The TinyGPS++ object
TinyGPSPlus gps;

// Serial connections to GPS and SIM800L
SoftwareSerial gpsSerial(RXPin, TXPin);
SoftwareSerial sim800l(SIM800L_RX, SIM800L_TX);

const double PERIMETER_LAT_MIN = 40.7128;  // Minimum latitude
const double PERIMETER_LAT_MAX = 40.7138;  // Maximum latitude  
const double PERIMETER_LON_MIN = -74.0060; // Minimum longitude
const double PERIMETER_LON_MAX = -74.0050; // Maximum longitude

// Phone number
const String PHONE_NUMBER = "+1234567890";

// Alert tracking variables
bool isInsidePerimeter = true;
bool alertSent = false;
unsigned long lastSMSTime = 0;
const unsigned long SMS_COOLDOWN = 300000; // 5 minutes between SMS alerts

void setup() {
  // Initialize serial for debugging
  Serial.begin(9600);
  
  // Initialize GPS serial
  gpsSerial.begin(GPSBaud);
  
  // Initialize SIM800L serial
  sim800l.begin(9600);
  
  Serial.println("Animal Perimeter Monitoring System Started");
  Serial.println("Initializing SIM800L...");
  
  // Initialize SIM800L module
  initializeSIM800L();
  
  // Send startup SMS
  sendSMS("System started. Monitoring animal perimeter.");
  
  Serial.println("System Ready - Monitoring Perimeter...");
}

void loop() {
  // Read GPS data
  while (gpsSerial.available() > 0) {
    if (gps.encode(gpsSerial.read())) {
      processGPSData();
    }
  }

  // If no GPS data is received for a while, show warning
  if (millis() > 5000 && gps.charsProcessed() < 10) {
    Serial.println("No GPS data received: Check wiring");
  }

  delay(1000); 
}

void processGPSData() {
  if (gps.location.isValid() && gps.location.isUpdated()) {
    double currentLat = gps.location.lat();
    double currentLon = gps.location.lng();
    
    // Print current position for debugging
    Serial.print("Position: ");
    Serial.print(currentLat, 6);
    Serial.print(", ");
    Serial.print(currentLon, 6);
    Serial.print(" | Satellites: ");
    Serial.println(gps.satellites.value());
    
    // Check if position is within perimeter
    bool currentlyInside = isWithinPerimeter(currentLat, currentLon);
    
    // Handle perimeter breach detection
    if (isInsidePerimeter && !currentlyInside) {
      // Animal has left the perimeter
      Serial.println("ALERT: Animal has left the perimeter!");
      sendPerimeterAlert(currentLat, currentLon, "LEFT");
      isInsidePerimeter = false;
      alertSent = true;
      lastSMSTime = millis();
    }
    else if (!isInsidePerimeter && currentlyInside) {
      // Animal has returned to perimeter
      Serial.println("Animal has returned to perimeter");
      sendPerimeterAlert(currentLat, currentLon, "RETURNED");
      isInsidePerimeter = true;
      alertSent = false;
    }
    else if (!currentlyInside && !alertSent && 
             (millis() - lastSMSTime > SMS_COOLDOWN)) {
      // Animal is still outside, send reminder
      Serial.println("REMINDER: Animal is still outside perimeter!");
      sendPerimeterAlert(currentLat, currentLon, "STILL_OUTSIDE");
      lastSMSTime = millis();
    }
  }
  else {
    Serial.println("Waiting for GPS fix...");
    Serial.print("Satellites: ");
    Serial.println(gps.satellites.value());
  }
}

bool isWithinPerimeter(double lat, double lon) {
  return (lat >= PERIMETER_LAT_MIN && lat <= PERIMETER_LAT_MAX &&
          lon >= PERIMETER_LON_MIN && lon <= PERIMETER_LON_MAX);
}

void sendPerimeterAlert(double lat, double lon, String status) {
  String message = "ANIMAL ALERT: Animal has " + status + " perimeter!";
  message += "\nPosition: " + String(lat, 6) + ", " + String(lon, 6);
  message += "\nTime: " + String(gps.time.hour()) + ":" + 
             String(gps.time.minute()) + ":" + String(gps.time.second());
  
  sendSMS(message);
}

void initializeSIM800L() {
  delay(1000);
  
  // Check if SIM800L is responding
  sim800l.println("AT");
  delay(1000);
  while(sim800l.available()){
    Serial.write(sim800l.read());
  }
  
  // Configure SMS text mode
  sim800l.println("AT+CMGF=1");
  delay(1000);
  while(sim800l.available()){
    Serial.write(sim800l.read());
  }
  
  Serial.println("SIM800L Initialized");
}

void sendSMS(String message) {
  Serial.println("Sending SMS...");
  Serial.println(message);
  
  // Set SMS mode to text
  sim800l.println("AT+CMGF=1");
  delay(1000);
  
  // Set the phone number
  sim800l.print("AT+CMGS=\"");
  sim800l.print(PHONE_NUMBER);
  sim800l.println("\"");
  delay(1000);
  
  // Send the message
  sim800l.print(message);
  delay(500);
  
  // End the message with Ctrl+Z
  sim800l.write(26);
  delay(5000);
  
  Serial.println("SMS Sent Successfully");
}

// Utility function to get GPS data as string
String getGPSData() {
  String data = "";
  if (gps.location.isValid()) {
    data += "Lat: " + String(gps.location.lat(), 6);
    data += " Lon: " + String(gps.location.lng(), 6);
  }
  if (gps.date.isValid()) {
    data += " Date: " + String(gps.date.month()) + "/" + 
            String(gps.date.day()) + "/" + String(gps.date.year());
  }
  if (gps.time.isValid()) {
    data += " Time: " + String(gps.time.hour()) + ":" + 
            String(gps.time.minute()) + ":" + String(gps.time.second());
  }
  return data;
}
