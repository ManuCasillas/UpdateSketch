#include <SimpleDHT.h>
#include <Process.h>
#include <YunClient.h>
#include <ThingerYun.h>

#define RUN_INTERVAL_MILLIS 2000 // Cada 500 milisegundos se leera la temperatura y humedad del sensor del sensor
#define RUN_INTERVAL_DAY    86400000
#define lecturasMaximas 10// 10 lecturas para que el LCD comienze a parpadear

#define USERNAME "Manu"
#define DEVICE_ID "Arduino1"
#define DEVICE_CREDENTIAL "ArduinoManu"
ThingerYun thing(USERNAME, DEVICE_ID, DEVICE_CREDENTIAL);

//DEFINIMOS DEBUG
#define DEBUG 1// if we dont want to appears debug messages, just comment this line


int pinDHT11 = 7;
SimpleDHT11 dht11;

int rangeMAX = 100; //Rango de lecturas extremas
int rangeMIN = 0;

//Para determinar los valores maximos y minimos
float highTem = 15.00; //float
float highHum = 20.00;
float lowTem  = 10.00;
float lowHum  = 10.00;

//Contadores de lecturas maximas y minimas
int nTemMax = 0; //int
int nTemMin = 0;
int nHumMax = 0;
int nHumMin = 0;

//Contadores de lecturas normalizadas
int normTemMax = 0; //int
int normTemMin = 0;
int normHumMax = 0;
int normHumMin = 0;

//Banderas para normalizar cada valor
bool normTemHigh = false;
bool normTemLow = false;
bool normHumHigh = false;
bool normHumLow = false;

//Boolean used in method: escribirSql()
bool firstEntry = true;

//Pin donde colocamos nuestro sensor DTH11
byte temp = 0;
byte hum = 0;

String tempLimit;
//Long utilizado para determinar cada cuanto tiempo se quiere escribir
//unsigned long lastRun = (unsigned long) - 6000;
unsigned int day = 0;

void setup()
{

    // put your setup code here, to run once:
  Bridge.begin();
  Serial.begin(9600);
  

  //Inicializamos Serial
 // while (!Serial) { ; }

  //Inicializamos Base de datos sqlite3
 // crearSql();

//-------------ENDPOINTS THINGER.IO-------------------

thing["Valores actuales"] >> [](pson& out){
      out["Temperatura actual"] = temp;
      out["Humedad actual"] = hum;
};

 /* thing["Temperatura actual"] >> outputValue(temp);
  thing["Humedad actual"] >> outputValue(hum);
*/
  
  thing["Temperatura Máxima"] << inputValue(highTem); // Son asignados de salida, te muestras el valor prefijado y ademas lo puedes cambiar tu 
  thing["Temperatura Minima"] << inputValue(lowTem);
  thing["Humedad Máxima"] << inputValue(highHum);

  //thing["Humedad Minima"] << inputValue(lowHum); 
  
  thing["MsgEndPoint"] >> outputValue(tempLimit); //Para mensajes endPoint
  
day = millis();


}


void loop()
{
   
  thing.handle();
  
  if(highTem > lowTem && highHum > lowHum){ //Comprobamos que los valores sean coherentes
    tomarTempHum();
  }else{
     debug("LOS Valores maximos y minimos NO SON COHERENTES\n", (String)__FUNCTION__ , (String) __LINE__ );
  }
     if(day >= RUN_INTERVAL_DAY){
      day = millis();
      horaActual(); // actualizamos la hora
    }
  

}



void horaActual(){

  Process p;
  p.runShellCommand("ntpd -qn -p es.pool.ntp.org");
  
}


