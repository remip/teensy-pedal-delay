// #define WITH_USB
#define WITH_SERIAL

#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SerialFlash.h>
#include "pedal.h"
#include <Bounce.h>
#include <AnalogSmooth.h> // https://michaelthessel.com/analog-smoothing-library-for-arduino/


// GUItool: begin automatically generated code
AudioInputI2S            audioInput;     //xy=148,145
AudioEffectBitcrusher    bitcrusher;    //xy=318,254
AudioMixer4              mixerFeedback;  //xy=327,361
AudioEffectDelay         delayLine;      //xy=385,580
AudioMixer4              mixerOutput;         //xy=499,167
AudioAmplifier           ampVolume;           //xy=689,162
AudioOutputI2S           audioOutput; //xy=887,164
AudioConnection          patchCord1(audioInput, 1, mixerOutput, 0);
AudioConnection          patchCord2(audioInput, 1, bitcrusher, 0);
AudioConnection          patchCord3(bitcrusher, 0, mixerFeedback, 0);
AudioConnection          patchCord4(mixerFeedback, delayLine);
AudioConnection          patchCord5(delayLine, 0, mixerFeedback, 1);
AudioConnection          patchCord6(delayLine, 0, mixerOutput, 1);
AudioConnection          patchCord7(delayLine, 1, mixerFeedback, 2);
AudioConnection          patchCord8(delayLine, 1, mixerOutput, 2);
AudioConnection          patchCord9(mixerOutput, ampVolume);
AudioConnection          patchCord10(ampVolume, 0, audioOutput, 1);
AudioConnection          patchCord11(ampVolume, 0, audioOutput, 0);
AudioControlSGTL5000     audioBoard;     //xy=156,64
// GUItool: end automatically generated code


#ifdef WITH_USB
AudioOutputUSB           usb;           //xy=876,216
AudioConnection          patchCord103(ampVolume, 0, usb, 0);
AudioConnection          patchCord104(ampVolume, 0, usb, 1);
#endif


// bypass push button
Bounce b_bypass = Bounce(SW_BYPASS, 15);

// smooth delay pot
AnalogSmooth delay_s = AnalogSmooth(40);


int basedelay = 750; // in ms
float mix = 0.7;
float feedback = 0.2;

float ratio = 1.6180339;

void setup() {

#ifdef WITH_SERIAL
  Serial.begin(38400); // open the serial
#endif

  init_pedal();
  
  // Audio connections require memory to work.  For more
  // detailed information, see the MemoryAndCpuUsage example
  AudioMemory(600);
  
  // Enable the audio shield, select input, and enable output
  audioBoard.enable();
  audioBoard.inputSelect(AUDIO_INPUT_LINEIN);
  audioBoard.dacVolumeRamp();
  audioBoard.adcHighPassFilterDisable();
  audioBoard.volume(0.5); // phone output

  bitcrusher.bits(16);
  bitcrusher.sampleRate(44100);
  
  mixerOutput.gain(0, 1.0); // DRY
  mixerOutput.gain(1, mix); // DELAY1
  mixerOutput.gain(2, mix); // DELAY2
  mixerOutput.gain(3, 0.0); // unused
  delayLine.delay(0, basedelay);
  delayLine.delay(1, int(basedelay*ratio));

  mixerFeedback.gain(0, 0.0); // effect off
  mixerFeedback.gain(1, feedback);
  mixerFeedback.gain(2, feedback);
  
#ifdef WITH_SERIAL
  Serial.println("Setup done");
#endif

  AudioProcessorUsageMaxReset();
  AudioMemoryUsageMaxReset();  
}

unsigned long last_time = millis();
unsigned long last_pot_check = millis();
float f, val2 = 0.0, val3 = 0.0, val4 = 0.0, volume = 0.0;
int led_state = 0;
int b = 0;
int m = 0;
int bypass_state = 0;
int i = basedelay;
int delay_lines = 2;

int changed = 0;

void loop() {

  
    // Time POT
    i = delay_s.smooth(get_pot_i(POT1, 0, 750));  
    if ( abs(i - basedelay) >= 5 ) {
      basedelay = i;
      if ( millis() - last_pot_check >= 250 ) {
        if ( delay_lines == 2 )
          delayLine.delay(0, basedelay);
        delayLine.delay(1, int(basedelay*ratio));
        changed++;
        last_pot_check = millis();
      }
    }

    // Feedback POT
    f = get_pot_f(POT2, 0.0, 0.5);
    if (f != feedback) {
      feedback = f;
      mixerFeedback.gain(1, feedback);
      mixerFeedback.gain(2, feedback);
    }

    // Mix POT
    f = get_pot_f(POT4, 0.0, 1.0);
    if (f != mix) {
      mix = f;
      mixerOutput.gain(1, mix); // DELAY1
      mixerOutput.gain(2, mix); // DELAY2
    }
  
    // Volume POT
    f = get_pot_f(POT5, 0.0, 2.0);
    if (f != volume) {
      volume = f;
      ampVolume.gain(volume);
    }

  // delay lines
  if(digitalRead(SW_BANK) == LOW) {
    delay_lines = 2;
    delayLine.delay(0, basedelay);
  } else {
    delay_lines = 1;
    delayLine.disable(0);
  }

  // bypass switch
  b_bypass.update();
  if ( b_bypass.fallingEdge() ) {
    if (bypass_state == 0) {
      // effect ON
      mixerFeedback.gain(0, 1.0);        
      analogWrite(LED_BYPASS, LED_INTENSITY);
      bypass_state = 1;
    } else {
      // effect OFF
      mixerFeedback.gain(0, 0.0);
      analogWrite(LED_BYPASS, 0);
      bypass_state = 0;
    }      
  }


  if(millis() - last_time >= 1000) { 

    f = get_pot_f(POT3, 0.0, 1.0); if (f != val3) val3 = f;
    f = get_pot_f(POT4, 0.0, 1.0); if (f != val4) val4 = f;

      
    m = get_mode();

    if ( m == 0) {
      analogWrite(LED_TAP_R, 255);
      analogWrite(LED_TAP_G, 0);
      bitcrusher.bits(16);
      bitcrusher.sampleRate(44100);  
    } else if ( m == 1 ) {
      analogWrite(LED_TAP_R, 0);
      analogWrite(LED_TAP_G, 255);
      bitcrusher.bits(12);
      bitcrusher.sampleRate(22500);  
    } else {
      analogWrite(LED_TAP_R, 255);
      analogWrite(LED_TAP_G, 255);
      bitcrusher.bits(8);
      bitcrusher.sampleRate(11250);  
    }
#ifdef WITH_SERIAL
    Serial.print("Proc = ");
    Serial.print(AudioProcessorUsage());
    Serial.print(" (");    
    Serial.print(AudioProcessorUsageMax());
    Serial.print(" max),  Mem = ");
    Serial.print(AudioMemoryUsage());
    Serial.print(" (");    
    Serial.print(AudioMemoryUsageMax());
    Serial.println(" max)");

    Serial.print("TIME=");
    Serial.print(basedelay);
    Serial.print(" (changed=");
    Serial.print(changed);
    Serial.print(") FEEDBACK=");
    Serial.print(feedback);
    Serial.print(" POT3=");
    Serial.print(val3);
    Serial.print(" MIX=");
    Serial.print(val4);
    Serial.print(" VOL=");
    Serial.print(volume);
    Serial.print(" LINES=");
    Serial.print(delay_lines);
    Serial.print(" MODE=");
    Serial.print(m);
    Serial.print(" BYPASS=");
    Serial.println(bypass_state);
#endif   
    last_time = millis();
  }   

  delay(50);
}
