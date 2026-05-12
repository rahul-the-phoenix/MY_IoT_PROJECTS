//=======================DAY 1 of Learning IoT (LED, Serial monitor) ==============================

//int led = 2;
void setup(){           //   run once
pinMode(2,OUTPUT);
Serial.begin(115200);    // for serial monitor output 

} 
void loop (){            //  run i a loop 
digitalWrite(2,1);       //  1==high (led,HIIGH) define int led if you not directly using the pin 
Serial.println("LEd on");   // serial monitor output 
delay(1000);
digitalWrite(2,0);
Serial.println("LEd off");
delay(2000);
}




//=======================DAY 2 of Learning IoT(Serial Monitor In details)==============================


const int LED_PIN = 2;  // Using pin 2 (not built-in, use external LED)
bool ledstage = false;

void setup() {
  pinMode(LED_PIN, OUTPUT);
  Serial.begin(115200);         // serial monitor open at rate 115200
  Serial.println("Serial Monitor Ready!");
  Serial.println("Type 'on' to turn LED ON");
  Serial.println("Type 'off' to turn LED OFF");
  Serial.println("Type 'toggle' to flip LED state");
  Serial.println("Type '5sec' to turn ON for 5 seconds");
  Serial.println("Type 'status' to check LED state");
}

void loop() {
  if (Serial.available() > 0) {     // Checking if data is available
    String userInput = Serial.readString();    // Reading the input
    userInput.trim();    // Remove whitespace and newline characters
    userInput.toLowerCase();     // Convert to lowercase
    
    if (userInput == "on") {
      digitalWrite(LED_PIN, HIGH);  //digitalWrite(2, 1/0) as 1 & 0 for high & low, 2= ledpin    
      ledstage = true;
      Serial.println("✅ LED is now ON");
    }
    else if (userInput == "off") {
      digitalWrite(LED_PIN, LOW);
      ledstage = false;
      Serial.println("❌ LED is now OFF");
    }
    else if (userInput == "toggle") {
      ledstage = !ledstage;                        // TOGGLING MOST IMPORTANT 
      digitalWrite(LED_PIN, ledstage);
      Serial.print("🔄 LED toggled. Now ");
      Serial.println(ledstage ? "ON" : "OFF");         // Ternary Operator if 1 then on else off 
    }
    else if (userInput == "5sec") {
      ledstage = true;
      digitalWrite(LED_PIN, HIGH);
      Serial.println("💡 LED will stay ON for 5 seconds...");
      delay(5000);
      digitalWrite(LED_PIN, LOW);
      ledstage = false;
      Serial.println("⏰ 5 seconds completed. LED is now OFF");
    }
    else if (userInput == "status") {
      Serial.print("📊 LED is currently ");
      Serial.println(ledstage ? "ON" : "OFF");
    }
    else {
      Serial.print("❓ Unknown command: '");
      Serial.print(userInput);                   
      Serial.println("' - Please type: on, off, toggle, 5sec, or status");
    }
  }
}

//=======================DAY 3 of Learning IoT(FOR LOOPS , Led brightness , Function In details)==============================


int led = 2;
void ledcontrol() {             // creating a function 

  for(int i = 0; i < 3; i++) {       //for loop using for effects and all
    digitalWrite(led, HIGH);
    delay(200);
    digitalWrite(led, LOW);
    delay(200);
  }
  delay(1000);

  for(int brightness = 0; brightness <= 255; brightness++) {      //  for controlling the brightness use brightness 
    analogWrite(led, brightness);                       //   this effect is known as breath effect
    delay(5);
  }      
}

void setup() {
  pinMode(led, OUTPUT);
}

void loop() {
  ledcontrol();     // Call the function functon will run in a loop
}




//=======================DAY 4 of Learning IoT(Switch uses , floating points , input pullups)==============================

int buttonPin = 12;  
int ledPin = 2;    
int buttonState = 0;  

void setup() {
 // pinMode(buttonPin, INPUT);   // Set button pin as input   ### Floating point error so that led is may on or off (10K resistor is not connected)
  pinMode(buttonPin, INPUT_PULLUP);    // solving the problem  by using input_pullup default high , if pressed then low 
  pinMode(ledPin, OUTPUT);        
  digitalWrite(ledPin, LOW);          // Start with LED off
}

void loop() {
  buttonState = digitalRead(buttonPin);  // Read button state 
  
  if (buttonState == LOW) {     // If button is pressed (LOW) means connected to gnd 
    digitalWrite(ledPin, HIGH); // Turn LED ON
  } else {                      
    digitalWrite(ledPin, LOW);  // not connected to gnd so  Turn LED OFF
  }

}

//===================================DAY %5 of Learning IoT(toggle switch)============================

int buttonPin = 12;
int ledPin = 2;
int lastButtonState = HIGH;

void setup() {
  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(ledPin, OUTPUT);
}

void loop() {
  int buttonState = digitalRead(buttonPin);
  
  if (lastButtonState == HIGH && buttonState == LOW) {             //button state is low when pressed (connected to gnd)
    digitalWrite(ledPin, !digitalRead(ledPin));        //   ledPin condition toggles 
    delay(200);
  }
  
  lastButtonState = buttonState;
}
