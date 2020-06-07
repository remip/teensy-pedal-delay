
#define POT1 33
#define POT2 34
#define POT3 35
#define POT4 36
#define POT5 37

#define SW_BANK 31
#define SW_MODE0 25
#define SW_MODE1 26
#define SW_BYPASS 27
#define SW_TAP 28
#define LED_BYPASS 38
#define LED_TAP_R 30
#define LED_TAP_G 29

#define DMIN 1
#define DMAX 1023

#define BANK_A 0
#define BANK_B 1

#define LED_INTENSITY 255

void init_pedal(void) {
  pinMode(SW_BANK, INPUT_PULLUP);
  pinMode(SW_MODE0, INPUT_PULLUP);
  pinMode(SW_MODE1, INPUT_PULLUP);
  pinMode(SW_BYPASS, INPUT_PULLUP);
  pinMode(SW_TAP, INPUT_PULLUP);
}

int get_pot_i(int pin, int min, int max) {  
  return analogRead(pin) * max / DMAX;
}

float get_pot_f(int pin, float min, float max) {
  return analogRead(pin) * max / DMAX;
}

int get_bank(void) {
  if(digitalRead(SW_BANK) == LOW) {
    return BANK_B;
  }
  return BANK_A;
}

int get_mode(void) {
  int a = (digitalRead(SW_MODE0) == LOW);
  int b = (digitalRead(SW_MODE1) == LOW);

  if (a) {
    return 2;
  } else if(b) {
    return  0;
  }
  return 1;  
}

int get_bypass(void) {
  if(digitalRead(SW_BYPASS) == LOW)
    return 1;
  return 0;
}
