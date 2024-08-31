
// const int EnableL = 9;
const int EnableL = 11;
const int HighL = 10;  // LEFT SIDE MOTOR
const int LowL = 12;

const int EnableR = 6;
const int HighR = 7;  //RIGHT SIDE MOTOR
const int LowR = 8;

const int D0 = 2;  //Raspberry GPIO 26
const int D1 = 3;  //Raspberry GPIO 19
const int D2 = 4;  //Raspberry GPIO 6
const int D3 = 5;  //Raspberry GPIO 5

const int leftIndicator=14;
const int rightIndicator=15;

int i = 0;
unsigned long int j = 0;
int a, b, c, d, data;
int Lstate=0; 
int Rstate=0;
volatile int L_R=0; //0 idle 1 left 2 right

void delaySetUp()
{
  // Pause interrupts
  cli();

  // Reset counter1
  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1  = 0;
 
  // Set the value that the counter has to reach before it triggers an interrupt
  // 15624 = 1s (assuming you use 1024 as the prescaler value)
  // Counter1 is 16-bit so the value must be less than or equal to 65535
  OCR1A = 15624/2;
 
  // Clear timer on compare match
  // The timer resets itself when it reaches 15625 (OCR1A +1)
  TCCR1B |= (1 << WGM12);
 
  // Set the prescaler to 1024 (See ATMega328PU datasheet for infos)
  TCCR1B |= (1 << CS12) | (1 << CS10);
 
  // Enable timer interrupt
  TIMSK1 |= (1 << OCIE1A);

  // Enable interrupts
  sei();
}

ISR(TIMER1_COMPA_vect)
{ 
  int pin=NULL;
  switch(L_R){
    case 0:
      digitalWrite(leftIndicator,LOW);
      digitalWrite(rightIndicator,LOW);
      break;
    case 1:
      pin=leftIndicator;
      digitalWrite(pin,(Lstate^=1) ? HIGH : LOW);
      break;
    case 2:
      pin=rightIndicator;
      digitalWrite(pin,(Rstate^=1) ? HIGH : LOW);
      break;
    default:
      digitalWrite(leftIndicator,LOW);
      digitalWrite(rightIndicator,LOW);
      break;
  }
}

void setup() {

  pinMode(EnableL, OUTPUT);
  pinMode(HighL, OUTPUT);
  pinMode(LowL, OUTPUT);

  pinMode(EnableR, OUTPUT);
  pinMode(HighR, OUTPUT);
  pinMode(LowR, OUTPUT);

  pinMode(leftIndicator, OUTPUT);
  pinMode(rightIndicator, OUTPUT);
  digitalWrite(leftIndicator,LOW);
  digitalWrite(rightIndicator,LOW);

  pinMode(D0, INPUT_PULLUP);
  pinMode(D1, INPUT_PULLUP);
  pinMode(D2, INPUT_PULLUP);
  pinMode(D3, INPUT_PULLUP);
  Serial.begin(9600);
  while(!Serial);
  delaySetUp();
  delay(25000); // for setup
}


void Data() {
  a = digitalRead(D0);
  b = digitalRead(D1);
  c = digitalRead(D2);
  d = digitalRead(D3);

  data = 8 * d + 4 * c + 2 * b + a;
}

void Forward() {
  digitalWrite(HighL, LOW);
  digitalWrite(LowL, HIGH);
  analogWrite(EnableL, 255);

  digitalWrite(HighR, LOW);
  digitalWrite(LowR, HIGH);
  analogWrite(EnableR, 255);
}


void Backward() {
  digitalWrite(HighL, HIGH);
  digitalWrite(LowL, LOW);
  analogWrite(EnableL, 255);

  digitalWrite(HighR, HIGH);
  digitalWrite(LowR, LOW);
  analogWrite(EnableR, 255);
}

void Stop() {
  digitalWrite(HighL, LOW);
  digitalWrite(LowL, HIGH);
  analogWrite(EnableL, 0);

  digitalWrite(HighR, LOW);
  digitalWrite(LowR, HIGH);
  analogWrite(EnableR, 0);
  delay(1000);
}

void Left1() {
  digitalWrite(HighL, LOW);
  digitalWrite(LowL, HIGH);
  analogWrite(EnableL, 200);

  digitalWrite(HighR, LOW);
  digitalWrite(LowR, HIGH);
  analogWrite(EnableR, 255);
}

