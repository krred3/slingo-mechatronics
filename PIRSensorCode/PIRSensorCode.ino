int pirPin = 2;  // Digital pin connected to the PIR sensor
int ledPin = 8; // Digital pin connected to the LED (optional)

void setup() {
  pinMode(pirPin, INPUT);
  pinMode(ledPin, OUTPUT);
  Serial.begin(9600);
}

void loop() {
  int pirState = digitalRead(pirPin);

  if (pirState == HIGH) {
    digitalWrite(ledPin, HIGH);  // Turn on the LED (optional)
    Serial.println("Motion detected!");
    delay(1000);  // Optional delay to avoid repeated detection
  } else {
    digitalWrite(ledPin, LOW);   // Turn off the LED (optional)
  }
}