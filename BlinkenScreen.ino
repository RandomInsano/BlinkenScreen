#include <SPI.h>

SPISettings spi_settings(2000, LSBFIRST, SPI_MODE0); 

const int BUFFER_SIZE = 6;

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
  
  // Reverse since LED logic is inverted
  return ~scratch;
}

void setup() {
//  Serial.begin(9600);
  SPI.begin();
  
  s.i = 0;
}
  
char mode = 0;
void loop() {
  s.i = 1;
  mode ^= 1;
  screen temp;

  while (s.i != 0) {
    // Translate or not
    temp.i = mode ? ~s.i : massage(s.i);
    
    SPI.beginTransaction(spi_settings);
    SPI.transfer(temp.b, BUFFER_SIZE);
    SPI.endTransaction();
  
    s.i = s.i << 1;
      
    delay(80);
  }
}
