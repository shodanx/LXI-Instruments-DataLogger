#define ERROR_CREATE_THREAD -11
#define ERROR_JOIN_THREAD   -12
#define SUCCESS        0

#define MAX_CHANNELS   16
#define RESPONSE_LEN   64

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <libconfig.h>
#include <string.h>
#include <time.h>
#include <lxi.h>
#include <pthread.h>
#include <ncurses.h>
#include <malloc.h>
#include <locale.h>
#include <linux/i2c-dev.h>

static int i2c_fd;

typedef struct
{
    int device[MAX_CHANNELS];
    int Timeout[MAX_CHANNELS];
    const char *Read_command[MAX_CHANNELS];
    pthread_t tid[MAX_CHANNELS];
    int screen_timeout;
    const char *csv_dots;
} SettingsDef;

typedef struct
{   int i2c_address;
    int config_word;
    float temperature;
} temp_sensorsDef;

pthread_t tid[MAX_CHANNELS];
uint64_t sample_num = 0;
int term_x,term_y;
char response_massive[MAX_CHANNELS][RESPONSE_LEN];

WINDOW *log_win;
WINDOW *channels_win;
WINDOW *help_win;

void* measurement_thread(void *arg);

