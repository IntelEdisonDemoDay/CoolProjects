//#########################################################################
//######################## Web Server Local   #############################
//#########################################################################
/*

Choosen project for the Intel Edison demo day on the 18 Nov 2014 in London
was to build a client server system when a button on the server would 
trigger an LED on the client(s).

This is the server code which is based on the WebServer example code plus the 
debounce demo code for the button input.  The debounce button code is vital
as otherwise suprious readings from the button might get passed onto the client.

The code on the server makes the button toggle the LED on and off.

The orginal WebServer code outputted a HTML page with multiple sensor readings.
Because of the shortage of time to get it all working (a few hours) this was chopped
back to having a single "0" or "1" being available in the web page.

The idea was to have the server and client of differnet sides of the room (but 
connected to the same WiFi Access Point

The Server code drives the LCD screen to show status info as it goes through its 
setup process and finally shows the DHCPd IP address that the clients need to 
connect to.  The URL is the format:  http://x.x.x.x:8080/index.html
*/

//################# LIBRARIES ################

#include <SPI.h>        //Used to communicate with the ethernet chip
#include <WiFi.h>   //Used to interface with the ethernet portion
#include <Wire.h>
#include <rgb_lcd.h>

//char ssid[] = "yourNetwork"; //  your network SSID (name) 
//char pass[] = "secretPassword";    // your network password (use for WPA, or use as key for WEP)
char ssid[] = "The Ballroom"; //  your network SSID (name) 
char pass[] = "239events";    // your network password (use for WPA, or use as key for WEP)
int keyIndex = 0;            // your network key Index number (needed only for WEP)

int status = WL_IDLE_STATUS;

//################ PINS USED ######################
const int kButtonPin = 4;    // Button in D4
const int kledPin = 3;       // LED in D3

//################ VARIABLES ################

//------ NETWORK VARIABLES---------
//--MAKE CHANGE HERE--MAKE CHANGE HERE--MAKE CHANGE HERE--MAKE CHANGE HERE--MAKE CHANGE HERE--							
//IPAddress ip(***,***,***,***); //A free ip address on your network 
                             //(for more help http://iotm.org/NETW)
int port = 8080;  //Port your server will listen to 
                  //(80 is default for HTTP but the Edison already has one running there)
WiFiServer server(port);

//------ SKETCH VARIABLES -------------

// LCD Settings - background of the LCD colour can be changed here
rgb_lcd lcd;
const int colorR = 254;
const int colorG = 254;
const int colorB = 254;

// BUTTON SETUP
// Variables will change:
int ledState = LOW;         // the current state of the output pin
int buttonState;             // the current reading from the input pin
int lastButtonState = LOW;   // the previous reading from the input pin

// the following variables are long's because the time, measured in miliseconds,
// will quickly become a bigger number than can be stored in an int.
long lastDebounceTime = 0;  // the last time the output pin was toggled
long debounceDelay = 50;    // the debounce time; increase if the output flickers


/*
 * setup() - this function runs once when you turn your Arduino on 
 */
void setup()
{
  lcd.begin(16, 2);
  lcd.setRGB(colorR, colorG, colorB);   // Set the background colour
  lcd.print("Starting...");
  Serial.begin(9600);
  Serial.println(".:Web Server:. -- (local)");
  
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Setting up...");
  setupNetwork();  //sets up the boards WiFi connection
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Networking...");
  
  Serial.println("Small  Web Server Running");
  Serial.print("    Visit http://");
  Serial.print(WiFi.localIP());
  Serial.print(":");
  Serial.print(port);
  Serial.println(" in a web browser to see the result.");  
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Server IP:");
  lcd.setCursor(0, 1);
  lcd.print(WiFi.localIP());  // Display the IP address on the screen

// Setup the button

  pinMode(kButtonPin, INPUT);
  pinMode(kledPin, OUTPUT);

  // set initial LED state
  digitalWrite(kledPin, ledState);

}

