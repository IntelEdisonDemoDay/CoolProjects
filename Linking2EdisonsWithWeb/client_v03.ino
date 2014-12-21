//#########################################################################
//######################## Web Server Local   #############################
//#########################################################################
/*

Choosen project for the Intel Edison demo day on the 18 Nov 2014 in London
was to build a client server system when a button on the server would 
trigger an LED on the client(s).

This is the client code which is based on the WiFiWebClientRep example code.

The code on the client reads the servers web page and toggles the LED on and off.

The orginal WebServer code outputted a HTML page with multiple sensor readings.
Because of the shortage of time to get it all working (a few hours) this was chopped
back to having a single "0" or "1" being available in the web page.

The idea was to have the server and client on different sides of the room (but 
connected to the same WiFi Access Point
 
 Circuit:
 * Wifi shield attached to pins 10, 11, 12, 13
*/

//################# LIBRARIES ################

#include <SPI.h>
#include <WiFi.h>

//char ssid[] = "yourNetwork"; //  your network SSID (name) 
//char pass[] = "secretPassword";    // your network password (use for WPA, or use as key for WEP)
char ssid[] = "The Ballroom";      //  your network SSID (name) 
char pass[] = "239events";   // your network password
int keyIndex = 0;            // your network key Index number (needed only for WEP)

//################ PINS USED ######################
int led = 3;	// LED in D3

char c[] = "0";
int LLEEDD = 0;

int status = WL_IDLE_STATUS;

// Initialize the Wifi client library
WiFiClient client;

// server address:
char server[] = "192.168.12.134";	// ############ to be updated from the IP address allocated to the SERVER Edison ############
//IPAddress server(64,131,82,241);

unsigned long lastConnectionTime = 0;           // last time you connected to the server, in milliseconds
boolean lastConnected = false;                  // state of the connection last time through the main loop
const unsigned long postingInterval = 500;  // delay between updates, in milliseconds

void setup() {
  
  pinMode(led, OUTPUT);  
  //Initialize serial and wait for port to open:
  Serial.begin(9600); 
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }
  
  // check for the presence of the shield:
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WiFi shield not present"); 
    // don't continue:
    while(true);
  } 

  String fv = WiFi.firmwareVersion();
  if( fv != "1.1.0" )
    Serial.println("Please upgrade the firmware");
  
  // attempt to connect to Wifi network:
  while ( status != WL_CONNECTED) { 
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:    
    status = WiFi.begin(ssid, pass);

    // wait 10 seconds for connection:
    delay(10000);
  } 
  // you're connected now, so print out the status:
  printWifiStatus();
}

void loop() {
  // if there's incoming data from the net connection.
  // send it out the serial port.  This is for debugging
  // purposes only:
  while (client.available()) {
    char c = client.read();	// read the single character LED state provided by the server
    Serial.write(c);
    if (c=='1')
    {
      Serial.println("LED on");
        digitalWrite(led, HIGH);   // turn the LED on (HIGH is the voltage level)
    }
    else
    {
      Serial.println("LED off");
        digitalWrite(led, LOW);    // turn the LED off by making the voltage LOW
    }
        c = client.read();	// read the end of line symbol on the webpage and discard
    Serial.write(c);
        c = client.read();	// read the carriage return symbol on the webpage and discard
    Serial.write(c);
    
    client.stop();
  }

  // if there's no net connection, but there was one last time
  // through the loop, then stop the client:
  if (!client.connected()/* && lastConnected*/) {
    Serial.println();
  //  Serial.println("disconnecting."); // used for debugging
    client.stop();
  }

  // if you're not connected, and ten seconds have passed since
  // your last connection, then connect again and send data:
  if(!client.connected() && (millis() - lastConnectionTime > postingInterval)) {
    httpRequest();
  }
  // store the state of the connection for next time through
  // the loop:
  lastConnected = client.connected();
}

// this method makes a HTTP connection to the server:
void httpRequest() {
  // if there's a successful connection:
  if (client.connect(server, 8080)) {
    Serial.println("connecting...");
    // send the HTTP PUT request:
    client.println("GET /index.html HTTP/1.1");
    
//    client.println("Host: www.arduino.cc");
//    client.println("User-Agent: arduino-ethernet");
    client.println("Connection: close");
    client.println();

    // note the time that the connection was made:
    lastConnectionTime = millis();
  } 
  else {
    // if you couldn't make a connection:
    Serial.println("connection failed");
    Serial.println("disconnecting.");
    client.stop();
  }
}


void printWifiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}





