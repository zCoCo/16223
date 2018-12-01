#define LED_R 9
#define LED_B 6
#define LED_G 11

//int led_vals[3] = {255,255,255}; // Defaults

void setup(){
  Serial.begin(115200);

  pinMode(LED_R, OUTPUT);
  pinMode(LED_B, OUTPUT);
  pinMode(LED_G, OUTPUT);
   delay(100);

  //
  // digitalWrite(LED_R, HIGH);
  // digitalWrite(LED_B, LOW);
  // digitalWrite(LED_G, LOW);
  // digitalWrite(LASER, HIGH);
   delay(1000);

} // #setup

void loop(){
  // for(int i=0; i<200; i++){
  //   S1.write(i);
  //   S2.write(200-i);
  //   S3.write(i);
  //   delay(10);
  // }
  // for(int i=0; i<200; i++){
  //   S1.write(200-i);
  //   S2.write(i);
  //   S3.write(200-i);
  //   delay(10);
  // }
  Serial.println("G");
  digitalWrite(LED_R, LOW);
  digitalWrite(LED_B, LOW);
  digitalWrite(LED_G, HIGH);
  delay(1000);

  Serial.println("W");
  analogWrite(LED_G, 255);
  digitalWrite(LED_R, HIGH);
  analogWrite(LED_B, 200);
  delay(1000);

  Serial.println("B");
  digitalWrite(LED_R, LOW);
  digitalWrite(LED_B, HIGH);
  digitalWrite(LED_G, LOW);
  delay(1000);

  Serial.println("R");
  digitalWrite(LED_R, HIGH);
  digitalWrite(LED_B, LOW);
  digitalWrite(LED_G, LOW);
  delay(1000);

  // for(int i=0; i<255; i++) {
  //   analogWrite(LED_R, i);
  //   analogWrite(LED_G, 255-i);
  //   analogWrite(LED_B, 0);
  //   delay(20);
  // }
  //
  // // fade from red to blue
  // for(int i=0; i<255; i++) {
  //   analogWrite(LED_R, 255-i);
  //   analogWrite(LED_G, 0);
  //   analogWrite(LED_B, i);
  //   delay(20);
  // }
  //
  // // fade from blue to green
  // for(int i=0; i<255; i++) {
  //   analogWrite(LED_R, 0);
  //   analogWrite(LED_G, i);
  //   analogWrite(LED_B, 255-i);
  //   delay(20);
  // }
}
//
// #define LED_DELAY 20
// void old_loop(){
//   int led = random(1,4); // Choose Random LED ID
//
//   Serial.println(led);
//   // Serial.print(led_vals[0]); Serial.print(", ");
//   // Serial.print(led_vals[1]); Serial.print(", ");
//   // Serial.print(led_vals[2]); Serial.println("\n");
//
//   int led_val = led_vals[led-1]; // Get Current_
//   int led_pin = (led==1) ? LED_R : ( (led==2) ? LED_B : LED_G );
//   Serial.println(led_pin);
//
//   if(led_val > 127){ // Dim
//     for(int i=led_val; i>=0; i-=5){
//       analogWrite(led_pin, i);
//       led_vals[led-1] = i;
//       delay(LED_DELAY);
//     }
//   } else{ // Brighten Up
//     for(int i=led_val; i<=255; i+=5){
//       analogWrite(led_pin, i);
//       led_vals[led-1] = i;
//       delay(LED_DELAY);
//     }
//   }
// } // #loop
