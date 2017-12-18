#include <SimpleDHT.h>
#include <FileIO.h>
#include <Process.h>
#include <LiquidCrystal.h>

#define RUN_INTERVAL_MILLIS 2000 // Cada 500 milisegundos se leera la temperatura y humedad del sensor del sensor
#define lecturasMaximas 10// 10 lecturas para que el LCD comienze a parpadear


#define rangeMAX  100 //Rango de lecturas extremas
#define rangeMIN  0


//DEFINIMOS DEBUG
//#define DEBUG 1// if we dont want to appears debug messages, just comment this line

//Para determinar los valores maximos y minimos
float highTem = 21.00;
float highHum = 20.00;
float lowTem  = 10.00;
float lowHum  = 10.00;

//Contadores de lecturas maximas y minimas
int nTemMax = 0;
int nTemMin = 0;
int nHumMax = 0;
int nHumMin = 0;

//Contadores de lecturas normalizadas
int normTemMax = 0;
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
//int pin = 7;
//DHT11  dht11(pin);
int pinDHT11 = 7;
SimpleDHT11 dht11;


//Pines donde colocamos LCD LiquidCrystal
const int rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

//Definicion de bonotes del LCD LiquidCrystal
int lcd_key     = 0;
int adc_key_in  = 0;
#define btnRIGHT  0
#define btnUP     1
#define btnDOWN   2
#define btnLEFT   3
#define btnSELECT 4
#define btnNONE   5


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




//Long utilizado para determinar cada cuanto tiempo se quiere escribir
unsigned long lastRun = (unsigned long) - 6000;

void setup()
{

  lcd.begin(16, 2); // start the library
  lcd.setCursor(0, 0);

  // put your setup code here, to run once:
  Bridge.begin();
  Serial.begin(9600);
  FileSystem.begin();

  //Inicializamos Serial
  while (!Serial) { ; }

  //Inicializamos Base de datos sqlite3
  crearSql();

  day = millis();


}


void loop()
{

  if(controlValues()){ //Comprobamos que los valores sean coherentes
      
    unsigned long now = millis();
    if (now - lastRun >= RUN_INTERVAL_MILLIS) {
      lastRun = now;
      tomarTempHum();
    }
  }
   if(day >= RUN_INTERVAL_DAY){
      day = millis();
      horaActual(); // actualizamos la hora
    }


  lcd_key = read_LCD_buttons();
  lcdMaxMin(lcd_key);

}



void horaActual(){

  Process p;
  p.runShellCommand("ntpd -qn -p es.pool.ntp.org");
  
}

void Xively()
{
  Process p;
  p.runShellCommand("cd /root/");

  delay(1000);

  p.runShellCommand("./mosquitoPub.sh");

  Serial.println("-Enviado-");
  
  
  
  
}




/*controlValues : Controla que los Valores maximo y minimos impuestos sean coherentes
                  No puede haber un valor máximo menor que un valor minimo*/
bool controlValues(){
 
  
  if( highTem > lowTem && highHum > lowHum){
      return true;
  }else{
    
      debug("LOS Valores maximos y minimos NO SON COHERENTES\n", (String)__FUNCTION__ , (String) __LINE__ );
      Serial.print("Valores de maximos y minimos no coherentes - ");
      Serial.println("Los valores de los maximos deben de ser mayores a los valores minimos");
      Serial.println("REINICIA EJECUCION");
      
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Valores maximos");
      lcd.setCursor(2,1);
      lcd.print("no coherentes");
      delay(1000);
      lcd.clear();  
    return false;
   }
  }


