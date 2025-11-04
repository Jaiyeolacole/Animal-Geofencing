#include <SoftwareSerial.h>

// GSM Configuration
SoftwareSerial sim800(2, 3); // RX, TX for SIM800L

// Phone number for alerts
const String ALERT_PHONE_NUMBER = "+2347063135499"; // Replace with actual number

void setup() {
  Serial.begin(9600);
  sim800.begin(9600);  // SIM800L baud rate
  
  // Initialize GSM module
  delay(1000); // Allow GSM to stabilize
  initializeGSM();
}

void initializeGSM() {
  // Test GSM communication
  sim800.println("AT"); 
  delay(1000);
  readGSMResponse();
  
  // Check signal quality
  sim800.println("AT+CSQ"); 
  delay(1000);
  readGSMResponse();
  
  // Check SIM card status
  sim800.println("AT+CPIN?");
  delay(1000);
  readGSMResponse();
  
  Serial.println("GSM Module Initialized");
}

// Send SMS function
void sendSMS(const String& message) {
  Serial.println("Sending SMS...");
  
  // Set SMS to text mode
  sim800.println("AT+CMGF=1"); 
  delay(1000);
  
  // Set recipient number
  sim800.print("AT+CMGS=\"");
  sim800.print(ALERT_PHONE_NUMBER);
  sim800.println("\"");
  delay(1000);
  
  // Send message
  sim800.print(message);
  delay(500);
  
  // Send Ctrl+Z to end message
  sim800.write(26); 
  delay(5000);
  
  Serial.println("SMS sent successfully!");
}

// Send HELLO message on startup
void sendHelloMessage() {
  String message = "HELLO! System initialized and ready.";
  sendSMS(message);
}

// Read response from GSM module
void readGSMResponse() {
  Serial.print("GSM Response: ");
  while (sim800.available()) {
    Serial.write(sim800.read());
  }
  Serial.println();
}

void loop() {
  // Example usage - send test SMS every 60 seconds
  static unsigned long lastSend = 0;
  if (millis() - lastSend > 60000) {
    sendSMS("System heartbeat - All systems operational");
    lastSend = millis();
  }
  
  // Check for incoming messages or other GSM operations
  if (sim800.available()) {
    String response = sim800.readString();
    Serial.print("Received: ");
    Serial.println(response);
  }
  
  delay(1000);
}