/* 
 * TCP Sync Door [Master Unit]
 * Hardware: Arduino Uno + Ethernet shield Enc28J60
 * @author: Vanderlei Mendes
 * Project to solve a problem of my friend Gustavo (Só Portões)
 * 
 * This is the Client part wich is intended to 
 * be used with another Arduino Uno + ENC28J60 as a server
 * 
 * It sends a command to the Master Door. It can close or open depending on the master door status
 * and sends a request to the slave arduino keeping both doors in sync. if Master is opened, the salve opens
 * and vice versa. 
 */

#include <EtherCard.h>

#define commandButton A5
#define commandRelay A4
#define doorSensorLed A3
#define doorSensor A2

int doorClosed;

// ethernet interface mac address, must be unique on the LAN
static byte mymac[] = { 0x74,0x68,0x69,0x2D,0x30,0x31 };

byte Ethernet::buffer[700];

const char website[] PROGMEM = "www.mocky.io";

// called when the client request is complete
static void my_callback (byte status, word off, word len) {
  Serial.println(">>>");
  Ethernet::buffer[off+300] = 0;
  Serial.print((const char*) Ethernet::buffer + off);
  Serial.println("");
}

void setup () {
  
  Serial.begin(57600);
  Serial.println(F("\n[TCP Sync Door System ( Master Unit )]"));

  // Change 'SS' to your Slave Select pin, if you arn't using the default pin
  if (ether.begin(sizeof Ethernet::buffer, mymac, SS) == 0)
    Serial.println(F("Failed to access Ethernet controller"));
  if (!ether.dhcpSetup())
    Serial.println(F("DHCP failed"));

  ether.printIp("IP:  ", ether.myip);
  ether.printIp("Gateway:  ", ether.gwip);
  ether.printIp("DNS server: ", ether.dnsip);


  //uncomment when done
  //byte hisip[] = { 192,168,25,49 };
  //ether.copyIp(ether.hisip, hisip);

//--------------- remove this block when done --------------
#if 1
  // use DNS to resolve the website's IP address
  if (!ether.dnsLookup(website))
    Serial.println("DNS failed");    
#elif 2
  // if website is a string containing an IP address instead of a domain name,
  // then use it directly. Note: the string can not be in PROGMEM.
  char websiteIP[] = "192.168.1.1";
  ether.parseIp(ether.hisip, websiteIP);
#else
  // or provide a numeric IP address instead of a string
  byte hisip[] = { 192,168,1,1 };
  ether.copyIp(ether.hisip, hisip);
#endif
//----------------------------------------------------------


  ether.printIp("Server IP: ", ether.hisip);

  pinMode(commandButton, INPUT);
  pinMode(commandRelay, OUTPUT);
  pinMode(doorSensorLed, OUTPUT);
  pinMode(doorSensor, INPUT);
  digitalWrite(commandRelay, LOW);
  digitalWrite(doorSensorLed, LOW);
}

void loop () {
  boolean buttonPressed = false;
  
  doorClosed = digitalRead(doorSensor);
  while(digitalRead(commandButton) == HIGH){
    buttonPressed = true;        
    digitalWrite(commandRelay,HIGH);           
    delay(1000);        
    digitalWrite(commandRelay, LOW);
  }     

  if(doorClosed == 1){
    digitalWrite(doorSensorLed, HIGH);    
  }
  else{
    digitalWrite(doorSensorLed, LOW);
  }
  
  ether.packetLoop(ether.packetReceive());

  if (buttonPressed) {
    String requestArg = "masterdoor=closing";    
    
    Serial.println("");
    Serial.println("=== Button press detected ===");
    if(doorClosed == true){      
      Serial.println("The closed door is opening now.");
      requestArg.replace("closing", "opening");
    }
    else{
      Serial.println("The opened door is closing now.");
    }
    
    Serial.println("Sending (" + requestArg + ") to the Slave Arduino.");
    
    Serial.println();
    Serial.print("<<< RESPONSE FROM SLAVE ");    
    ether.browseUrl(PSTR("/v2/"), "5e5bfa793000009975f9f305",website, my_callback);
    //ether.browseUrl(PSTR("/?/"), requestArg, website, my_callback);
    buttonPressed = false;
  }
}
