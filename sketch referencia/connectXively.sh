#!/bin/sh
curl -u "113dec73-5ecd-4ada-ac4e-56303f83badd:e75IOJa5VXcmVEZLG9LAFnRnmJMcNoakoEhWe58SdXs=" -X POST \
-d \
'{"msgText":",TempoC,24,"}]}' \
--cacert ./certificate.crt \
-v  https://manu-arduino.broker.xively.com/messaging/publish/xi/blue/v1/74300b67-90fe-4b73-b88b-e33eebee2f50/d/113dec73-5ecd-4ada-ac4e-56303f83badd/Temperatura
