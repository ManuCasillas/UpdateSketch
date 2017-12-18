mosquitto_sub \
-p 8883 \
-h broker.xively.com \
-i 74300b67-90fe-4b73-b88b-e33eebee2f50 \
-d \
-u 113dec73-5ecd-4ada-ac4e-56303f83badd \
-P e75IOJa5VXcmVEZLG9LAFnRnmJMcNoakoEhWe58SdXs= \
-t xi/blue/v1/74300b67-90fe-4b73-b88b-e33eebee2f50/d/113dec73-5ecd-4ada-ac4e-56303f83badd/Temperatura \
--cafile "certificate.crt"


//--------------------------------

//Constraseña Xively = Arduino1Manu
#define Username 113dec73-5ecd-4ada-ac4e-56303f83badd   //$1
#define Password e75IOJa5VXcmVEZLG9LAFnRnmJMcNoakoEhWe58SdXs=  //$2
#define IdCuenta 74300b67-90fe-4b73-b88b-e33eebee2f50    //$3
#define DevideID 113dec73-5ecd-4ada-ac4e-56303f83badd   ////$4
#define Subdomain manu-arduino  //$5
//String path // $6
//String Temperatura, Humedad....... //$7


mosquitto_sub \
-p 8883 \
-h broker.xively.com \
-i $3 \
-d \
-u $4 \
-P $2 \
-t $6 \
--cafile "certificate.crt"
