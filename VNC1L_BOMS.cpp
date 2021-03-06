/*
 VNC1L_BOMS by falstaff
 
 VNC1L_BOMS is a library to write on USB Flash Disk's using the FTDI's VNC1L 
 chip. BOMS means Bulk Only Mass Storage is a general description for USB Flash
 Disk's.
 
 Dependencies:
 - NewSoftSerial library
 
 created 10 July 2011
 by Stefan Agner

 http://falstaff.agner.ch/lightscythe/

*/

#include <WProgram.h>
#include "VNC1L_BOMS.h"

#define DEFAULT_BAUD 9600

VNC1L_BOMS::VNC1L_BOMS(long baud, byte pin_rx, byte pin_tx)
  : _vnc1l(pin_rx, pin_tx) {
  _baud = baud;
  _vnc1l.begin(DEFAULT_BAUD);
}

void VNC1L_BOMS::sync() {
  // If there are no data yet, the VNC1L probably did not rebooted... Send a newline to get a prompt!
  if(!_vnc1l.available())
    _vnc1l.print(0x0D, BYTE);
  // Waiting for initial prompt, show output on console...
  waitforprompt(true);
  
  // Switch to ASCII mode
  _vnc1l.print("IPA");
  _vnc1l.print(0x0D, BYTE);
  waitforprompt();
  
  // Switching baudrate
  _vnc1l.print("SBD $");
  if(_baud == 9600)
    _vnc1l.print(0x384100, HEX); // 9600
  else if(_baud == 19200)
    _vnc1l.print(0x9C8000, HEX); // 19200
  else if(_baud == 38400)
    _vnc1l.print(0x4EC000, HEX); // 38400
  else if(_baud == 57600)
    _vnc1l.print(0x34C000, HEX); // 57600
  _vnc1l.print(0x0D, BYTE);
  Serial.print("Switching baudrate to ");
  Serial.println(_baud);
  waitforprompt();
  delay(10);
  _vnc1l.begin(_baud);
  delay(50);
  waitforprompt();
  Serial.println("Switching succeeded");
}

static char cf[] = "Command Failed";
static char bc[] = "Bad Command";

boolean VNC1L_BOMS::waitforprompt(boolean show)
{
  char msg[30];
  int i = 0;
  
  // Wait for prompt...
  while(true){
    if(_vnc1l.available())
    {
      msg[i] = _vnc1l.read();
      if(msg[i] == '\r')
      {
        msg[i] = 0;
        if(show)
          Serial.println(msg);
        if(msg[i-1] == '>')
          return true;
        else if(!strcmp(msg, cf))
        {
          Serial.println("Command Failed");
          return false;
        }
        else if(!strcmp(msg, bc))
        {
          Serial.println("Bad Command");
          return false;
        }
        i = 0;
      }
      else
        i++;
    }
  }
}

boolean VNC1L_BOMS::file_open(const String &file) {
  _vnc1l.print("OPR ");
  _vnc1l.print(file);
  _vnc1l.print(0x0D, BYTE);
  return waitforprompt();
}

boolean VNC1L_BOMS::file_seek(long offset) {
  _vnc1l.print("SEK $");
  _vnc1l.print(offset, HEX);
  _vnc1l.print(0x0D, BYTE);
  return waitforprompt();
}

void VNC1L_BOMS::file_read(int count, byte buffer[]) {
  int done = 0;
  _vnc1l.print("RDF $");
  _vnc1l.print(count, HEX);
  _vnc1l.print(0x0D, BYTE);
  
  while(done < count)
  {
    if(_vnc1l.available())
    {
      buffer[done] = _vnc1l.read();
      done++;
    }
  }
  waitforprompt();
}

void VNC1L_BOMS::file_close(const String &file) {
  _vnc1l.print("CLF ");
  _vnc1l.print(file);
  _vnc1l.print(0x0D, BYTE);
  waitforprompt();
}



