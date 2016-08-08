use warnings;
use strict;

use Config; 
use Inline C => 'DATA', libs => '-lwiringPi';

my $dht_pin = 4;
my $err_pin = 1; 
my $trigger_pin = 6; 
my $temp_limit = 65.0;

my $ret = read_temp($dht_pin, $err_pin, $trigger_pin, $temp_limit);

print "$ret\n";

cleanup($dht_pin, $err_pin, $trigger_pin);

__DATA__
__C__

#include <wiringPi.h> 

#include <signal.h> 
#include <stdio.h>
#include <stdlib.h> 
#include <stdint.h>

#define MAXTIMINGS 85

float read_temp(int dhtPin, int tempErrPin, int triggerPin, float tempErrLimit){
    int dht11_dat[5] = { 0, 0, 0, 0, 0 };
    
    uint8_t laststate = HIGH;
    uint8_t counter = 0;
    uint8_t j = 0, i;
    float f; /* fahrenheit */

    dht11_dat[0] = dht11_dat[1] = dht11_dat[2] = dht11_dat[3] = dht11_dat[4] = 0;
    
    pinMode(dhtPin, OUTPUT);
    digitalWrite(dhtPin, LOW);
    delay(18);
    
    digitalWrite(dhtPin, HIGH);
    delayMicroseconds(40);
    
    pinMode(dhtPin, INPUT);

    for (i = 0; i < MAXTIMINGS; i++){
        counter = 0;
        while (digitalRead( dhtPin) == laststate){
            counter++;
            delayMicroseconds(1);
            if (counter == 255){
                break;
            }
        }
        laststate = digitalRead(dhtPin);

        if (counter == 255)
            break;

        /* ignore first 3 transitions */
        if ( (i >= 4) && (i % 2 == 0)){
            /* shove each bit into the storage bytes */
            dht11_dat[j / 8] <<= 1;
            if (counter > 16)
                dht11_dat[j / 8] |= 1;
            j++;
        }
    }

    /*
     * check we read 40 bits (8bit x 5 ) + verify checksum in 
     * the last byte
     */
    if ((j >= 40) &&
         (dht11_dat[4] == ((dht11_dat[0] + dht11_dat[1] + dht11_dat[2] + dht11_dat[3]) & 0xFF))){
        f = dht11_dat[2] * 9. / 5. + 32;

        int warn = 0;

        pinMode(tempErrPin, OUTPUT);
        pinMode(triggerPin, OUTPUT);

        if (f > tempErrLimit){
            warn = 1;
            digitalWrite(tempErrPin, HIGH);

            if (triggerPin > -1){
                digitalWrite(triggerPin, HIGH);
            }
        } 
        else {
            digitalWrite(tempErrPin, LOW);
            digitalWrite(triggerPin, LOW);
        }
    
        /*
         * printf( "Humidity = %d.%d %% Temperature = %d.%d *C (%.1f *F)\n",
         * dht11_dat[0], dht11_dat[1], dht11_dat[2], dht11_dat[3], f );
         */

        if (warn > 0){
            return f;
        }
        else {
            return 0.0;
        }
    }
}

int cleanup(int dhtPin, int tempErrPin, int triggerPin){
    digitalWrite(dhtPin, LOW);
    pinMode(dhtPin, INPUT);

    digitalWrite(tempErrPin, LOW);
    pinMode(tempErrPin, INPUT);

    if (triggerPin > -1){
        digitalWrite(triggerPin, LOW);
        pinMode(tempErrPin, INPUT);
    }
    return(0);
}