/* tomarTempHum : Recoge la temperatura y humedad del componente DTH11*/
void tomarTempHum()
{

  int err;
  byte temp = NULL  , hum = NULL;
  int cnt = 0;
   
 
   if ((err = dht11.read(pinDHT11, &temp, &hum, NULL)) != SimpleDHTErrSuccess) {
     
      lcd.setCursor(0,0); 
      lcd.print(" ERROR  SENSOR"); 
      lcd.setCursor(1,1); 
      lcd.print(" NO CONECTADO"); 

      delay(1000);lcd.clear();delay(1000);//Parpadea
      debug(", LECTURAS DE SENSOR ERRONEAS, error: " + (String)err, (String)__FUNCTION__ , (String) __LINE__ );
      
    }else{ 
      //Funciona correctamente

      //Comprobacion de lecturas erroneas
      if (  rangeMAX > temp > rangeMIN  &&  rangeMAX > hum > rangeMIN) {
        escribirSql((float)temp, (float)hum);
      } else {
           debug("LECTURAS DE SENSOR EXTREMAS\n", (String)__FUNCTION__ , (String) __LINE__ );
           lcd.clear();
           lcd.setCursor(0,0);
           lcd.print("LECTURAS SENSOR");
           lcd.setCursor(4,1);
           lcd.print("EXTREMAS");
      }


    }
}


/* crearFichero: Creamos directorio dentro de arduino. 
                  Nos ayudamos del Process para realizar MKDIR*/
void crearFichero()
{

  Serial.println(F("Creando directorio : /tmp/DTH11/sensor.db"));
  Process p2;
  p2.begin("mkdir");
  p2.addParameter("/tmp/DTH11");
  p2.run();


}


/*crearSql :  Creamos la tabla sqlite3 SENSOR. 
              Si estaba creada en el directorio, hacemos DROP TABLE y la volvemos a crear */
void crearSql()
{

  //Creamos fichero
  crearFichero();

  Serial.println(F("Creando sqlite con ruta: /tmp/DTH11/sensor.db"));

  Process p, p1;
  String cmd = "sqlite3 ";
  String param2 = "/tmp/DTH11/sensor.db ";
  String param3 = "'CREATE TABLE sensor(ID INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT, DATE DATETIME NOT NULL, TEMP REAL NOT NULL, HUM REAL NOT NULL);'";
  String param5 = "'DROP TABLE sensor;'";


  p.runShellCommand(cmd  + param2 + param5);//hacemos DROP sobre sql

  //Comprobacion de errores
  while (p.available() > 0) {
    char c = p.read();
    if ( c == '1') {
      Serial.println(F("ERROR DROP"));
      debug("ERROR DROP\n", (String)__FUNCTION__ , (String) __LINE__ );
      break;
    }
  }

  
  delay(2000); //Le damos tiempo a que realize correctamente la ejecution de Process p

  p1.runShellCommand(cmd  + param2 + param3); //Creamos tabla

  //Comprobacion de errores
  while (p1.available() > 0) {
    Serial.println(F("entra"));
    char c = p1.read();
    if ( c == '1') {
      Serial.println(F("ERROR CREATE"));
      debug("ERROR CREATE\n", (String)__FUNCTION__ , (String) __LINE__ );
      break;
    } else {
      Serial.println(F("SQLite creada correctamente"));
      break;
    }
  }

  Serial.flush();

}

/*escribirSql : Almacena en la base de datos sqlite3 una nueva tupla
PARAMETROS:   temp: Temperatura actual 
              hum: Humedad actual*/
void escribirSql(float temp, float hum)
{

  //Para que solo se muestre la primera vez que entra
  if (firstEntry == true) {
    Serial.println(F("\nIntroduciendo datos en la tabla sensor situada en: /tmp/DTH11/sensor.db"));
    firstEntry = false;
  }

  Process p;
  String cmd = "sqlite3 ";
  cmd += "/tmp/DTH11/sensor.db ";
  cmd += "'INSERT INTO sensor (DATE , TEMP , HUM) VALUES (CURRENT_TIMESTAMP," + String(temp) +  " ," + String(hum) + ");'";
  cmd += "; echo $?";

  p.runShellCommand(cmd);//Hacemos insert a la base de datos

  while (p.available() > 0) {
    char c = p.read();
    #ifdef DEBUG
       getDataDebug(temp, hum);
    #else
      getData(); //Muestra la ultima temperatura
    #endif
  }

  Serial.flush();

}

/*getDataDebug : Método alternativo al getData. Creado para cuando este activo la definicion de DEBUG
  PARAMETROS   : Directamente recogidos del sensor
                float debugTemp
                flaot debugHum*/
