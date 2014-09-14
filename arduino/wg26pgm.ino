#include <SoftwareSerial.h>  // SoftwareSerial must be included because the library depends on it
#include "RFID.h"
#include "TimerOne.h"
#include <EEPROM.h>

#include "c.h" 

#define ledPin 4
#define buzzerPin 5
#define doorPin 6



// Creates an RFID instance in Wiegand Mode
// DATA0 of the RFID Reader must be connected 
// to Pin 2 of your Arduino (INT0 on most boards, INT1 on Leonardo)
// DATA1 of the RFID Reader must be connected
// to Pin 3 of your Arduino (INT1 on most boards, INT0 on Leonrado)
RFID rfid(RFID_WIEGAND, W26BIT);
// Declares a struct to hold the data of the RFID tag
// Available fields:
//  * id (3 Bytes) - card code
//  * valid - validity

RFIDTag tag;
long MASTER_ADD = 9038195;
long MASTER_DEL = 9033777;
long storedCard;
boolean programMode = false;
int countDown = -1;

boolean doorOpen = false;

char strMsg[80];
			

void setup() 
{
  countDown = -1;
  
  pinMode(doorPin, OUTPUT);
  digitalWrite(doorPin,LOW); // start with door close
  
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin,HIGH); // Set LED normal color
  
  pinMode(buzzerPin, OUTPUT);
  digitalWrite(buzzerPin,HIGH); // Set buzzer to off
  

  
  Serial.begin(9600);  // Initializes serial port
  // Waits for serial port to connect. Needed for Leonardo only
  while ( !Serial ) ;
   
   Serial.print("Current lenght = ");
   Serial.println(sizeof(cardInfo)/4);
 Serial.print("OLD EE PROM lenght = ");
   Serial.println(EEPROMReadLong(0) );
   
  { EEPROMWriteLong(0,0); }
   
    Serial.print("New EE PROM lenght = ");
    Serial.println(EEPROMReadLong(0) );
   
}

void callback()
{
  digitalWrite(ledPin, digitalRead(ledPin) ^ 1);
  if (countDown == 0) { 
     Timer1.stop();
   digitalWrite(ledPin,HIGH); // Set LED normal color  
  }
     
  if (countDown > 0) countDown--;
}



void loop()
{
  if( rfid.available() )  // Checks if there is available an RFID tag
  {
    tag = rfid.getTag();  // Retrieves the information of the tag
    Serial.print("CC = ");  // and prints that info on the serial port
    Serial.println(tag.id, DEC);
    /*        
    Serial.print("The ID is ");
    if (tag.valid) Serial.println("valid");
    else Serial.println("invalid");
    */
 if (programMode) 
    {
      if ( tag.id != MASTER_ADD)
      {
        writeID(tag.id);
        sprintf(strMsg, "WARN|%ld|New Key Added|%ld", millis(),tag.id);
        Serial.println(strMsg);
        programMode = false;
        Timer1.stop();
        digitalWrite(ledPin,HIGH); // Set LED normal color

      }
    } 
    else if ( tag.id == MASTER_ADD)
    {
      programMode = true;
       sprintf(strMsg, "WARN|%ld|Master Add Used|%ld", millis(),tag.id);
        Serial.println(strMsg);
      Serial.println("Detected MASTER ADD key");
      Serial.println("Swipe key you wish to add...");
      countDown = -1;
      Timer1.initialize(500000);    
      Timer1.attachInterrupt(callback); 
    } 

else { 
	doorOpen = false;
	doorOpen = findID( tag.id ) ;

	if (doorOpen == false) 
	   doorOpen = findIDEEPROM( tag.id ) ;


		if ( doorOpen )
			{
			  //Serial.println("Detected VALID key");
			  Serial.println("Open Door"); 
			   sprintf(strMsg, "INFO|%ld|Valid Key Used|%ld", millis(),tag.id);
        Serial.println(strMsg);

			  openDoor(2);
			   Serial.println("Open Close"); 
			} 
			else 
			{ 
			  //Serial.println("ID not valid"); 
			  sprintf(strMsg, "ERR|%ld|Invalid Key Used|%ld", millis(),tag.id);
              Serial.println(strMsg);
			  countDown = 10;
			   Timer1.initialize(500000/4);    
			  Timer1.attachInterrupt(callback); 
	  
			}
		}
  }
  delay(500);
}

// Write an array to the EEPROM in the next available slot
void writeID( long tagId )
{
  if ( !findID( tagId ) )          // Before we write to the EEPROM, check to see if we have seen this card before!
  {
    long num = EEPROMReadLong(0);  // Get the numer of used spaces, position 0 stores the number of ID cards
//     Serial.print("Number of slots");
//     Serial.println(num);
    num++;                         // Increment the counter by one
    EEPROMWriteLong( 0, num );        // Write the new count to the counter
    EEPROMWriteLong( num, tagId );
    //successWrite();
  }
  else
  {
    //failedWrite();
  }
}

