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
