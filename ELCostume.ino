
#define 

// defines for setting and clearing register bits
#ifndef cbi
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif
#ifndef sbi
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#endif

// Rolling Average variables
const int bufferLength = 256;
int incomingReadings[bufferLength];
int readingIndex = 0;
boolean averageReady = false;

// minimum beat length for audio detection 
const long minChange = 500;

//Automatic display variables
byte state = 1;
const long minTimer = 1500;
const long maxTimer = 3500;
const long overlap = 375;

// Display variables
const byte numColors = 6;  // up to 8 can be used
const byte numActive = 2;

long nextChange = 2000;
long lastChange = 0;
byte currentColor[] = {0,1};
byte lastColor[] = {0,1};
byte nextColor[] = {0,1};

void setup() {                
  // The EL channels are on pins 2 through 9
  // Initialize the pins as outputs
  pinMode(2, OUTPUT);  // channel A  
  pinMode(3, OUTPUT);  // channel B   
  pinMode(4, OUTPUT);  // channel C
  pinMode(5, OUTPUT);  // channel D    
  pinMode(6, OUTPUT);  // channel E
  pinMode(7, OUTPUT);  // channel F
  pinMode(8, OUTPUT);  // channel G
  pinMode(9, OUTPUT);  // channel H

  // Set ADC to 77khz, max for 10bit
  sbi(ADCSRA,ADPS2);
  cbi(ADCSRA,ADPS1);
  cbi(ADCSRA,ADPS0);

  // Start serial line for debug
  Serial.begin(9600);

  // initialize randomizer
  randomSeed(analogRead(4));

  // Read status of audio jack
  pinMode(A3, INPUT);
  digitalWrite(A3, HIGH);
}

// Main Loop
void loop() 
{
  if (digitalRead(A3) == LOW){
    ELAudio();
  } else {
    ELCycle();
  }
}

// Display code for change on beat
void ELAudio(void){
  boolean beat = readAudio();
  if (millis() - lastChange > minChange && beat){
    lastChange = millis();
    for (int i = 0; i < numActive; i++){
      Serial.print("Turning off ");
      Serial.println(currentColor[i]);
      ELoff(currentColor[i]);
    }
    for (int i = 0; i < numActive; i++){
      nextColor[i] = getNextColor(currentColor);
      Serial.print("Turning on ");
      Serial.println(nextColor[i]);
      ELon(nextColor[i]);
    }
    for (int i = 0; i < numActive; i++){
      currentColor[i] = nextColor[i];
    }
  }
}
 
// Display code for standalone mode
void ELCycle(void){
  long now = millis();
  if (now > (lastChange + overlap) && state == 1){
    state = 2;
    for (int i = 0; i < numActive; i++){
      Serial.print("Turning off ");
      Serial.println(lastColor[i]);
      ELoff(lastColor[i]);
    }
  }
  if (now > (nextChange - overlap) && state == 2){
    state = 3;
    for (int i = 0; i < numActive; i++){
      nextColor[i] = getNextColor(currentColor);
      Serial.print("Turning on ");
      Serial.println(nextColor[i]);

      ELon(nextColor[i]);
    }
  }
  if (now > nextChange){
    state = 1;
    lastChange = now;
    long pulseLength = random(minTimer, maxTimer);
    Serial.print("Next Pulse is ");
    Serial.println(pulseLength);
    
    nextChange = now + pulseLength;

    for (int i = 0; i < numActive; i++){
      lastColor[i] = currentColor[i];
      currentColor[i] = nextColor[i];
    }
  }
}

void ELon(byte pin){
  digitalWrite(pin+2, HIGH);
}

void ELoff(byte pin){
  digitalWrite(pin+2, LOW);
}

byte getNextColor(byte current[]){
  byte next;
  do {
    next = (byte)random(0,numColors);
  } while (isValueIn(next,current));
  return next;
}

boolean isValueIn(byte value, byte arr[]){
    for (int i = 0; i < sizeof(arr); i++){
    if (value == arr[i]){
      return true;
    }
  }
  return false;
}

float rollingAverage(int value){
  readingIndex = ++readingIndex % bufferLength;
  incomingReadings[readingIndex] = value;
  if (!averageReady && readingIndex == 0){
    averageReady = true;
  }
  long total = 0;
  for (int i = 0; i < bufferLength;i++){
    total += incomingReadings[i];
  }
  float avg = total / (float)bufferLength;
  return (avg);
}
