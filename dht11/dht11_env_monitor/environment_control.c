/*
 *  dht11_env_monitor.c
 *  
 *  daemon binary to monitor for temp/humidity changes, and
 *  command the library to turn on/off environment devices
 *
 */

#include <wiringPi.h>
#include "environment_control_lib.h"
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <unistd.h>

static volatile int keep_running = 1;

void int_handler(int dummy) {
    keep_running = 0;
}

int main(int argc, char *argv[]){
    if (argc == 6 && atoi(argv[5]) == 1){
        daemon(0, 0);
    }

    if (wiringPiSetup() == -1) 
        exit(1);

    if(argc < 5){
        printf("usage: %s <DHT11 pin#> <temp pin#> <humidity pin#> <temp limit> <humidity limit> [bool daemonize]\n", argv[0]);
        exit(0);
    }

    int dht_pin = atoi(argv[1]);
    int temp_pin = atoi(argv[2]); 
    int humidity_pin = atoi(argv[3]);
    float temp_high = atof(argv[4]); 
    float humidity_low = atof(argv[5]); 

    signal(SIGINT, int_handler);

    while (keep_running){
        time_t current_time;
        char* c_time_string;
        current_time = time(NULL);
        c_time_string = ctime(&current_time);
       
        EnvData env_data = read_env(dht_pin);
        
        if (env_data.temp > temp_high){
            printf(
                "temp high: %.1f > %.1f at %s", 
                env_data.temp, temp_high, c_time_string
            );
        }
        if (env_data.humidity > humidity_low){
            printf(
                "humidity low: %.1f < %.1f at %s", 
                humidity_low, env_data.humidity, c_time_string
            );
        }
        delay(3000);
    }

    cleanup(dht_pin, temp_pin, humidity_pin);    

    return(0);
}
