#define ERROR_CREATE_THREAD -11
#define ERROR_JOIN_THREAD   -12
#define SUCCESS               0

#define MAX_CHANNELS       16
#define MAX_SUB_CHANNELS   10
#define RESPONSE_LEN       64
#define SEND_LEN          256
#define REFRESH_SCREEN_TIMEOUT_MS 100

#include <sys/ioctl.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>
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
#include <linux/i2c.h>
#include <unistd.h>


static int i2c_fd;

typedef struct
{
  int device[MAX_CHANNELS];
  int Timeout[MAX_CHANNELS];
  const char *Device_name[MAX_CHANNELS][MAX_SUB_CHANNELS];
  const char *IP[MAX_CHANNELS];
  const char *Read_command[MAX_CHANNELS][MAX_SUB_CHANNELS];
  const char *Display_on_command[MAX_CHANNELS];
  const char *Display_off_command[MAX_CHANNELS];
  const char *Exit_command[MAX_CHANNELS];
  const char *Instance[MAX_CHANNELS];
  const char *Init_commands[MAX_CHANNELS][255];
  int Protocol[MAX_CHANNELS];
  int Port[MAX_CHANNELS];
  int sub_channels_count[MAX_CHANNELS];
  pthread_t tid[MAX_CHANNELS];
} ChannelsDef;


typedef struct
{
  int lxi_connect_timeout;
  const char *csv_dots;
  const char *csv_delimeter;
  int syncfs;
  int display_state;
} SettingsDef;

typedef struct
{
  int i2c_address;
  int config_word;
  double delay;
  double tmp117_last_read_time;
  float temperature;
  const char *device_temp_name;
} temp_sensorsDef;

uint64_t sample_num = 0;
int term_x, term_y;
char response_massive[MAX_CHANNELS][MAX_SUB_CHANNELS][RESPONSE_LEN];

WINDOW *log_win;
WINDOW *channels_win;
WINDOW *help_win;
WINDOW *legend_win;

FILE *csv_file_descriptor, *js_file_descriptor;

SettingsDef Settings;
ChannelsDef Channels;
temp_sensorsDef temperature_sensors[4];
const char *i2c_file_name;

const char *Arroyo_5305_TECSource_port;
struct termios tty;
char tec_massive[2][RESPONSE_LEN];
double tec_last_read_time;
pthread_t p_tec_read_thread;

pthread_t p_window_refresh_thread;

struct timespec start, stop, start_i2c, stop_i2c, start_tec, stop_tec, display_start, display_stop;
double temperature_last_read_time;

time_t rawtime;
struct tm *timeinfo;
char time_buffer[80];
int tspan_count = 0;
char lxi_command_to_send[SEND_LEN];
int channel_count = 0;
config_t cfg;
config_setting_t *setting, *init_commands, *setting_temp, *settings_sub_channels;
int exit_code = 0;
int channel_count_temp = 0;
int total_channels_count = 0;
int total_temp_count = 0;
pthread_t p_read_temp_thread;


void *measurement_thread(void *arg);
void send_command_to_instrument(int chan, const char *arg);
void i2c_write(char i2c_dev_addr, char register_pointer, char data_MSB, char data_LSB);
int16_t i2c_read_temp(char i2c_dev_addr, __u8 addr);
void configure_tmp117(int addr, int config);
void read_temp(int chan, int addr);
void init_config();
void draw_info_win();
