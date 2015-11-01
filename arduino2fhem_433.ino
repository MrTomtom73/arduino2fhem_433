// This demo does web requests via DNS lookup, using a fixed gateway.
// 2010-11-27 <jc@wippler.nl> http://opensource.org/licenses/mit-license.php

#include <EtherCard.h>
#include <RCSwitch.h>

#define REQUEST_RATE 5000 // milliseconds

// ethernet interface mac address
static byte mymac[] = { 0x74,0x69,0x69,0x2D,0x30,0x31 };
// ethernet interface ip address
static byte myip[] = { 10,195,1,230 };
// gateway ip address
static byte gwip[] = { 10,195,1,1 };
// dns ip address
static byte dnsip[] = { 10,195,1,1 };
// remote fhem name
const char website[] PROGMEM = "fhem.fritz.box";
// myswitch
RCSwitch mySwitch = RCSwitch();

byte Ethernet::buffer[300];   // a very small tcp/ip buffer is enough here
static long timer;

// called when the client request is complete
static void my_result_cb (byte status, word off, word len) {
  Serial.print("<<< reply ");
  Serial.print(millis() - timer);
  Serial.println(" ms");
  Serial.println((const char*) Ethernet::buffer + off);
}

void setup () {
  Serial.begin(57600);
  Serial.println("\n[getViaDNS]");

  if (ether.begin(sizeof Ethernet::buffer, mymac, 10) == 0) 
    Serial.println( "Failed to access Ethernet controller");

//  if(!ether.dhcpSetup())
//    Serial.println( "Failed to get IP address");
    
  ether.printIp("IP:   ", ether.myip); // output IP address to Serial
  ether.printIp("GW:   ", ether.gwip); // output gateway address to Serial
//   ether.printIp("Mask: ", ether.mymask); // output netmask to Serial
  ether.printIp("DHCP server: ", ether.dhcpip); // output IP address of the DHCP server

  ether.staticSetup(myip, gwip, dnsip);

  ether.printIp("IP:   ", ether.myip); // output IP address to Serial
  ether.printIp("GW:   ", ether.gwip); // output gateway address to Serial
//   ether.printIp("Mask: ", ether.mymask); // output netmask to Serial
  ether.printIp("DHCP server: ", ether.dhcpip); // output IP address of the DHCP server

  if (!ether.dnsLookup(website))
    Serial.println("DNS failed");
    
  timer = - REQUEST_RATE; // start timing out right away

  mySwitch.enableReceive(1); // Receiver on interrupt 0 => that is pin D3
}

void loop () {
  ether.packetLoop(ether.packetReceive());
  
  if (mySwitch.available()) {
    unsigned long value = mySwitch.getReceivedValue();
    int radix = 10;
    char buffer[40];
    char *sendstring; 
    
    if (value == 0) {
      Serial.print("Unknown encoding");
    } else {
      sendstring = ultoa(value,buffer,radix);
      Serial.println(sendstring);
      //Serial.println("OK 5 " + String(value) + " 1");
      if (millis() > timer + REQUEST_RATE) {
        timer = millis();
        Serial.println("\n>>> REQ");
        ether.hisport = 8083;
        ether.browseUrl(PSTR("/fhem?cmd.XX_messwert=setreading%20XX_messwert%20Messwert%20"), sendstring, website, my_result_cb);
      }
    }
  }
  
  mySwitch.resetAvailable();

}
