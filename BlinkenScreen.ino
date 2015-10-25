#include <SPI.h>

SPISettings spi_settings(2000, LSBFIRST, SPI_MODE0); 

#define BUFFER_SIZE 6   // Size of screen frame buffer
#define BUTTON_PIN  7   // Analog pin 7
#define BUTTON_THRESHOLD 5 // How many cycles before button considered pressed

// Buttons
#define BIDLE 10 // Nothing pressed
#define B1 5
#define B2 4
#define B3 3
#define B4 7
#define B5 2
#define B6 0

typedef union screen {
  char b[BUFFER_SIZE];
  uint64_t i;
};

const uint64_t conversion_table[] = {
  // Row 1
  0x000010000000,
  0x000002000000,
  0x000400000000,
  0x000080000000,
  0x010000000000,
  0x002000000000,
  0x400000000000,
  0x080000000000, 

  // Row 2
  0x000008000000,
  0x000001000000,
  0x000200000000,
  0x000040000000,
  0x008000000000,
  0x001000000000,
  0x200000000000,
  0x040000000000,

  // Row 3
  0x000020000000,
  0x000004000000,
  0x000800000000,
  0x000100000000,
  0x020000000000,
  0x004000000000,
  0x800000000000,
  0x100000000000,

  // Sentinal
  0x00
};

enum modes {
  RAW_WIPE,
  WIPE,
  MODE_SENTINAL
};

screen s;

uint64_t massage(uint64_t data) {
  uint64_t scratch = 0;
  
  // Switch from left side to right
  for (char j = 0; j <= 1; j++) {
    scratch >>= 24;
    
    // Loop over the half display
    for (char i = 0; i <= 24; i++) {
      if (data & 1) {
        scratch |= conversion_table[i];
      }
      data = data >> 1;
    }
  }
  
  return scratch;
}

void setup() {
  Serial.begin(115200);
  SPI.begin();
  
  s.i = 0;
}

char button_last;
char button_confidence = 0;
char read_buttons()
{
  char val = analogRead(BUTTON_PIN) / 100; // Analog values are in the hundreds

  // Keep track of how many loops we've been the
  // same value
  if (val == button_last) {
    if (button_confidence <= BUTTON_THRESHOLD) {
      button_confidence++;
    }
  } else {
    button_confidence = 0;
    button_last = val;
    return 0;
  }

  // Only consider a button pressed if we've
  // seen the same value for five loops
  if (button_confidence != 5) {
    return 0;
  }

  return val;
}

inline void wipe() {
  static uint64_t i = 0;
  
  i = i == 0 ? 1 : i << 1;

  s.i = massage(i);
}

inline void raw_wipe() {
  // Show single pixel walking across the display
  s.i = s.i == 0 ? 1 : s.i << 1;
}

// NOTE: We want to pass a copy because SPI.transfer is destructive
int print_framebuffer(union screen framebuffer) {
  // Screen is active low, so modify before pushing it out  
  framebuffer.i = ~framebuffer.i;

  SPI.beginTransaction(spi_settings);
  SPI.transfer(framebuffer.b, BUFFER_SIZE);
  SPI.endTransaction();
}

// Unsigned allows us to modulo to correct value on overflow
unsigned char mode = WIPE;
void loop() {
  char button = read_buttons();

  switch (mode)
  {
    case RAW_WIPE:
      raw_wipe();
      break;
      
    case WIPE:
      wipe();
      break;

    // Outside range, correct it
    default:
      mode %= MODE_SENTINAL;
  }
  
  print_framebuffer(s);

  delay(10);
  
  if (button == B1)
    mode++;
  else if (button == B2)
    mode--;
}
