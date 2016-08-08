typedef struct env_data {
    float temp;
    float humidity;
} EnvData;

EnvData read_env(int dht_pin);
int cleanup(int dht_pin, int temp_pin, int humidity_pin);
