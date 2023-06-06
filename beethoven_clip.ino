#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#define STASSID "Redmi Note 9 Pro" //wifi name
#define STAPSK  "vaibhav123"  //wifi password

unsigned int localPort = 8888; 

char packetBuffer[UDP_TX_PACKET_MAX_SIZE + 1]; //buffer to hold incoming packet,

  
const int nButtons = 4;     //initialize no. of buttons
const int calibration=130;  //it works as a kind of delay
boolean noteoff[nButtons];
int counter[nButtons];

//the instument[] array must have the same number of elements
//than the number of buttons, since each note corresponds to a button
//(kyoki har key ki apni apni value hoti h toh jitne no. of buttons lege utne hi muskical key ke no.
//lena h verna error generate ho jayegi )

byte instrument[] = {51,54,55,56}; //grade or key no. instrument ka !!!
byte buttons[] = {5,4,1,2,};  // nodemcu digital pin configure!!!

//numbers of potentiometers actually we already initialize it zero kyoki pd work kr lega virtually
//toh circuit complex krne ki kya jrurt 
const int nPots=0;     // pure data ka jo pot hai usko 0 intialize krna hai verna voh auto sound dega!
int readings[nPots];   
int PreviousReading[nPots];

bool hayip=false;
WiFiUDP Udp;

void setup() {

  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.begin(STASSID, STAPSK);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(500);
  }
  Serial.print("Connected! IP address: ");
  Serial.println(WiFi.localIP());
  Serial.printf("UDP server on port %d\n", localPort);
  Udp.begin(localPort);
  
  for (int i=0;i<nButtons;i++)
  {
   pinMode(buttons[i], INPUT_PULLUP);
  }
}

//ableton use kr rha hu midi output ke liye toh 1 virtually loopmidi ko connect 
//krna pdega uske hisab se kyoki physically device nhi h mere pass !!!

void midi(unsigned char command, unsigned char note,unsigned char vel)
{
    String happens = String(note,DEC);
    happens+= " ";
    happens+= String(vel,DEC);
    happens+= ";";
    happens+= char(10);
    happens+= char(13);    
    Serial.println(happens);
    char buf[happens.length() ];
    happens.toCharArray(buf, happens.length() );
    Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
    Udp.write(buf);
    Udp.endPacket();
}


void loop() {

  if (!hayip)
  {
    int packetSize = Udp.parsePacket();
    if (packetSize) {
      hayip=true;
      Serial.printf("Received packet of size %d from %s:%d\n    (to %s:%d, free heap = %d B)\n",
                    packetSize,
                    Udp.remoteIP().toString().c_str(), Udp.remotePort(),
                    Udp.destinationIP().toString().c_str(), Udp.localPort(),
                    ESP.getFreeHeap());
  
      // read the packet into packetBufffer
      int n = Udp.read(packetBuffer, UDP_TX_PACKET_MAX_SIZE);
      packetBuffer[n] = 0;
      Serial.println("Contents:");
      Serial.println(packetBuffer);
    }
  }
  else
  {

    
  
  for (int i=0; i<nButtons; i++) //the variable i loops through the numbers 2 to 6
  {
      if (digitalRead(buttons[i]) == LOW) // button press
      {
        if (counter[i]==0)//countdowns finished?
        {
          if (noteoff[i]== 1) //is the note off????
          {
            counter[i]=calibration; // countdown ki value
            midi(144,instrument[i],100); //note is sent
            noteoff[i] = 0; // the note is not off (it is on)
          }
        }
        
      }
      else //button not pressed (possible send Note Off)
      {
        if (counter[i]==0) //countdown finished?
        {
          if (noteoff[i] ==0) //Is the note this activated?
          {
            counter[i]=calibration;  //countdown ki value
            midi(144,instrument[i],0); // send note off
            noteoff[i] = 1;  //note is no longer on!!
          }
        }
      }
  }
for (int i=0; i<nButtons;i++)
//countdowns😂😂😂😂
{
if (counter[i]>0) counter[i]--;
}

for (int k=0; k<nPots; k++) //a run is made from k=0 until k is less than nPots
 {
  readings[k] = map(analogRead(k),0,1023,0,255); //we map the range to twice the CC
 }

for (int k=0; k<nPots;k++)
{   //we see if there was a change in the value of the potentiometers
    if (readings[k] > (PreviousReading[k]+1) || readings[k] < (PreviousReading[k]-1) )
      {                   
          midi(176,k+30,readings[k]/2); //we send from cc 30
          PreviousReading[k] = readings[k];
      } 
}

}


}
