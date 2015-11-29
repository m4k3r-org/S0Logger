//DHCP = 1, static IP = 0
#define DHCP 0

#include <SPI.h>
#include <Ethernet.h>
#include <SD.h>
#include <Wire.h>
#include <RTClib.h>

byte mac[] = {0xDE, 0xAD, 0xBE, 0xAF, 0x00, 0x20};

#if DHCP
IPAddress ip;
#else
// static IP address, gateway and subnet
IPAddress ip(192,168,178,200);
IPAddress gateway(192,168,178,1);
IPAddress subnet(255, 255, 255, 0);
#endif

volatile unsigned long counter[] =
{
  0L, 0L, 0L, 0L, 0L, 0L  
};

volatile unsigned long data[] =
{
  0L, 0L, 0L, 0L, 0L, 0L  
};

EthernetServer server(80);
File index;
DateTime lastRead;

RTC_DS1307 RTC;

void setup() 
{

    pinMode(10, OUTPUT);
    digitalWrite(10, HIGH);
    
    Serial.begin(9600);

    Wire.begin();
    RTC.begin();
    
    if(! RTC.isrunning()) {
        Serial.println("DS1307 not running");
        return;
    }
    Serial.println("DS1307 running");
    DateTime now = RTC.now();    
    Serial.print(now.year(), DEC);
    Serial.print('/');
    Serial.print(now.month(), DEC);
    Serial.print('/');
    Serial.print(now.day(), DEC);
    Serial.print(' ');
    Serial.print(now.hour(), DEC);
    Serial.print(':');
    Serial.print(now.minute(), DEC);
    Serial.print(':');
    Serial.print(now.second(), DEC);
    Serial.println();

    // initialize SD card
    Serial.println("Initializing SD card...");
    if (!SD.begin(4)) {
        Serial.println("ERROR - SD card initialization failed!");
        return;    // init failed
    }
    Serial.println("SUCCESS - SD card initialized.");
    
    // check for index.htm file
    if (!SD.exists("index.htm")) {
        Serial.println("ERROR - Can't find index.html file!");
        return;  // can't find index file
    }
    Serial.println("SUCCESS - Found index.html file.");    
  
    Serial.println("Setup input lines");
    pinMode(2, INPUT_PULLUP);
    pinMode(3, INPUT_PULLUP);
    pinMode(21, INPUT_PULLUP);
    pinMode(20, INPUT_PULLUP);
    pinMode(19, INPUT_PULLUP);
    pinMode(18, INPUT_PULLUP);
    
    Serial.println("Attaching interrupts");
    attachInterrupt(0, isr_int0, LOW);
    attachInterrupt(1, isr_int1, LOW);
    attachInterrupt(2, isr_int2, LOW);
    attachInterrupt(3, isr_int3, LOW);
    attachInterrupt(4, isr_int4, LOW);
    attachInterrupt(5, isr_int5, LOW);
  
#if DHCP
    Serial.println("Trying DHCP");
    if ( ! Ethernet.begin(mac) ) 
    {
        Serial.println("Failed to obtain a DHCP IP, check Link please!");
    }
#else
    Serial.println("Using static IP address");
    Ethernet.begin(mac, ip);
#endif

    Serial.print("local IP address: ");
    Serial.println(Ethernet.localIP());
    
    server.begin();
  
}

void loop() 
{
    DateTime now = RTC.now();
  
    for(int i = 0; i < 5; i++) {
        if(counter[i] > 1000) {
            data[i]++;
            counter[i] = counter[i] - 1000;
        }
    }
  
    if(now.minute() != lastRead.minute()) {
        File dataLog = SD.open("data.tsv", FILE_WRITE);
        if(dataLog) {
            dataLog.print(now.year(), DEC);
            dataLog.print(now.month(), DEC);
            dataLog.print(now.day(), DEC);
            dataLog.print(now.hour(), DEC);
            dataLog.print(now.minute(), DEC);
            dataLog.print("\t");
            dataLog.print(data[0], DEC);
            dataLog.print("\t");
            dataLog.print(data[1], DEC);
            dataLog.print("\t");
            dataLog.print(data[2], DEC);
            dataLog.println();
            dataLog.close();
        } else {
            Serial.println("Error opening data.tsv");
        }
    } else {
        return;
    }
  
    EthernetClient client = server.available();
    if (client) {
        
        client.println("Content-Type: text/html");
        client.println("Connection: keep-alive");
        client.println();
        index = SD.open("index.htm");
        if (index) {
            while(index.available()) {
                client.write(index.read());
            }
            index.close();
        }      
    }  
}

void isr_int0() 
{
    counter[0]++;
}

void isr_int1()
{
    counter[1]++;  
}

void isr_int2()
{
    counter[2]++;
}

void isr_int3()
{
    counter[3]++;
}

void isr_int4()
{
    counter[4]++;
}

void isr_int5()
{
    counter[5]++;
}

