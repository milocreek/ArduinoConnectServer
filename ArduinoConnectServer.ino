// Filename ArduinoConnectServer.ino
// Version 1.5 08/15/13 RV MiloCreek
//

//#define WEBDUINO_SERIAL_DEBUGGING  1

#include "SPI.h"
#include "Ethernet.h"
#include "WebServer.h"
#include "MemoryFree.h"
#include "avr/pgmspace.h"



#include "Config.h"

long messageCount;

static uint8_t mac[] = LOCALMAC;


static uint8_t ip[] = LOCALIP;

// this is our current command object structure.  It is only valid inside void jsonCmd 
typedef struct {
    char ObjectID[40];
    char ObjectFlags[40];
    char ObjectAction[40]; 
    char ObjectName[40];
    char ObjectServerID[40];
    char Password[40];
    char ObjectType[40];
    char Validate[40]; 
} currentObjectStructure;
  
char *md5str;

char ST1Text[40];   // used in ST-1 Send text control



#include "MD5.h"

#include "smallJSON.h"

#include "ExecuteJSONCommand.h"




#include <stdarg.h>
void p(char *fmt, ... ){
        char tmp[128]; // resulting string limited to 128 chars
        va_list args;
        va_start (args, fmt );
        vsnprintf(tmp, 128, fmt, args);
        va_end (args);
        Serial.print(tmp);
}


// no-cost stream operator as described at 
// http://sundial.org/arduino/?page_id=119
template<class T>
inline Print &operator <<(Print &obj, T arg)
{ obj.print(arg); return obj; }

#define PREFIX ""

WebServer webserver(PREFIX, WEB_SERVER_PORT);

// commands are functions that get called by the webserver framework
// they can read any posted data from client, and they output to server

void jsonCmd(WebServer &server, WebServer::ConnectionType type, char *url_tail, bool tail_complete)
{

  currentObjectStructure parsedObject; 
  initParsedObject(&parsedObject);
  
#ifdef DEBUG 
Serial.println("------------------");
#endif    


  char myBuffer[1024];
  


 
  server.httpSuccess("application/json");
  
  if (type == WebServer::HEAD)
    return;
    
    int myChar;
    int count;
    myChar = server.read();
    count = 0;
 
    
    while (myChar > 0)
    {
     myBuffer[count] = (char) myChar;
     
      myChar = server.read();
      
      count++;

    }
    myBuffer[count] = '\0';

    delay(25);

    messageCount++;
 
    char returnJSON[500] = "";
   
  
   ExecuteCommandAndReturnJSONString(myBuffer, messageCount, md5str, &parsedObject, returnJSON, returnJSON);


#ifdef DEBUG
    Serial.print("returnJSON =");
    Serial.println(returnJSON);
#endif


  
  server << returnJSON;
  
#ifdef DEBUG
    Serial.print("Mem1=");
    Serial.println(freeMemory());
    Serial.println("------------------");
#endif
}





void setup()
{
  
  Serial.begin(9600);           // set up Serial library at 9600 bps
  
  Serial.print(F("ArduinoConnectServer "));
  Serial.print(VERSIONNUMBER);
  Serial.println(F(" 08/23/13")); 
  Serial.print(F("Compiled at:"));
  Serial.print (F(__TIME__));
  Serial.print(F(" "));
  Serial.println(F(__DATE__));
  
  p("ip address = %i.%i.%i.%i\n", ip[0], ip[1], ip[2], ip[3]);
  p("port number = %i\n", WEB_SERVER_PORT);
  p("mac address = %#X %#X %#X %#X %#X %#X\n", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

  randomSeed(analogRead(0));


  Ethernet.begin(mac, ip);
  webserver.begin();


  
  webserver.setDefaultCommand(&jsonCmd);
  webserver.addCommand("arduino", &jsonCmd);
  messageCount=0;
  
  
  unsigned char* hash=MD5::make_hash(PASSWORD);
  //generate the digest (hex encoding) of our hash
  md5str = MD5::make_digest(hash, 16);


}

void loop()
{
  // process incoming connections one at a time forever
  webserver.processConnection();
//Serial.println(freeMemory());

  // if you wanted to do other work based on a connecton, it would go here
}
