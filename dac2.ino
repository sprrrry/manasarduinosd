/*
  dac2.ino

  Interrupt timer used to output a wave file to PORTA
  On a MEGA PORTA is pins 22-29
  PORTA is connected to a DAC
  For Uno use REPLACE PORTA WITH PORTD (pins 0-7)

  Least frequency deviation achieved by utilising Timer 2 interrupt
  With 8 prescalar

  Timer 2 has a finite resolution as it is ultimately dependent on integer values
  set in a single byte: OCR2A = setocroa where 0 >= setocra <=255
  Thus requested frequencies may be a best fit-
  Some will be better than others- its a modular maths factor.

  Timer 1 - the frequency increments are too big
  (Small changes in requested frequency can not be reflected in interrupt period)
  Timer 0 appears to affect millis()

*/

// Defines for clearing register bits
#ifndef cbi
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif
// Defines for setting register bits
#ifndef sbi
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#endif

#include <SD.h>
#include <SPI.h>
// Open serial communications and wait for port to open:

// set up variables using the SD utility library functions:
Sd2Card card;
SdVolume volume;
SdFile root;
const int chipSelect = 53; // (Or any suitable pin!)
const int defaultselect = 53; // mega
boolean hascard = false;
File tempfile, folder;
char bufFile[] = "/adlog/00112233.wav";
unsigned long starttime, stoptime, counter, readings, played;
float frequency = 9300;
boolean finished = false;
#define BUF_SIZE 512
uint8_t bufa[BUF_SIZE];
uint8_t bufb[BUF_SIZE];
uint16_t bufcount;
byte headbuf[60]; // will read the header into this

byte scroll = 4; // set the number of wav plays before the folder is re-displayed
// alter this if your serial monitor height needs it
boolean aready, readit;

void setup() {
  if (chipSelect != defaultselect) {
    pinMode(defaultselect, OUTPUT);
  }
  Serial.begin(9600); // start serial for output
  Serial.flush();
  played = 0;


  //---------------------------------------------------------------------
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  Serial.print("Initializing SD card...");

  if (!SD.begin(53)) {
    Serial.println("initialization failed!");
    while (1);
  }
  Serial.println("initialization done.");
  //---------------------------------------------------------------------
  /* wavheader setup
    // little endian (lowest byte 1st)
    byte wavheader[44];
    wavheader[0]='R';
    wavheader[1]='I';
    wavheader[2]='F';
    wavheader[3]='F';
    //wavheader[4] to wavheader[7] size of data + 44 -8
    wavheader[8]='W';
    wavheader[9]='A';
    wavheader[10]='V';
    wavheader[11]='E';
    wavheader[12]='f';
    wavheader[13]='m';
    wavheader[14]='t';
    wavheader[15]=' ';
    wavheader[16]=16;
    wavheader[17]=0;
    wavheader[18]=0;
    wavheader[19]=0;
    wavheader[20]=1;
    wavheader[21]=0;
    wavheader[22]=1;
    wavheader[23]=0;
    //wavheader[24] to wavheader[27] samplerate hz
    //wavheader[28] to wavheader[31] samplerate*1*1
    wavheader[32]=1;
    wavheader[33]=0;
    wavheader[34]=8;
    wavheader[35]=0;
    // optional fields can appear here with some software
    // forcing following fields downadele11.wav

    wavheader[36]='d';
    wavheader[37]='a';
    wavheader[38]='t';
    wavheader[39]='a';
    //wavheader[40] to wavheader[43] sample number
  */

  Serial.println(F("\nFast D/A"));

  // On Mega PORTA is pins 22-29
  // set digital pins PORTA as output
  DDRA = B11111111;

  // On UNO PORTD is pins 0-7
  // set digital pins PORTD as output
  //DDRD = B11111111;

  // Set port to half voltage
  PORTA = 128;
  //PORTD=128;
  if (SD.begin(chipSelect)) {
    hascard = true;
    card.init(SPI_FULL_SPEED, chipSelect);
  } else {
    Serial.println(F("SD card problem!"));
    while (1); // no point in continuing
  }
  root.openRoot(volume);
  folder = SD.open("/adlog/");
  Serial.println();
  Serial.println(F("adlog directory"));
  if (printDirectory(folder, 0) == 0) {
    Serial.println(F("No wav files, nothing to do!"));
    while (1);
  }
  // test here
  getfile();
  frequency = fileopen();
  if (frequency == 0) Serial.println(F("\nFrequency problem"));
  readings = datasize();
  if (datasize == 0) Serial.println(F("\ndata size problem"));

  if ((frequency > 0) && (datasize > 0)) setintrupt(frequency);
}

