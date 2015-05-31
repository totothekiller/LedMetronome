
#include <Adafruit_NeoPixel.h>

#define PIN            10
#define NUMPIXELS      16
#define XMAX      15

#define RESETBUTTONPIN      2
#define TEMPOBUTTONPIN      3

#define NBRTAP  8

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

unsigned long _previousTime = 0;

double _bpm;

double _position;

int _direction = 1;

boolean _tick = false;
unsigned long _previousTick = 0;

int _previousResetState;

int _previousTempoState;
unsigned long _tempos[NBRTAP];
unsigned int _currentTemposIndex;


void setup() {
  // put your setup code here, to run once:
  pixels.begin(); // This initializes the NeoPixel library.


  // initialize the pushbutton pin as an input:
  pinMode(RESETBUTTONPIN, INPUT_PULLUP);
  pinMode(TEMPOBUTTONPIN, INPUT_PULLUP);

  _bpm = 120;
  
  Serial.begin(9600);
  
  }

void loop() {

  unsigned long currentTime = micros();// * 1.549;
  unsigned long delta = currentTime - _previousTime;

  // Handle Button
  userButton(currentTime);

  // Conpute Postition
  engine(currentTime, delta);

  // Paint !
  paint(currentTime);

  _previousTime = currentTime;
}


//
//
//
void engine(unsigned long currentTime, unsigned long dt)
{

  double speed = _bpm * XMAX  /  60.0 ;

  _position = _position + _direction * speed * dt / 1000000.0;


  if (_position > XMAX)
  {
    _direction = -1;
    _position -=  _position - XMAX;
  }

  else if (_position < 0.0 )
  {
    _tick = true;
    _previousTick = currentTime;

    _direction = 1;
    _position = - _position;
  }

}

// Display
void paint(unsigned long currentTime)
{
  
  // 70 ms of flash
  boolean flash = currentTime - _previousTick < 70000;

  for (int p = 0; p < NUMPIXELS; p++) {

    if (flash)
    {
      pixels.setPixelColor(p, pixels.Color(00, 00, 50));

    } else
    {
      pixels.setPixelColor(p, 0, 0, 0);
    }
  }


  int i = _position + 0.5;

  pixels.setPixelColor(i, pixels.Color(0, 150, 0)); // Moderately bright green color.

  pixels.show(); // This sends the updated pixel color to the hardware.

}


void userButton(unsigned long currentTime)
{

  // RESET
  int stateReset = digitalRead(RESETBUTTONPIN);

  if (stateReset ==  LOW && _previousResetState == HIGH)
  {
    // Reset Position
    _tick = true;
    _previousTick = currentTime;
    _direction = 1;
    _position = 0.0;
    
    // Reset Tempo
    _currentTemposIndex = 0;
  }
  _previousResetState = stateReset;


  // TEMPO

  int stateTempo = digitalRead(TEMPOBUTTONPIN);

  if (stateTempo ==  LOW && _previousTempoState == HIGH)
  {
    Serial.print("Tap\t");Serial.print(_currentTemposIndex);Serial.print("\t");Serial.println(currentTime); 
    
    _tempos[_currentTemposIndex] = currentTime;
    
    _currentTemposIndex = (_currentTemposIndex + 1) % NBRTAP ;
    
    if(_currentTemposIndex == 0)
    {
      unsigned long sum = 0;
      
      for (int i = 0; i < NBRTAP-1; i++) {        
        sum += _tempos[i+1] - _tempos[i];
       }
       
       //Serial.print("Sum ="); Serial.println(sum); 
       
      double mean = sum / (NBRTAP-1);
       
       // Serial.print("Mean ="); Serial.println(mean); 
      
      _bpm = 60000000.0 / mean;
      
      Serial.print("BPM\t"); Serial.println(_bpm); 
      
          // Reset Position
    //_tick = true;
    //_previousTick = currentTime;
    _direction = -1;
    _position = XMAX;
      
    }
    
  }
  
  _previousTempoState = stateTempo;

}
