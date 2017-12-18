mosquitto_pub \
-p 8883 \
-h broker.xively.com \
-i 74300b67-90fe-4b73-b88b-e33eebee2f50 \
-d \
-u 113dec73-5ecd-4ada-ac4e-56303f83badd \
-P e75IOJa5VXcmVEZLG9LAFnRnmJMcNoakoEhWe58SdXs= \
-t xi/blue/v1/74300b67-90fe-4b73-b88b-e33eebee2f50/d/113dec73-5ecd-4ada-ac4e-56303f83badd/Temperatura \
-m ",TempoC,24," \
--cafile "certificate.crt"