void setintrupt(float freq) {
  float bitval = 8; // 8 for 8 bit timers 0 and 2, 1024 for timer 1
  byte setocroa = (16000000 / (freq * bitval)) - 0.5; // -1 +0.5
  // The resolution of the timer is limited-
  // Ultimately determined by magnitude of bitval

  //Serial.print(F("Frequency scalar "));
  //Serial.println(setocroa);

  bufcount = 0; // initialise counters
  counter = 0;
  played++;

  Serial.println(F("Outputting File to PORT"));
  cli();//disable interrupts
  //set timer2 interrupt
  TCCR2A = 0;// set entire TCCR2A register to 0
  TCCR2B = 0;// same for TCCR2B
  TCNT2  = 0;//initialize counter value to 0
  // set compare match register for frequency (hz) increments
  OCR2A = setocroa;// = (16*10^6) / (frequency*8) - 1 (must be <256)
  // turn on CTC mode
  TCCR2A |= (1 << WGM21);
  // Set CS21 bit for 8 prescaler
  TCCR2B |= (1 << CS21);
  // enable timer compare interrupt
  //  TIMSK2 |= (1 << OCIE2A);
  sbi(TIMSK2, OCIE2A); //  enable interrupt on timer 2
  aready = true;
  readit = true; // need to start reading bufb!
  starttime = millis();
  sei();//enable interrupts
}

// interrupt routine! ***************************************
ISR(TIMER2_COMPA_vect) { //interrupt routine timer 2
  if (counter < readings) {
    if (aready) {
      PORTA = bufa[bufcount];
    } else {
      PORTA = bufb[bufcount];
    }
    /*
      if(aready) {
      PORTD=bufa[bufcount];
      } else {
      PORTD=bufb[bufcount];
      }
    */
    counter++;
    bufcount++;
    if (bufcount == BUF_SIZE) {
      if (readit == false) { // file reading not going on
        bufcount = 0;
        aready = ! aready;
        readit = true;
      } else {
        // file reading still going on so backup and wait
        counter--;
        bufcount--;
      }
    }
  } else {
    cli();
    stoptime = millis();
    cbi(TIMSK2, OCIE2A); // disable interrupt
    PORTA = 128;
    //PORTD=128;
    finished = true;
    tempfile.close();
    sei();
  }
}
// End interrupt ********************************************

/*set timer1 interrupt at frequency for information
  TCCR1A = 0;// set entire TCCR1A register to 0
  TCCR1B = 0;// same for TCCR1B
  TCNT1  = 0;//initialize counter value to 0
  // set compare match register for frequency increments
  OCR1A = setocroa;// = (16*10^6) / (1*1024) - 1 (must be <65536)
  // turn on CTC mode
  TCCR1B |= (1 << WGM12);
  // Set CS10 and CS12 bits for 1024 prescaler
  TCCR1B |= (1 << CS12) | (1 << CS10);
  // enable timer compare interrupt
  //  TIMSK1 |= (1 << OCIE1A);
  sbi(TIMSK1,OCIE1A);

  // Additional interupt routines for experimentation:

  ISR(TIMER0_COMPA_vect){ //interrupt routine timer 0
  if (counter<2*frequency) {
  counter++;
  // do stuff here
  } else {
  cli();
  stoptime = millis();
  cbi(TIMSK0,OCIE0A); // disable interrupt
  finished=true;
  sei();
  }
  }

  ISR(TIMER1_COMPA_vect){ //interrupt routine timer 1
  if (counter<2*frequency) {
  counter++;
  // do stuff here
  } else {
  cli();
  stoptime = millis();
  cbi(TIMSK1,OCIE1A); // disable interrupt
  finished=true;
  sei();
  }
  }
*/

void loop() {
  if (readit) {
    if (! aready) {
      // initiate SDCard block read to bufa
      tempfile.read(bufa, BUF_SIZE);
    } else {
      // initiate SDCard block read to bufb
      tempfile.read(bufb, BUF_SIZE);
    }
    readit = false;
  }
  if (finished == true) {
    finished = false;
    Serial.println(F("Output completed"));
    Serial.print(F("Time "));
    Serial.println(float(stoptime - starttime) / 1000, 3);
    Serial.print(F("Frequency "));
    float measured = float(counter) * 1000 / float(stoptime - starttime);
    Serial.print(measured, 1);
    Serial.print(F(" , Deviation from required "));
    Serial.print(measured - frequency, 1);
    Serial.println(F(" Hz"));
    if (played % scroll == 0) { // Put a new copy on serial monitor so that
      // the monitor need not be scrolled to read directory
      Serial.println(F("\nadlog directory"));
      folder = SD.open("/adlog/");
      printDirectory(folder, 0);
    }
    getfile();
    frequency = fileopen();
    if (frequency == 0) Serial.println(F("\nFrequency problem"));
    readings = datasize();
    if (datasize == 0) Serial.println(F("\ndata size problem"));

    if ((frequency > 0) && (datasize > 0)) setintrupt(frequency);
  }
}