void getDataDebug(float debugTemp, float debugHum){
  
String b = "  HUM = " + String(debugHum) ;
String a = "TEMP = " + String(debugTemp) ;
  
   imprimirValuesLCD(a, b);
  
   minMaxValues( debugTemp  ,  a  , debugHum  , b );
   normalizarValues(debugTemp  ,  a , debugHum  , b );

  Serial.println("");
  Serial.flush();

  }



/*getData: Consultamos la Temperatura y la Humedad de la ultima tupla de la base de datos*/
void getData() {

  
  Process p, p1;
  String cmd = "sqlite3 ";
  String param1 = "-line ";
  String param2 = "/tmp/DTH11/sensor.db ";
//  String param3 = "'SELECT TEMP FROM sensor WHERE ID  = (SELECT MAX(ID) FROM sensor);';"; //Query para llamar la data desde SQLite
  String param3 = "'SELECT TEMP FROM sensor ORDER BY ID DESC LIMIT 1;';"; //Query para llamar la data desde SQLite
//  String param4 = "'SELECT HUM FROM sensor WHERE ID = (SELECT MAX(ID) FROM sensor);';"; //Query para llamar la data desde SQLite
  String param4 = "'SELECT HUM FROM sensor ORDER BY ID DESC LIMIT 1;';"; //Query para llamar la data desde SQLite
  p.runShellCommand(cmd + param1 + param2 + param3);


  //Recogemos la TEMPERATURA mas reciente de la base de datos sqlite3
  String temp, tempString;
  while (p.available() > 0) {
    char c = p.read();

    if (p.available() != 0) //Eliminamos el ultimo espacio del char, para que se muestre el mensaje correctamente el display
      temp.concat(c);

    int inChar = c;        //Calculamos el String a int, para poder compararlos con las temperaturas maximas y minimas
    if (isDigit(inChar)) {
      tempString += (char)inChar;
    }
  }


  //Recogemos la HUMEDAD mas reciente de la base de datos sqlite3
  p1.runShellCommand(cmd + param1 + param2 + param4);

  String hum, humString;
  while (p1.available() > 0) {
    char c = p1.read();

    if (p1.available() != 0) //Eliminamos el ultimo espacio del char, para que se muestre el mensaje correctamente el display
      hum.concat(c);

    int inChar = c;    //Calculamos el String a int, para poder compararlos con las humedades maximas y minimas
    if (isDigit(inChar)) {
      humString += (char)inChar;
    }

  }

  imprimirValuesLCD(temp, hum);

  //Llamada a métodos, para controlar las temperaturas y humedades maximas y minimas
  minMaxValues( tempString.toFloat()  / 10,  temp , humString.toFloat() / 10, hum);
  normalizarValues(tempString.toFloat() / 10,  temp , humString.toFloat() / 10, hum );



  Serial.println("");
  Serial.flush();

}

/*imprimirValuesLCD : Para mostrar la información por del LCD
 PARAMETROS:        String temp : Mensaje a mostrar con la informacion del temperatura
                    String temp : Mensaje a mostrar con la informacion del temperatura*/
void imprimirValuesLCD( String temp, String hum){
  
  #ifdef DEBUG
     temp = " "+temp;
  #endif
  
    lcd.setCursor(0, 0);
    lcd.print(temp);
    lcd.print((char)223);//Simbolo de degree
    lcd.print("C");
    Serial.print(temp);
    Serial.println("ªC");

    lcd.setCursor(0, 1);
    lcd.print(hum);
    lcd.print("%");
    Serial.print(hum);
    Serial.println("%");

      
  //Parpadeo
  if (normTemHigh == true || normTemLow == true && normHumHigh == true || normHumLow == true){
     delay(2000);
     lcd.setCursor(8, 0);
     lcd.print("       ");
     lcd.setCursor(8, 1);
     lcd.print("      ");
     delay(250);//Parpadea
     
  }else if(normHumHigh == true || normHumLow == true){
    delay(2000);
    lcd.setCursor(8, 1);
    lcd.print("      ");
    delay(250);//Parpadea
  }else if(normTemHigh == true || normTemLow == true){
    delay(2000);
    lcd.setCursor(8, 1);
    lcd.print("      ");
    delay(250);//Parpadea
   }
  
  }


