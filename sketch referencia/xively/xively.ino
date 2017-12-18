#include <DHT11.h>

// Example testing sketch for various DHT humidity/temperature sensors
// and update to xively server

//#include "DHT.h"
//#include <Serial.h>
#include <Process.h>

//define xively connect info
#define APIKEY        "74300b6790fe4b73b88be33eebee2f50"   // replace your xively api key here 
#define FEEDID        "manu-arduino"; //"feea422fecf64b75bd2c39b0bc5f5d8f"                   // replace your xively feed ID

/*Subdomain: manu-arduino
 * Password: Arduino1Manu
 * •Xively Device ID
Iddevice = 113dec73-5ecd-4ada-ac4e-56303f83badd
•Xively Account ID
idCuenta = 74300b67-90fe-4b73-b88b-e33eebee2f50

Password = e75IOJa5VXcmVEZLG9LAFnRnmJMcNoakoEhWe58SdXs=
username = 113dec73-5ecd-4ada-ac4e-56303f83badd
*/
curl --user "username=113dec73-5ecd-4ada-ac4e-56303f83badd&password=e75IOJa5VXcmVEZLG9LAFnRnmJMcNoakoEhWe58SdXs=" \ 
-X POST\ 
-d  ‘{"/xi/blue/v1/74300b67-90fe-4b73-b88b-e33eebee2f50/d/113dec73-5ecd-4ada-ac4e-56303f83badd/temperatura": [ { "msgText”:“,TempoC,24,” } ] }’ \ 
-v \
https://subdominio.broker.xively.com/messaging/publish/xi/blue/v1/IdCuenta/d/Iddevice/ topic

#curl --user "username=113dec73-5ecd-4ada-ac4e-56303f83badd&password=e75IOJa5VXcmVEZLG9LAFnRnmJMcNoakoEhWe58SdXs=" \ 
-X POST\
-d
‘{"/xi/blue/v1/74300b67-90fe-4b73-b88b-e33eebee2f50/d/113dec73-5ecd-4ada-ac4e-56303f83badd/temperatura": [ { "msgText”:“,TempoC,24,” } ] }’ \ 
-v \ https://subdominio.broker.xively.com/messaging/publish/xi/blue/v1/IdCuenta/d/Iddevice/topic


curl --user "username=113dec73-5ecd-4ada-ac4e-56303f83badd&password=e75IOJa5VXcmVEZLG9LAFnRnmJMcNoakoEhWe58SdXs=" \
-X POST\
-d
‘{"/xi/blue/v1/74300b67-90fe-4b73-b88b-e33eebee2f50/d/13dec73-5ecd-4ada-ac4e-56303f83badd/Temperatura": [ { "msgText”:“,TempoC,24,” } ] }’ \
-v \ https://manu-arduino.broker.xively.com/messaging/publish/xi/blue/v1/74300b67-90fe-4b73-b88b-e33eebee2f50/d/113dec73-5ecd-4ada-ac4e-56303f83badd/Temperatura








//#define DHTPIN A0 // which signal pin the sensor is connected to

// Uncomment whatever type you're using!
//#define DHTTYPE DHT11 // DHT 11
//#define DHTTYPE DHT22 // DHT 22 (AM2302)
//#define DHTTYPE DHT21 // DHT 21 (AM2301)
DHT11  dht11(7);

//define sensor object
//DHT dht(DHTPIN, DHTTYPE);

// set up net client info:
const unsigned long postingInterval = 6000;  //delay between updates to xively.com
unsigned long lastRequest = 0;      // when you last made a request
String dataString = "";

static char tbuffer[6];
static char hbuffer[6];

void setup()
{
    Bridge.begin();
    Serial.begin(9600);

  while (!Serial) {
    ; // wait for Serial port to connect.
  }

  Serial.println("hola");
   // dht.begin();
}

void updateData() {
   Serial.println("ENTRANDO EN UPDATE-DATA");
    // Reading temperature or humidity takes about 250 milliseconds!
    // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
     float hum = 0, temp = 0;
     int err;
    if((err = dht11.read(hum, temp)) == 0)    // Si devuelve 0 es que ha leido bien
          {
             Serial.print(F("Temperature: "));
             Serial.print(temp);
             Serial.println(" *C");
             Serial.print(F(" Humidity: "));
             Serial.print(hum);
             Serial.print(" %\t");
             Serial.println();

             //convert the sensor data from float to string
               dtostrf(temp,5, 2, tbuffer);
               dtostrf(hum,5, 2, hbuffer);


          // convert the readings to a String to send it:
           dataString = "Temperature,";
        
            dataString += tbuffer;
          //add Humidity:
           dataString += "\nHumidity,";
           dataString += hbuffer;

          sendData();
           
          }else{
            Serial.println("Failed to read from DHT");
            Serial.println(err);
          }


          Serial.println("SALIENDO EN UPDATE-DATA");
}

// this method makes a HTTP connection to the server:
void sendData() {
  Serial.println("ENTRANDO EN SEND-DATA");
  // form the string for the API header parameter:
  String apiString = "X-ApiKey: ";
  apiString += APIKEY;

  // form the string for the URL parameter:
  String url = "https://api.xively.com/v2/feeds/"; //https://manu-arduino.app.xively.com
  url += FEEDID;
  url += ".csv";

  // Send the HTTP PUT request

  // Call a command in MS14.
  // Is better to declare the Process here, so when the
  // sendData function finishes the resources are immediately
  // released. Declaring it global works too, BTW.
  Process xively;
  Serial.print("Sending data... ");
  xively.begin("curl");
  xively.addParameter("-k");
  xively.addParameter("--request");
  xively.addParameter("PUT");
  xively.addParameter("--data");
  xively.addParameter(dataString);
  xively.addParameter("--header");
  xively.addParameter(apiString); 
  xively.addParameter(url);
  xively.run();
  Serial.println("done!\n\n");

  // If there's incoming data from the net connection,
  // send it out the Serial:
  while (xively.available()>0) {
    char c = xively.read();
    Serial.write(c);
  }

}

void loop()
{
  // Serial.println("loop");
  // get a timestamp so you can calculate reading and sending intervals:
  long now = millis();

  // if the sending interval has passed since your
  // last connection, then connect again and send data:
  if (now - lastRequest >= postingInterval) {
    updateData();
    //sendData();
   lastRequest = now;
  }
}
