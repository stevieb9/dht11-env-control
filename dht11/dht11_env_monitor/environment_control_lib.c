/*
 *  dht11.c:
 *  Temperature action system 
 *  
 */

#include <wiringPi.h>
#include "environment_control_lib.h"

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#define MAXTIMINGS  85

EnvData read_env(int dht_pin){
    int dht11_dat[5] = { 0, 0, 0, 0, 0 };
    
    uint8_t laststate = HIGH;
    uint8_t counter = 0;
    uint8_t j = 0, i;
    float f; /* fahrenheit */

    dht11_dat[0] = dht11_dat[1] = dht11_dat[2] = dht11_dat[3] = dht11_dat[4] = 0;
    
    pinMode(dht_pin, OUTPUT);
    digitalWrite(dht_pin, LOW);
    delay(18);
    
    digitalWrite(dht_pin, HIGH);
    delayMicroseconds(40);
    
    pinMode(dht_pin, INPUT);

    for (i = 0; i < MAXTIMINGS; i++){
        counter = 0;
        while (digitalRead( dht_pin) == laststate){
            counter++;
            delayMicroseconds(1);
            if (counter == 255){
                break;
            }
        }
        laststate = digitalRead(dht_pin);

        if (counter == 255)
            break;

        if ( (i >= 4) && (i % 2 == 0)){
            dht11_dat[j / 8] <<= 1;
            if (counter > 16)
                dht11_dat[j / 8] |= 1;
            j++;
        }
    }

    EnvData env_data;

    if ((j >= 40) &&
         (dht11_dat[4] == ((dht11_dat[0] + dht11_dat[1] + dht11_dat[2] + dht11_dat[3]) & 0xFF))){
        f = dht11_dat[2] * 9. / 5. + 32;

        env_data.temp = f;
        env_data.humidity = (float)dht11_dat[0];

        /*
         * printf( "Humidity = %d.%d %% Temperature = %d.%d *C (%.1f *F)\n",
         * dht11_dat[0], dht11_dat[1], dht11_dat[2], dht11_dat[3], f );
         */
    }
    else {
        env_data.temp = 0.0;
        env_data.humidity = 0.0;
    }
    return env_data;
}

int cleanup(int dht_pin, int temp_pin, int humidity_pin){
    digitalWrite(dht_pin, LOW);
    pinMode(dht_pin, INPUT);

    if (temp_pin > -1){
        digitalWrite(temp_pin, LOW);
        pinMode(temp_pin, INPUT);
    }
    if (humidity_pin > -1){
        digitalWrite(humidity_pin, LOW);
        pinMode(humidity_pin, INPUT);
    }

    return(0);
}