/*minMaxValues: Nos avisa cuando una temperatura extrema aparece durante 10 lecturas seguidas
 PARAMETROS: temp: Para comparar la temperatura actual con la temperatura maxima
              temp2: String utilizado para mostrar la temperatura actual por pantalla DisplayCrystal 
              hum: Para comparar la humedad actual con la humedad maxima
              hum2:String utilizado para mostrar la humedad actual por pantalla DisplayCrystal */
void minMaxValues(float temp, String temp2 , float hum , String hum2)
{


  if (temp >= highTem && normTemHigh == false) {
    nTemMax++;
    Serial.println("---TEMPERATURA MUY ALTA---");

    if (nTemMax == lecturasMaximas ) { //Controlamos que debe haber 10 lecturas minimos 
      nTemMax = 0;
      
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("TEMP MUY ALTA");
      lcd.setCursor(2, 1);
      lcd.print(temp2);
      lcd.print((char)223);//Simbolo de degree
      lcd.print("C");

      delay(3000);
      lcd.clear();
      normTemHigh = true;
    }

  } else {//Para asegurarnos que son 10 valores maximos seguidos
    nTemMax = 0;
  }

  if (temp <= lowTem && normTemLow == false) {
    nTemMin++;
    Serial.println("---TEMPERATURA MUY BAJA---  ");

    if (nTemMin == lecturasMaximas) {
      nTemMin = 0;

      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("TEMP MUY BAJA");
      lcd.setCursor(2, 1);
      lcd.print(temp2);
      lcd.print((char)223);//Simbolo de degree
      lcd.print("C");

      delay(3000);
      lcd.clear();
      normTemLow = true;
    }
  } else { //Para asegurarnos que son 10 valores maximos seguidos
    nTemMin = 0;
  }

  if (hum >= highHum && normHumHigh == false) {
    nHumMax++;
    Serial.println("---HUMEDAD MUY ALTA---");

    if (nHumMax == lecturasMaximas) {
      nHumMax = 0;

      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("HUM MUY ALTA");
      lcd.setCursor(2, 1);
      lcd.print(hum2);
      lcd.print("%");
//      lcd.blink();

      delay(3000);
      lcd.clear();
      normHumHigh = true;
    }

  } else {//Para asegurarnos que son 10 valores maximos seguidos
    nHumMax = 0;
  }

  if (hum <= lowHum && normHumLow == false) {
    
    Serial.println("---HUMEDAD MUY BAJA---");
    nHumMin++;
    
    if (nHumMin == lecturasMaximas) {
      nHumMin = 0;

      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("HUM MUY BAJA");
      lcd.setCursor(2, 1);
      lcd.print(hum2);
      lcd.print("%");
//      lcd.blink();

      delay(3000);
      lcd.clear();
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
void normalizarValues (float temp, String temp2 , float hum , String hum2)
{

  if (temp < highTem && normTemHigh == true) {

    normTemMax++;

    if (normTemMax == lecturasMaximas) {
      normTemMax = 0;

      Serial.println("---TEMPERATURA ALTA NORMALIZADA---");
      lcd.setCursor(0, 0);
      lcd.print("TEMP NORMALIZADA");
      lcd.setCursor(2, 1);
      lcd.print(temp2);
      lcd.print((char)223);//Simbolo de degree
      lcd.print("C");
      //lcd.noBlink();

      delay(3000);
      lcd.clear();
      normTemHigh = false;
    }

  } else {//Para asegurarnos que son 10 valores maximos seguidos
    normTemMax = 0;
  }

  if (temp > lowTem && normTemLow == true) {
    normTemMin++;

    if (normTemMin == lecturasMaximas) {
      normTemMin = 0;

      Serial.println("---TEMPERATURA BAJA NORMALIZADA---");
      lcd.setCursor(0, 0);
      lcd.print("TEM NORMALIZADA");
      lcd.setCursor(2, 1);
      lcd.print(temp2);
      lcd.print((char)223);//Simbolo de degree
      lcd.print("C");
      //lcd.noBlink();

      delay(3000);
      lcd.clear();
      normTemLow = false;
    }
  } else { //Para asegurarnos que son 10 valores maximos seguidos
    normTemMin = 0;
  }

  if (hum < highHum && normHumHigh == true) {
    normHumMax++;

    if (normHumMax == lecturasMaximas) {
       normHumMax = 0;
      // highHum = hum;

      Serial.println("---HUMEDAD ALTA NORMALIZADA---");
      lcd.setCursor(0, 0);
      lcd.print("HUM NORMALIZADA");
      lcd.setCursor(2, 1);
      lcd.print(hum2);
      lcd.print("%");

      delay(3000);
      lcd.clear();
      normHumHigh = false;
    }

  } else { //Para asegurarnos que son 10 valores maximos seguidos
    normHumMax = 0;
  }

  if (hum > lowHum && normHumLow == true) {
    normHumMin++;

    if ( normHumMin == lecturasMaximas) {
      normHumMin = 0;

      Serial.println("---HUMEDAD ALTA NORMALIZADA---");
      lcd.setCursor(0, 0);
      lcd.print("HUM NORMALIZADA");
      lcd.setCursor(2, 1);
      lcd.print(hum2);
      lcd.print("%");
      //lcd.noBlink();

      delay(3000);
      lcd.clear();
      normHumLow = false;

    }
  } else { //Para asegurarnos que son 10 valores maximos seguidos
    normHumMin = 0;
  }

}


/*read_LCD_buttons(): devuelve el identificador del boton pulsado */
int read_LCD_buttons()
{
  adc_key_in = analogRead(0);      // read the value from the sensor

  if (adc_key_in > 1000) return btnNONE; // We make this the 1st option for speed reasons since it will be the most likely result

  if (adc_key_in < 50)   return btnRIGHT;
  if (adc_key_in < 195)  return btnUP;
  if (adc_key_in < 380)  return btnDOWN;
  if (adc_key_in < 555)  return btnLEFT;
  if (adc_key_in < 790)  return btnSELECT;

  return btnNONE;  // when all others fail, return this...
}


/*lcdMAxMin: Método para controlar los bonotes del DisplayCrystal, PARAMETROS: lcd_key : boton que pulsamos*/
void lcdMaxMin(int lcd_key) {


  switch (lcd_key)   // depending on which button was pushed, we perform an action
  {
    
      case btnLEFT:
       {
       lcd.clear();
       lcd.setCursor(5, 0);
       lcd.print("DEBUG");
       #ifdef DEBUG
         lcd.setCursor(3, 1);
         lcd.print("ACTIVADO");
       #else
         lcd.setCursor(2, 1);
         lcd.print("DESACTIVADO");
       #endif
       delay(3000);
        lcd.clear();
       break;
       }
       
      case btnRIGHT:
       {
       lcd.clear();
       lcd.setCursor(0,0);
       lcd.print("RangoMax = ");
       lcd.print(rangeMAX);
       lcd.setCursor(0,1);
       lcd.print("RangoMin = ");
       lcd.print(rangeMIN);
       delay(3500);
       lcd.clear();
       break;

       }
    
    case btnUP:
      {
        lcd.setCursor(0, 0);
        lcd.print("Tem maxima:");
        #ifdef DEBUG
         lcd.print((int)highTem );
       #else
           lcd.print((int)highTem);
       #endif
       
        lcd.print((char)223);//Simbolo de degree
        lcd.print("C");

        lcd.setCursor(0, 1);
        lcd.print("Hum maxima:");
          lcd.print((int)highHum );
        lcd.print("%");

        delay(5000);
        lcd.clear();
        break;
      }
    case btnDOWN:
      {
        lcd.setCursor(0, 0);
        lcd.print("Tem minima:");
        lcd.print((int)lowTem );
        lcd.print((char)223);//Simbolo de degree
        lcd.print("C");

        lcd.setCursor(0, 1);
        lcd.print("Hum minima:");
        lcd.print((int)lowHum);
        lcd.print("%");

        delay(5000);
        lcd.clear();
        break;
      }

      /*case btnSELECT:
        {
        break;
        }
      */
  }

}

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