/*
 * loop() - this function will start after setup finishes and then repeat
 */
void loop()
{
  takeReadButton();  // check the button state
  checkNetwork();              //checks to see if there has been a web request
}


/*
 * setupNetwork() - Sets up your WiFi connection
 */
void setupNetwork()
{
  Serial.print("    setupNetwork() Started..");
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
  while (status != WL_CONNECTED) { 
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:    
    status = WiFi.begin(ssid, pass);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("WiFi...");
  
    // wait 10 seconds for connection:
    delay(5000);
  } 
  server.begin();
  Serial.println("..Connected to wifi..");
}

/*
 * READ THE BUTTON WITH DEBOUNCE
 */
 
void takeReadButton(){
    // read the state of the switch into a local variable:
  int reading = digitalRead(kButtonPin);

  // check to see if you just pressed the button 
  // (i.e. the input went from LOW to HIGH),  and you've waited 
  // long enough since the last press to ignore any noise:  

  // If the switch changed, due to noise or pressing:
  if (reading != lastButtonState) {
    // reset the debouncing timer
    lastDebounceTime = millis();
  } 
  
  if ((millis() - lastDebounceTime) > debounceDelay) {
    // whatever the reading is at, it's been there for longer
    // than the debounce delay, so take it as the actual current state:

    // if the button state has changed:
    if (reading != buttonState) {
      buttonState = reading;

      // only toggle the LED if the new button state is HIGH
      if (buttonState == HIGH) {
        ledState = !ledState;
      }
    }
  }
  
  // set the LED:
  digitalWrite(kledPin, ledState);

  // save the reading.  Next time through the loop,
  // it'll be the lastButtonState:
  lastButtonState = reading;
}
  

/*
 * checkNetwork() - Checks for a web connection attempt, if 
 * one is found a webpage is served up.
 */
void checkNetwork()
{
  WiFiClient client = server.available();   //listen for incoming clients
  if (client) {                             //If a request has been received
  
    boolean currentLineIsBlank = true; //variable for checking if end of request found.
    
    while (client.connected()) {       //this is a simple implementation which does not
                                       //capture any information from the request, it reads the entire
                                       //request and once the end is found (all http requests end with a blank line)
                                       //this is checked for by finding two new line '\n' in a row      
      if (client.available()) {        //if there is still an http request waiting
      
        char c = client.read();        //read the next charachter of the request
        
        if (c == '\n' && currentLineIsBlank) {  //if the last and current charachter are new lines '\n' the end of the
                                                //http request has been found, so return a web page
          
          Serial.print("      Serving Webpage at: "); //Send request timestamp to Serial Monitor
          Serial.println(millis());

          sendWebpage(client);  // Send the web page in response
          
          break;                                                            //exit this sub routine as the page is finished sending
        }
        
        //only reached if not returning a web page
        
        if (c == '\n') {               //if current charachter is a new line '\n'
          currentLineIsBlank = true;   //set a variable to mark the current charachter
                                       //as a new line, done because if the next charachter is
                                       //a new line the end of the request has been reached and
                                       //a web page needs to be returned
        } 
        else if (c != '\r') {          //if the current charachter isn't a new line
          currentLineIsBlank = false;  //set the variable telling the loop the last charachter wasn't
                                       //a new line
        }
      }
    }
    delay(1);       //give the web browser time to receive the data
    client.stop();  //close the connection:
  }
}

/*
 * sendWebpage(aClient) - Serves the webpage to the given network client
 */ 
void sendWebpage(WiFiClient aClient)
{
//###########################
//####### HTTP HEADER #######   // No HTTP headers
//###########################
//  aClient.println("HTTP/1.1 200 OK");          //send a standard http response header
//  aClient.println("Content-Type: text/html");
//  aClient.println();

//###########################
//#######  WEB PAGE   #######  //to customize change the page details below
//###########################
  aClient.println(ledState);  // Just output 0 or 1 -- no HTML  (would be better in a JSON format)
}