// Looks in the EEPROM to try to match any of the EEPROM ID's with the passed ID
boolean findIDEEPROM( long tagId )
{
  long count = EEPROMReadLong(0);           // Read the first Byte of EEPROM that
  //Serial.print("Count: ");                // stores the number of ID's in EEPROM
  //Serial.print(count);
  //Serial.print("\n");
  for ( long i = 1; i <= count; i++ )      // Loop once for each EEPROM entry
  {
    long readId = EEPROMReadLong(i);      // Read an ID from EEPROM, it is stored in storedCard[6]
    if( readId == tagId )    // Check to see if the storedCard read from EEPROM 
    {                                     // is the same as the find[] ID card passed
      //Serial.print("We have a matched card!!! \n");
       sprintf(strMsg, "WARN|%ld|Temp Key used|%ld", millis(),tagId);
        Serial.println(strMsg);
      return true;
      break;                              // Stop looking we found it
    }
    else                                  // If not, return false
    {
      //Serial.print("No Match here.... \n");
    }
  }
  return false;
}

boolean findID( long tagId )
{
long readId;
for (int idx=0;idx<sizeof(cardInfo)/4;idx++)
  {
    readId = pgm_read_dword(&cardInfo[idx]);
    if( readId == tagId ) 
    {
     return true;
    }
   // printf("%i = %l \r",idx,myvalue);
    
  }
return false;
}

// // Looks in the EEPROM to try to match any of the EEPROM ID's with the passed ID
// boolean findID( long tagId )
// {
//   long count = EEPROMReadLong(0);           // Read the first Byte of EEPROM that
//   //Serial.print("Count: ");                // stores the number of ID's in EEPROM
//   //Serial.print(count);
//   //Serial.print("\n");
//   for ( long i = 1; i <= count; i++ )      // Loop once for each EEPROM entry
//   {
//     long readId = EEPROMReadLong(i);      // Read an ID from EEPROM, it is stored in storedCard[6]
//     if( readId == tagId )    // Check to see if the storedCard read from EEPROM 
//     {                                     // is the same as the find[] ID card passed
//       //Serial.print("We have a matched card!!! \n");
//       return true;
//       break;                              // Stop looking we found it
//     }
//     else                                  // If not, return false
//     {
//       //Serial.print("No Match here.... \n");
//     }
//   }
//   return false;
// }

// Write an array to the EEPROM in the next available slot
// void writeID( long tagId )
// {
//   if ( !findID( tagId ) )          // Before we write to the EEPROM, check to see if we have seen this card before!
//   {
//     long num = EEPROMReadLong(0);  // Get the numer of used spaces, position 0 stores the number of ID cards
//     Serial.print("Number of slots");
//     Serial.println(num);
//     num++;                         // Increment the counter by one
//     EEPROMWriteLong( 0, num );        // Write the new count to the counter
//     EEPROMWriteLong( num, tagId );
//     //successWrite();
//   }
//   else
//   {
//     //failedWrite();
//   }
// }

void openDoor(int setDelay)
{
         digitalWrite(ledPin,LOW); // Set LED Opposite color
         // digitalWrite(buzzerPin,LOW); /// it is too noisey

  setDelay *=1000; // Sets delay in seconds
  digitalWrite(doorPin, HIGH); // Unlock door
  delay(setDelay);
  digitalWrite(doorPin,LOW); // Relock door
         digitalWrite(ledPin,HIGH); // Set LED normal color
          digitalWrite(buzzerPin,HIGH); 

}





long EEPROMReadLong( long address )  // Number = position in EEPROM to get the 5 Bytes from 
{
  long start = (address * 4 );  // Figure out starting position
  long four = EEPROM.read(start);
  long three = EEPROM.read(start+1);
  long two = EEPROM.read(start+2);
  long one = EEPROM.read(start+3);
  return ((four<<0)&0xFF) + ((three<<8)&0xFFFF) + ((two<<16)&0xFFFFFF) + ((one<<24)&0xFFFFFFFF);
}

void EEPROMWriteLong(long address, long value)
{
  long start = (address * 4 );  // Figure out starting position
  //Decomposition from a long to 4 bytes by using bitshift.
  //One = Most significant -> Four = Least significant byte
  byte four = (value & 0xFF);
  byte three = ((value >> 8) & 0xFF);
  byte two = ((value >> 16) & 0xFF);
  byte one = ((value >> 24) & 0xFF);
  
  //Write the 4 bytes into the eeprom memory.
  EEPROM.write(start, four);
  EEPROM.write(start + 1, three);
  EEPROM.write(start + 2, two);
  EEPROM.write(start + 3, one);
  Serial.println(four);
  Serial.println(three);
  Serial.println(two);
  Serial.println(one);
  
}


