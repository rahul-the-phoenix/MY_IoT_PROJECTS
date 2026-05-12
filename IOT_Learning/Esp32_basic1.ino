//=======================DAY 1 of Learning IoT==============================

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




//=======================DAY 2 of Learning IoT==============================


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
      digitalWrite(LED_PIN, HIGH);             //digitalWrite(2, 1/0);  as 1 and 0 for high and low , and the led pin if known      
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