void getfile() {
  boolean gotname = false;
  char junk = ' ';
  Serial.println();
  // try to clear any stray serial input
  while (Serial.available() > 0) {
    junk = Serial.read() ;
  }
  Serial.println(F("Which file?"));
  //

  do { // wait for a valid filename from the serial input
    int charno = "00112233.wav";
    if (charno > 4) { // must be at least x.wav
      if (charno > 12) charno = 12; // 12 characters max for a filename
      for (int j = 7; j < 19; j++) {
        bufFile[j] = 0; // clear any previous filename
      }
      Serial.readBytes(bufFile + 7, charno);
      if (SD.exists(bufFile)) {
        gotname = true;
        Serial.print(F("File selected "));
        Serial.println(bufFile);
      } else {
        // try to clear any stray serial input
        while (Serial.available() > 0) {
          junk = Serial.read() ;
        }
        Serial.print(F("No such File "));
        Serial.println(bufFile);
        Serial.println();
      }
    } else {
      // try to clear any stray serial input
      while (Serial.available() > 0) {
        junk = Serial.read() ;
      }
      delay(250);
    }
  } while (!gotname);
}

int printDirectory(File dir, int numTabs) {
  String temp;
  int fcount = 0;
  while (true) {
    File entry =  dir.openNextFile();
    if (! entry) {
      // no more files
      Serial.println(F("*****************************************"));
      return (fcount);
      break;
    }
    for (uint8_t i = 0; i < numTabs; i++) Serial.print(F("          "));
    int mylen = strlen(entry.name()) - 4; // compare last 4 characters
    if ((strcmp(entry.name() + mylen, ".WAV") == 0) || (strcmp(entry.name() + mylen, ".wav") == 0)) {
      fcount++;
      temp = entry.name();
      temp.toLowerCase();
      Serial.print(temp);
      for (uint8_t i = mylen; i < 30; i++) Serial.print(F(" "));
      Serial.println(entry.size(), HEX);
    }

    if (entry.isDirectory()) {
      Serial.println(F("/"));
      printDirectory(entry, numTabs + 1);
    }
  }
}

unsigned long fileopen() {
  unsigned long retval;
  //byte tbuf[4];
  tempfile = SD.open(bufFile, FILE_READ);
  if (!tempfile) {
    return (0);
  } else {
    // read frequency specified in wav file header
    tempfile.seek(0);
    tempfile.read(headbuf, 60);
    retval = headbuf[27];
    retval = (retval << 8) | headbuf[26];
    retval = (retval << 8) | headbuf[25];
    retval = (retval << 8) | headbuf[24];
    Serial.print(F("File Frequency "));
    Serial.print(retval);
    return (retval);
  }
}

unsigned long datasize() {
  // read data size specified in wav file header
  unsigned long retval;

  // look for the word "data" in header
  // some software adds optional fields to the header
  // altering the position of the data start and data length fields
  // but expect to find the data length  and data, after the word "data"

  int mypos = 40;
  for (int i = 36; i < 60; i++) {
    if (headbuf[i] == 'd') {
      if (headbuf[i + 1] == 'a') {
        if (headbuf[i + 2] == 't') {
          if (headbuf[i + 3] == 'a') {
            // at last we have it
            mypos = i + 4;
            i = 60;
          }
        }
      }
    }
  }
  tempfile.seek(mypos);
  retval = headbuf[mypos + 3];
  retval = (retval << 8) | headbuf[mypos + 2];
  retval = (retval << 8) | headbuf[mypos + 1];
  retval = (retval << 8) | headbuf[mypos];
  tempfile.seek(mypos + 4);
  unsigned long nowtime, endtime;

  nowtime = micros();
  // Read 1st block of data
  tempfile.read(bufa, BUF_SIZE);
  endtime = micros();

  Serial.print(F(" , Data size "));
  Serial.print(retval);
  Serial.print(F("b Data speed "));
  Serial.print(BUF_SIZE);
  Serial.print(F("b in "));
  Serial.print(float(endtime - nowtime) / 1000, 2);
  Serial.println(F(" mS"));
  Serial.print(F("Sample period "));
  Serial.print(1000 / frequency, 2);
  Serial.println(F(" mS"));
  return (retval);
}