void Left2() {
  digitalWrite(HighL, LOW);
  digitalWrite(LowL, HIGH);
  analogWrite(EnableL, 150);

  digitalWrite(HighR, LOW);
  digitalWrite(LowR, HIGH);
  analogWrite(EnableR, 255);
}


void Left3() {
  digitalWrite(HighL, LOW);
  digitalWrite(LowL, HIGH);
  analogWrite(EnableL, 110);

  digitalWrite(HighR, LOW);
  digitalWrite(LowR, HIGH);
  analogWrite(EnableR, 255);
}

void Right1() {
  digitalWrite(HighL, LOW);
  digitalWrite(LowL, HIGH);
  analogWrite(EnableL, 255);

  digitalWrite(HighR, LOW);
  digitalWrite(LowR, HIGH);
  analogWrite(EnableR, 200);
}
void Right2() {
  digitalWrite(HighL, LOW);
  digitalWrite(LowL, HIGH);
  analogWrite(EnableL, 255);

  digitalWrite(HighR, LOW);
  digitalWrite(LowR, HIGH);
  analogWrite(EnableR, 150);
}

void Right3() {
  digitalWrite(HighL, LOW);
  digitalWrite(LowL, HIGH);
  analogWrite(EnableL, 255);

  digitalWrite(HighR, LOW);
  digitalWrite(LowR, HIGH);
  analogWrite(EnableR, 110);
}

void UTurn(){  
  Stop();
  L_R=1;
  tankForward();
  delay(500);
  digitalWrite(HighL, LOW);
  digitalWrite(LowL, HIGH);
  analogWrite(EnableL, 70);
  digitalWrite(HighR, LOW);
  digitalWrite(LowR, HIGH);
  analogWrite(EnableR, 255);
  delay(3400);
  L_R=0;
}

void tankLeft(){
  digitalWrite(HighL, HIGH);
  digitalWrite(LowL, LOW);
  digitalWrite(HighR, LOW);
  digitalWrite(LowR, HIGH);  //left
  analogWrite(EnableL, 250);
  analogWrite(EnableR, 250);
}

void tankRight(){
  digitalWrite(HighL, LOW);
  digitalWrite(LowL, HIGH);
  digitalWrite(HighR, HIGH);  //right
  digitalWrite(LowR, LOW);
  analogWrite(EnableL, 255);
  analogWrite(EnableR, 255);
}

void tankForward(){
  digitalWrite(HighL, LOW);
  digitalWrite(LowL, HIGH);
  digitalWrite(HighR, LOW);  // forward
  digitalWrite(LowR, HIGH);
  analogWrite(EnableL, 150);
  analogWrite(EnableR, 150);
}

void Object() {
  L_R=1;
  Stop();
  delay(1000);

  tankForward();
  delay(300);

  tankLeft();
  delay(500);

  Stop();
  delay(200);

  Forward();
  delay(1200);

  Stop();
  delay(200);
  L_R=0;


  tankRight();
  delay(400);

  Stop();
  delay(200);

  tankForward();
  delay(1100);

  L_R=2;
  Stop();
  delay(200);

  tankRight();
  delay(400);

  Stop();
  delay(200);

  Forward();
  delay(1200);

  Stop();
  delay(200);
  L_R=0;

  tankLeft();
  delay(500);

}

void stopSign(){
    analogWrite(EnableL, 0);
    analogWrite(EnableR, 0);
    delay(4000);

    analogWrite(EnableL, 150);
    analogWrite(EnableR, 150);
    delay(1000);
}



void loop() {

  Data();
  if (data == 7) {
    UTurn();
  }
  else if (data == 8) { //stop sign
    stopSign();
  }
  else if (data == 9) {
    Object();
  }
  else if (data == 0) {
    Forward();
    Serial.println("Forward");
  }
  else if (data == 1) {
    Right1();
    Serial.println("Right1");
  }
  else if (data == 2) {
    Right2();
    Serial.println("Right2");
  }
  else if (data == 3) {
    Right3();
    Serial.println("Right3");
  }
  else if (data == 4) {
    Left1();
    Serial.println("Left1");
  }
  else if (data == 5) {
    Left2();
    Serial.println("Left2");
  }
  else if (data == 6) {
    Left3();
    Serial.println("Left3");
  }
  else if (data > 10) {
    Stop();
    delay(3000);
  } 
}