/* tomarTempHum : Recoge la temperatura y humedad del componente DTH11*/
void tomarTempHum()
{

  int err, cont = 0;
    
   if ((err = dht11.read(pinDHT11, &temp, &hum, NULL)) != SimpleDHTErrSuccess) 
      debug(", LECTURAS DE SENSOR ERRONEAS, error: " + (String)err, (String)__FUNCTION__ , (String) __LINE__ );
    else if (  rangeMAX > temp > rangeMIN  &&  rangeMAX > hum > rangeMIN) 
        //escribirSql((float)temp, (float)hum);
        getDataDebug((float)temp, (float)hum);
       else 
           debug("LECTURAS DE SENSOR EXTREMAS\n", (String)__FUNCTION__ , (String) __LINE__ );

}

/*getDataDebug : Método alternativo al getData. Creado para cuando este activo la definicion de DEBUG
  PARAMETROS   : Directamente recogidos del sensor
                float debugTemp
                flaot debugHum*/
void getDataDebug(float debugTemp, float debugHum){
  
   minMaxValues( debugTemp  ,  "TEMP = " + String(debugTemp)   , debugHum  , "  HUM = " + String(debugHum) );
   normalizarValues(debugTemp  ,  "TEMP = " + String(debugTemp)  , debugHum  , "  HUM = " + String(debugHum) );

  }


/*minMaxValues: Nos avisa cuando una temperatura extrema aparece durante 10 lecturas seguidas
 PARAMETROS: temp: Para comparar la temperatura actual con la temperatura maxima
              temp2: String utilizado para mostrar la temperatura actual por pantalla DisplayCrystal 
              hum: Para comparar la humedad actual con la humedad maxima
              hum2:String utilizado para mostrar la humedad actual por pantalla DisplayCrystal */
void minMaxValues(float temp1, String temp2 , float hum1 , String hum2)
{


  if (temp1 >= highTem && normTemHigh == false) {
    nTemMax++;
    //Serial.println(F("---TEMPERATURA MUY ALTA---"));

    if (nTemMax == lecturasMaximas ) { //Controlamos que debe haber 10 lecturas minimos 
      nTemMax = 0;
     tempLimit ="TEMPERATURA MUY ALTA -  Temperatura actual = " + (String)temp1 + "ºC ,Limite: " + (String)highTem + "ºC";
       thing.call_endpoint("high_temp_email", thing["MsgEndPoint"]);
       //msg(1);
      normTemHigh = true;
    }

  } else {//Para asegurarnos que son 10 valores maximos seguidos
    nTemMax = 0;
  }

  if (temp1 < lowTem && normTemLow == false) {
    nTemMin++;
    //Serial.println(F("---TEMPERATURA MUY BAJA---  "));
    

    if (nTemMin == lecturasMaximas) {
      nTemMin = 0;
      
      tempLimit = "TEMPERATURA MUY BAJA -  Temperatura actual = " + (String)temp1 + "ºC ,Limite: " + (String)lowTem + "ºC";
      thing.call_endpoint("high_temp_email", thing["MsgEndPoint"]);
      //msg(1);
      normTemLow = true;
    }
  } else { //Para asegurarnos que son 10 valores maximos seguidos
    nTemMin = 0;
  }

  if (hum1 >= highHum && normHumHigh == false) {
    nHumMax++;
   // Serial.println(F("---HUMEDAD MUY ALTA---"));
    
    if (nHumMax == lecturasMaximas) {
      nHumMax = 0;
      
      tempLimit = "HUMEDAD MUY ALTA - Humedad actual = " + (String)hum1 + "% ,Limite: " + (String)highHum + "%";
      thing.call_endpoint("high_hum_email", thing["MsgEndPoint"]);
      //msg(1);
      normHumHigh = true;
    }

  } else {//Para asegurarnos que son 10 valores maximos seguidos
    nHumMax = 0;
  }

  if (hum1 < lowHum && normHumLow == false) {
    
    //Serial.println(F("---HUMEDAD MUY BAJA---"));
    
    nHumMin++;
    
    if (nHumMin == lecturasMaximas) {
      nHumMin = 0;
      
      tempLimit = "HUMEDAD MUY BAJA - Humedad actual = " + (String)hum1 + "% ,Limite: " + (String)lowHum + "%";
      thing.call_endpoint("high_hum_email", thing["MsgEndPoint"]);
      //msg(1);
      normHumLow = true;

    }
  } else { //Para asegurarnos que son 10 valores maximos seguidos
    nHumMin = 0;
  }
  
}

