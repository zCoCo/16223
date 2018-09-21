#include <Servo.h>

const float rad2deg = 180.0f / M_PI;

#define P_PIVOT 3
#define P_SERVA 6
#define P_SERVB 9

#define CONFIRM_PIN 2

Servo Sp, Sa, Sb;

#define P_ORIGIN 0
#define P_DIR 1
#define A_ORIGIN 140
#define A_DIR 1
#define B_ORIGIN 135
#define B_DIR -1

#define LEN_A 53.059 // [mm] First Arm Linkage Length
#define LEN_B 53.059 // [mm] Second Arm Linkage Length

#define N_GRID_W 3 // Width of Grid in Number of Buttons
#define N_GRID_H 4 // Height of Grid in Number of Buttons
#define GRID_W 50 // [mm] Width of Grid of Button Nodes
#define GRID_H 100 // [mm] Height of Grid of Button Nodes
#define OFFSET_X 0 // [mm] X Distance from Center of Joint A to Center Axis of Button Grid
#define OFFSET_Y 51.75 // [mm] Y Distance from Center of Joint A to Closest Row of Buttons

#define DIGIT_ENTRY_TIME 100 // [ms] Maximum Number of Milliseconds it Takes to Enter a Digit

float currThP = 0; float currThA = 0; float currThB = 0;

// Go Move Joint P to the Given Position, in degrees.
void goTo_ThP(int p){
  currThP = p;
  Sp.write(constrain(P_ORIGIN + P_DIR * p, 0, 180));
} // #goTo_ThP
// Go Move Joint A to the Given Position, in degrees.
void goTo_ThA(int a){
  currThA = a;
  Sa.write(constrain(A_ORIGIN + A_DIR * a, 0, 180));
} // #goTo_ThA
// Go Move Joint B to the Given Position, in degrees.
void goTo_ThB(int b){
  currThB = b;
  Sb.write(constrain(B_ORIGIN + B_DIR * b, 0, 180));
} // #goTo_ThB
// Go To Configuration Position (p,a,b), in degrees.
void goTo_cfg(int p, int a, int b){
  goTo_ThP(p);
  goTo_ThA(a);
  goTo_ThB(b);
} // #goTo_cfg

// Go To Position (x,y), in mm, Relative to the Center of Joint A in Plane Inclined by Joint P.
void goTo_XY(float x, float y){
  static float c2,s2,k1,k2;
  static int thA, thB;

  c2 = (x*x + y*y - LEN_A*LEN_A - LEN_B*LEN_B) / (2.0f * LEN_A * LEN_B);

  if(c2 < 0.95){ // Point Accessible and Away from Singularity
    s2 = sqrtf(1.0f - c2*c2);
    k1 = LEN_A + LEN_B * c2;
    k2 = LEN_B * s2;

    thA = rad2deg * atan2f( -1.0f*(x*k1 + y*k2), (y*k1 - x*k2) );
    thB = rad2deg * atan2f(s2, c2);

    goTo_ThA(thA); goTo_ThB(thB);
  } // c2<0.95?
} // #goTo_XY

// Move to Button in Row r and Column c (indexed from 0).
void goToButton(int r, int c){
  float Bx = OFFSET_X - GRID_W/2 + c * GRID_W / (N_GRID_W - 1);
  float By = OFFSET_Y + GRID_H - r * GRID_H / (N_GRID_H - 1);
  goTo_ThP(0);
  goTo_XY(Bx, By);
  goTo_ThP(90);
} // #goToButton

// Indices of Keypad Digits (staring from 0) (linearlly indexed in rows then columns):
int digitIndices[] = {10, 0, 1, 2, 3, 4, 5, 6, 7, 8};

// Move Arm to Keypad Digit
void goToDigit(int n){
  int idx = digitIndices[n];
  int r = idx / N_GRID_W;
  int c = idx % N_GRID_W;
  goToButton(r,c);
} // #goToDigit

#define COMBO_LENGTH 4
// Enter the Given Multi-digit Number on the Keypad
void enterNumber(int n){
  for(int i=COMBO_LENGTH-1; i>0; i--){
    int dig = n / Pow(10,i); // if number is less than combo length, this returns 0 (desired behavior)
    n - dig * Pow(10,i);
    goToDigit(dig);
    delay(DIGIT_ENTRY_TIME);
  }
} // #enterNumber

void setup(){
  Serial.begin(9600);

  Sp.attach(P_PIVOT);
  Sa.attach(P_SERVA);
  Sb.attach(P_SERVB);

  pinMode(CONFIRM_PIN, INPUT);

  goTo_cfg(0,0,0);
} // #setup

void loop(){
  static int count = 0;
  static long last_tap = 0;

  goToDigit(count % 10);

  // Wait for Tap to Confirm Receipt of Pin:
  while(digitalRead(CONFIRM_PIN)){ /*wait*/ };
  last_tap = millis();

  // Check for Double Tap to Indicate Success
  delay(50);
  while(last_press-millis() < 200){
    if(digitalRead(CONFIRM_PIN)){ // Successful guess
      // Do a victory dance:
      goTo_cfg(30, 45, 90);
      delay(250);
      goTo_cfg(40, 20, 20);
      delay(250);
      goTo_cfg(30, 45, 90);
      delay(250);
      goTo_cfg(40, 20, 20);
    }
  }

  count++;
} // #loop
