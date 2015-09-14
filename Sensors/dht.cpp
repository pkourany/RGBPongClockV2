#ifdef SENSOR_DHT

void dht_wrapper();

PietteTech_DHT DHT(DHT_PIN, DHT_TYPE, dht_wrapper);

bool dhtStarted = false;
String dhtError = "";
double dhtFahrenheit = 0;
double dhtHumidity = 0;
int dhtMillis = 0;


void dht_wrapper() { DHT.isrCallback(); }


void doDHT22() {
	if(millis()<dhtMillis)
		return;

	//RGB.control(true);
	//RGB.color(255, 255, 0);

		if(dhtStarted)
    		Particle.publish("DHTTIME", "started "+String(millis()), 60);
    	else
    		Particle.publish("DHTTIME", "NOT started", 60);


        // if(!dhtStarted) {
        //     DHT.acquire();
        //     dhtStarted = true;
        // }


        //if(!DHT.acquiring()) {
            //int dhtResult = DHT.getStatus();
            int dhtResult = DHT.acquireAndWait();

            switch (dhtResult) {
                case DHTLIB_OK:
                    dhtError = "";
                    break;
                case DHTLIB_ERROR_CHECKSUM:
                    dhtError = "Checksum";
                    break;
                case DHTLIB_ERROR_ISR_TIMEOUT:
                    dhtError = "ISR timeout";
                    break;
                case DHTLIB_ERROR_RESPONSE_TIMEOUT:
                    dhtError = "Response timeout";
                    break;
                case DHTLIB_ERROR_DATA_TIMEOUT:
                    dhtError = "Data timeout";
                    break;
                case DHTLIB_ERROR_ACQUIRING:
                    dhtError = "Acquiring";
                    break;
                case DHTLIB_ERROR_DELTA:
                    dhtError = "Delta time to small";
                    break;
                case DHTLIB_ERROR_NOTSTARTED:
                    dhtError = "Not started";
                    break;
                default:
                    dhtError = "Unknown";
                    break;
            }

            if(dhtResult==DHTLIB_OK) {
                //dhtTimestamp = Time.now();

                dhtHumidity = DHT.getHumidity();
                dhtFahrenheit = DHT.getFahrenheit();
                //dhtDewPoint = DHT.getDewPoint();

                //String pub = "{\"h\":" + String((float)dhtHumidity) + ",\"f\":" + String((float)dhtFahrenheit) + ",\"d\":" + String((float)dhtDewPoint) + "}";
                //Particle.publish("environmentals", pub, 2);

                String pub = "h:"+String((float)dhtHumidity)+"|g,f:"+String((float)dhtFahrenheit)+"|g";
                Particle.publish("env", pub, 60);
            } else {
            	Particle.publish("dht22", dhtError, 60);
            }
            
            Particle.publish("dht22", "lalalala", 60);

            dhtStarted = false;
        //}

        if(dhtError.length()>0) {
	        String pub = "dhtError:"+dhtError;
        	Particle.publish("env", pub, 60);
        }

        dhtMillis = millis()+2000;

        Particle.process();

        //RGB.control(false);
}

#endif