/*normalizarValues: Nos avisa cuando los valores extremos se hayan normalizado.
*  PARAMETROS: temp: Para comparar la temperatura actual con la temperatura maxima
              temp2: String utilizado para mostrar la temperatura actual por pantalla DisplayCrystal 
              hum: Para comparar la humedad actual con la humedad maxima
              hum2:String utilizado para mostrar la humedad actual por pantalla DisplayCrystal */
void normalizarValues (float temp1, String temp2 , float hum1 , String hum2)
{

  if (temp1 < highTem && normTemHigh == true) {

    normTemMax++;

    if (normTemMax == lecturasMaximas) {
      normTemMax = 0;
      
      tempLimit = "TEMPERATURA NORMALIZADA - Temperatura actual = " + (String)temp1 + "ºC ,Limite: " + (String)highTem + "ºC";
      thing.call_endpoint("normalized_temp", thing["MsgEndPoint"]);
      //msg(2);
      normTemHigh = false;
    }

  } else {//Para asegurarnos que son 10 valores maximos seguidos
    normTemMax = 0;
  }

  if (temp1 > lowTem && normTemLow == true) {
    normTemMin++;

    if (normTemMin == lecturasMaximas) {
      normTemMin = 0;
      
      tempLimit = "TEMPERATURA NORMALIZADA - Temperatura actual = " + (String)temp1 + "ºC ,Limite: " + (String)lowTem + "ºC";
      thing.call_endpoint("normalized_temp", thing["MsgEndPoint"]);
      //msg(2);
      normTemLow = false;
    }
  } else { //Para asegurarnos que son 10 valores maximos seguidos
    normTemMin = 0;
  }

  if (hum1 < highHum && normHumHigh == true) {
    normHumMax++;

    if (normHumMax == lecturasMaximas) {
       normHumMax = 0;
       
      tempLimit = "HUMEDAD NORMALIZADA - Humedad actual = " + (String)hum1 + "% ,Limite: " + (String)highHum + "%";
      thing.call_endpoint("normalized_hum", thing["MsgEndPoint"]);
      //msg(2);
      normHumHigh = false;
    }

  } else { //Para asegurarnos que son 10 valores maximos seguidos
    normHumMax = 0;
  }

  if (hum1 > lowHum && normHumLow == true) {
    normHumMin++;

    if ( normHumMin == lecturasMaximas) {
      normHumMin = 0;
      
      tempLimit = "HUMEDAD NORMALIZADA - Humedad actual = " + (String)hum1 + "% ,Limite: " + (String)lowHum + "%";
      thing.call_endpoint("normalized_hum", thing["MsgEndPoint"]);
      //msg(2);
      normHumLow = false;

    }
  } else { //Para asegurarnos que son 10 valores maximos seguidos
    normHumMin = 0;
  }

}

/*void msg(int op){
  if(op == 1 )  thing.call_endpoint("high_hum_email", thing["MsgEndPoint"]);
  else if(op == 2) thing.call_endpoint("normalized_temp", thing["MsgEndPoint"]);
  
}*/


/*METODO Debug : PARAMETROS=> str: mensaje de debug, Function: funcion de debug, Line: linea de debug */
void debug(String str, String Function, String Line){
  
  #ifdef DEBUG
  Serial.print(millis()); \
   Serial.print(", Method: "); \
   Serial.print(Function); \
   Serial.print(", Line: "); \ 
   Serial.print(Line); \
   Serial.print(", Debug: "); \
   Serial.println(str);
#endif
  
  }













