#include "main.h"
#include <features.h>
// ---------------------------------------------------------------------------------------------------
//
//
//
// ---------------------------------------------------------------------------------------------------
void *measurement_thread(void *arg)
{
  extern SettingsDef Settings;
  extern ChannelsDef Channels;
  int myid = (int) ((intptr_t) arg);
  unsigned long i;
  char response[RESPONSE_LEN];


  int counter, start_num;
  if(Channels.sub_channels_count[myid] == 0)
  {
    start_num = 0;
  } else
  {
    start_num = 1;
  }

  for (counter = start_num; counter <= Channels.sub_channels_count[myid]; counter++)
  {
    send_command_to_instrument(myid, Channels.Read_command[myid][counter]);
    lxi_receive(Channels.device[myid], response, sizeof(response), Channels.Timeout[myid]);     // Wait for response

    i = 0;

    while (strchr("\t\n\v\f\r ", response[i]) == NULL)
    {
      if(strchr(".", response[i]) != NULL)
        response[i] = Settings.csv_dots[0];
      response_massive[myid][counter][i] = response[i];
      i++;
    }

    response[i] = '\0';
    response_massive[myid][counter][i] = '\0';
  }

  pthread_exit(NULL);
}

// ---------------------------------------------------------------------------------------------------
//
//
//
// ---------------------------------------------------------------------------------------------------
void send_command_to_instrument(int chan, const char *arg)
{
  extern ChannelsDef Channels;
  char command[SEND_LEN];

  strcpy(command, arg);
  if(Channels.Protocol[chan] == 0)
    strcat(command, "\n");
  lxi_send(Channels.device[chan], command, strlen(command), Channels.Timeout[chan]);    // Send SCPI commnd

  return;
}

// ---------------------------------------------------------------------------------------------------
//
//
//
// ---------------------------------------------------------------------------------------------------
void i2c_write(char i2c_dev_addr, char register_pointer, char data_MSB, char data_LSB)
{
  int ret;

  __u8 data[3] = { register_pointer, data_MSB, data_LSB };
  struct i2c_msg msg[1];
  struct i2c_rdwr_ioctl_data xfer = {
    .msgs = msg,
    .nmsgs = 1,
  };


  if(i2c_dev_addr < 0 || i2c_dev_addr > 255)
  {
    fprintf(stderr, "i2c: Invalid I2C address. \n");
    return;
  }

  ret = ioctl(i2c_fd, I2C_SLAVE_FORCE, i2c_dev_addr);
  if(ret < 0)
  {
    perror("i2c: Failed to set i2c device address");
    return;
  }


  msg[0].addr = i2c_dev_addr;
  msg[0].flags = 0;
  msg[0].buf = data;
  msg[0].len = 3;

  ioctl(i2c_fd, I2C_RDWR, &xfer);

  return;
}

// ---------------------------------------------------------------------------------------------------
//
//
//
// ---------------------------------------------------------------------------------------------------
void *tec_read_thread(void *arg)
{

  int i = 0, j = 0;

  int serial_port = open(Arroyo_5305_TECSource_port, O_RDWR);
  memset(&tty, 0, sizeof tty);

// Read in existing settings, and handle any error
//  wprintw(log_win, "Error %i from tcgetattr: %s\n", errno, strerror(errno));
  if(tcgetattr(serial_port, &tty) != 0)
  {
    wprintw(log_win, "Error %i from tcgetattr: %s\n", errno, strerror(errno));
  }

  tty.c_cflag &= ~PARENB;       // Clear parity bit, disabling parity (most common)
  tty.c_cflag &= ~CSTOPB;       // Clear stop field, only one stop bit used in communication (most common)
  tty.c_cflag |= CS8;           // 8 bits per byte (most common)
  tty.c_cflag &= ~CRTSCTS;      // Disable RTS/CTS hardware flow control (most common)
  tty.c_cflag |= CREAD | CLOCAL;        // Turn on READ & ignore ctrl lines (CLOCAL = 1)

  tty.c_lflag &= ~ICANON;
  tty.c_lflag &= ~ECHO;         // Disable echo
  tty.c_lflag &= ~ECHOE;        // Disable erasure
  tty.c_lflag &= ~ECHONL;       // Disable new-line echo
  tty.c_lflag &= ~ISIG;         // Disable interpretation of INTR, QUIT and SUSP
  tty.c_iflag &= ~(IXON | IXOFF | IXANY);       // Turn off s/w flow ctrl
  tty.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL);  // Disable any special handling of received bytes

  tty.c_oflag &= ~OPOST;        // Prevent special interpretation of output bytes (e.g. newline chars)
  tty.c_oflag &= ~ONLCR;        // Prevent conversion of newline to carriage return/line feed

  tty.c_cc[VTIME] = 1;          // Wait for up to 0.1s (1 deciseconds), returning as soon as any data is received.
  tty.c_cc[VMIN] = 0;

// Set in/out baud rate to be 38400
  cfsetispeed(&tty, B38400);
  cfsetospeed(&tty, B38400);

// Save tty settings, also checking for error
  if(tcsetattr(serial_port, TCSANOW, &tty) != 0)
  {
    wprintw(log_win, "Error %i from tcsetattr: %s\n", errno, strerror(errno));
  }

  char read_buf_t[256];

  memset(&read_buf_t, '\0', sizeof(read_buf_t));

  unsigned char msg_t[] = "TEC:T?\nTEC:ITE?\nLOCAL\n";

  write(serial_port, msg_t, sizeof(msg_t));
  usleep(500000);
  read(serial_port, &read_buf_t, sizeof(read_buf_t));

  if((strlen(read_buf_t) >= 12) || (strlen(read_buf_t) <= 15))  // Если ответ не полный, прервать.
  {

    while (strchr("\t\n\v\f\r ", read_buf_t[i]) == NULL)
    {
      if(strchr(".", read_buf_t[i]) != NULL)
        read_buf_t[i] = Settings.csv_dots[0];
      tec_massive[0][i] = read_buf_t[i];
      i++;
    }
    tec_massive[0][i] = '\0';
    i += 2;
    while (strchr("\t\n\v\f\r ", read_buf_t[i]) == NULL)
    {
      if(strchr(".", read_buf_t[i]) != NULL)
        read_buf_t[i] = Settings.csv_dots[0];
      tec_massive[1][j] = read_buf_t[i];
      i++;
      j++;
    }

    tec_massive[1][j] = '\0';
  }
  close(serial_port);
  pthread_exit(NULL);

}

// ---------------------------------------------------------------------------------------------------
//
//
//
// ---------------------------------------------------------------------------------------------------
int16_t i2c_read_temp(char i2c_dev_addr, __u8 addr)
{
  int ret;
  __u8 data[2] = { 0, 0 };
  struct i2c_msg msg[2];
  struct i2c_rdwr_ioctl_data xfer = {
    .msgs = msg,
    .nmsgs = 2,
  };
  if(i2c_dev_addr < 0 || i2c_dev_addr > 255)
  {
    fprintf(stderr, "i2c: Invalid I2C address. \n");
    return 0;
  }

  ret = ioctl(i2c_fd, I2C_SLAVE_FORCE, i2c_dev_addr);
  if(ret < 0)
  {
    perror("i2c: Failed to set i2c device address");
    return 0;
  }

  msg[0].addr = i2c_dev_addr;
  msg[0].flags = 0;
  msg[0].buf = &addr;
  msg[0].len = 1;
  msg[1].addr = i2c_dev_addr;
  msg[1].flags = I2C_M_RD;
  msg[1].buf = data;
  msg[1].len = 2;
  ioctl(i2c_fd, I2C_RDWR, &xfer);
  return (data[0] << 8) + data[1];
}

// ---------------------------------------------------------------------------------------------------
//
//
//
// ---------------------------------------------------------------------------------------------------
void configure_tmp117(int addr, int config)
{
  char config1 = (config >> 8) & 0xFF;
  char config2 = config & 0xFF;
  i2c_fd = open(i2c_file_name, O_RDWR);
  if(i2c_fd < 0)
  {
    perror("i2c: Failed to open i2c device");
    return;
  }
  i2c_write(addr & 0xFF, 0x01, config1, config2);
  close(i2c_fd);
}

// ---------------------------------------------------------------------------------------------------
//
//
//
// ---------------------------------------------------------------------------------------------------
void read_temp(int chan, int addr)
{
  float result;
  i2c_fd = open(i2c_file_name, O_RDWR);
  if(i2c_fd < 0)
  {
    perror("i2c: Failed to open i2c device");
    return;
  }


  result = i2c_read_temp(addr & 0xFF, 0x00);    // Read temperature register
  close(i2c_fd);
  result *= 0.0078125;
  if(result > -256)
  {
    temperature_sensors[chan].temperature = result;
  }
}

// ---------------------------------------------------------------------------------------------------
//
//
//
// ---------------------------------------------------------------------------------------------------
void init_config()
{
  const char *csv_dir;
  char time_in_char[32];
  time_t tdate = time(NULL);
  char csv_file_name[512];
  int i, k, chan_count = 0;
  config_init(&cfg);            // Initialize Libconfig library
  if(!config_read_file(&cfg, "lxiidl.cfg"))
  {
    fprintf(stderr, "%s:%d - %s\n", config_error_file(&cfg), config_error_line(&cfg), config_error_text(&cfg));
    config_destroy(&cfg);
    exit_code++;
    return;
  }


  if(!config_lookup_string(&cfg, "dev_file", &i2c_file_name))
    i2c_file_name = "/dev/i2c-1";
  // Open CSV File
  if(!config_lookup_string(&cfg, "csv_save_dir", &csv_dir))
    csv_dir = "./";
  // Read Arroyo port
  if(!config_lookup_string(&cfg, "Arroyo_5305_TECSource_port", &Arroyo_5305_TECSource_port))
    Arroyo_5305_TECSource_port = "";
  strftime(time_in_char, sizeof(time_in_char), "%d_%m_%Y_%H:%M:%S", localtime(&tdate));
  csv_file_name[0] = '\0';
  strcat(csv_file_name, csv_dir);
  strcat(csv_file_name, "/");
  strcat(csv_file_name, time_in_char);
  strcat(csv_file_name, ".csv");
  csv_file_descriptor = fopen(csv_file_name, "w");
  if(csv_file_descriptor == NULL)
  {
    printf("\n Error trying to open 'csv' file. %s", csv_file_name);
    exit_code++;
    return;
  }

  strcat(csv_file_name, ".js");
  js_file_descriptor = fopen(csv_file_name, "w");
  if(js_file_descriptor == NULL)
  {
    printf("\n Error trying to open 'csv.js' file. %s", csv_file_name);
    exit_code++;
    return;
  }


  fprintf(js_file_descriptor, "var skip_samples = 1;    // Skip first samples\n");
  fprintf(js_file_descriptor, "var cut_samples = 1666600; // Maximum  samples\n");
  fprintf(js_file_descriptor, "var avgSamples = 64; // Moving average window \n");
  fprintf(js_file_descriptor, "var circle_size =  2; // Bubble size          \n");
  fprintf(js_file_descriptor, "var circle_op = 0.4;  // Bubble transparent   \n");
  fprintf(js_file_descriptor, "var line_op = 1.0;    // Line transparent     \n");
  fprintf(js_file_descriptor, "var axis_tick = 40;   // Tick of Y axis       \n");
  fprintf(js_file_descriptor, "                                              \n");
  fprintf(js_file_descriptor, "var curveArray = [                            \n");
  if(!config_lookup_string(&cfg, "csv_dots", &Settings.csv_dots))
    Settings.csv_dots = ".";
  if(!config_lookup_string(&cfg, "csv_delimeter", &Settings.csv_delimeter))
    Settings.csv_delimeter = ",";
  if(!config_lookup_int(&cfg, "syncfs", &Settings.syncfs))
    Settings.syncfs = 0;
  if(!config_lookup_int(&cfg, "display_state", &Settings.display_state))
    Settings.display_state = 1;
  if(!config_lookup_int(&cfg, "LXI_connect_timeout", &Settings.lxi_connect_timeout))
    Settings.lxi_connect_timeout = 1000;
  fprintf(csv_file_descriptor, "sample%sdate%stime%s", Settings.csv_delimeter, Settings.csv_delimeter, Settings.csv_delimeter);
  // Load tmp117 config -----------------------------------------
  setting_temp = config_lookup(&cfg, "inventory.tmp117_temperature");
  channel_count_temp = config_setting_length(setting_temp);
  if(setting_temp != NULL)
  {
    for (i = 0; i < channel_count_temp; ++i)
    {
      config_setting_t *channels_temp = config_setting_get_elem(setting_temp, i);
      /* Выводим только те записи, если они имеют все нужные поля. */
      if(!(config_setting_lookup_int(channels_temp, "address", &temperature_sensors[i].i2c_address)
           && config_setting_lookup_int(channels_temp, "config", &temperature_sensors[i].config_word)
           && config_setting_lookup_float(channels_temp, "delay", &temperature_sensors[i].delay) && config_setting_lookup_string(channels_temp, "device_name", &temperature_sensors[i].device_temp_name)));
//        continue;
      if(temperature_sensors[i].i2c_address > 0)
      {
        tspan_count++;
        total_temp_count++;
        if(tspan_count > 1)
        {
          fprintf(js_file_descriptor, "    {\"curveTitle\":\"%s\",		\"channel\":\"ch%i\",	\"offset\":0,		\"scale\":100,	\"group\":0,	\"tspan\":0,	\"axis_is_ppm\":0}, \n",
                  temperature_sensors[i].device_temp_name, i + 17);
        } else
          fprintf(js_file_descriptor, "    {\"curveTitle\":\"%s\",		\"channel\":\"ch%i\",	\"offset\":0,		\"scale\":100,	\"group\":0,	\"tspan\":1,	\"axis_is_ppm\":0}, \n",
                  temperature_sensors[i].device_temp_name, i + 17);
      }


    }
    for (i = 0; i < channel_count_temp; ++i)
      if(temperature_sensors[i].i2c_address > 0)
      {
        total_channels_count++;
        configure_tmp117(temperature_sensors[i].i2c_address, temperature_sensors[i].config_word);
        fprintf(csv_file_descriptor, "temp%i%s", i + 1, Settings.csv_delimeter);
      }

  }

  fprintf(csv_file_descriptor, "tect,tecite%s", Settings.csv_delimeter);

  setting = config_lookup(&cfg, "inventory.channels");
  channel_count = config_setting_length(setting);
  // Read settings -----------------------------------------------
  if(setting != NULL)
  {
    for (i = 0; i < channel_count; ++i)
    {
      config_setting_t *channels = config_setting_get_elem(setting, i);
      /* Выводим только те записи, если они имеют все нужные поля. */
      if(!
         (config_setting_lookup_string(channels, "device_name", &Channels.Device_name[i][0]) && config_setting_lookup_string(channels, "IP", &Channels.IP[i])
          && config_setting_lookup_int(channels, "Protocol", &Channels.Protocol[i]) && config_setting_lookup_string(channels, "Instance", &Channels.Instance[i])
          && config_setting_lookup_int(channels, "Port", &Channels.Port[i]) && config_setting_lookup_int(channels, "Timeout", &Channels.Timeout[i])))
        continue;
      if(!config_setting_lookup_string(channels, "Exit_command", &Channels.Exit_command[i]))
        Channels.Exit_command[i] = "";
      if(!config_setting_lookup_string(channels, "Display_on_command", &Channels.Display_on_command[i]))
        Channels.Display_on_command[i] = "";
      if(!config_setting_lookup_string(channels, "Display_off_command", &Channels.Display_off_command[i]))
        Channels.Display_off_command[i] = "";
      // LXI Connect and init devices
      if(Channels.Protocol[i] == 1)
      {
        Channels.device[i] = lxi_connect(Channels.IP[i], Channels.Port[i], Channels.Instance[i], Settings.lxi_connect_timeout, VXI11);  // Try connect to LXI
      } else
      {
        Channels.device[i] = lxi_connect(Channels.IP[i], Channels.Port[i], Channels.Instance[i], Settings.lxi_connect_timeout, RAW);    // Try connect to LXI
      }

//      if(!(Channels.device[i] < 0))
//      {
      settings_sub_channels = config_setting_get_member(channels, "sub_channels");
      k = 0;
      if(settings_sub_channels)
      {
        while (1)
        {

          config_setting_t *settings_sub_channels_entry = config_setting_get_elem(settings_sub_channels, k);
          if(settings_sub_channels_entry == NULL)
            break;
          config_setting_lookup_string(settings_sub_channels_entry, "device_name", &Channels.Device_name[i][k + 1]);
          config_setting_lookup_string(settings_sub_channels_entry, "Read_command", &Channels.Read_command[i][k + 1]);
          Channels.sub_channels_count[i] = k + 1;
          chan_count++;
          fprintf(csv_file_descriptor, "val%i%s", chan_count, Settings.csv_delimeter);
          fprintf(js_file_descriptor, "    {\"curveTitle\":\"%s %s\",		\"channel\":\"ch%i\",	\"offset\":0,		\"scale\":1,	\"group\":0,	\"tspan\":0,	\"axis_is_ppm\":0}, \n",
                  Channels.Device_name[i][0], Channels.Device_name[i][k + 1], chan_count);
          total_channels_count++;
          k++;
        }
      } else
      {
        config_setting_lookup_string(channels, "Read_command", &Channels.Read_command[i][0]);
        Channels.sub_channels_count[i] = 0;
        chan_count++;
        total_channels_count++;
        fprintf(csv_file_descriptor, "val%i%s", chan_count, Settings.csv_delimeter);
        fprintf(js_file_descriptor, "    {\"curveTitle\":\"%s\",		\"channel\":\"ch%i\",	\"offset\":0,		\"scale\":1,	\"group\":0,	\"tspan\":0,	\"axis_is_ppm\":0}, \n",
                Channels.Device_name[i][0], chan_count);
      }
//      }


      if(!(Channels.device[i] < 0))
      {
        init_commands = config_setting_get_member(channels, "Init_string");
        k = 0;
        if(init_commands)
        {
          while (1)
          {
            if(config_setting_get_elem(init_commands, k) == NULL)
            {
              Channels.Init_commands[i][k] = "";
              break;
            }
            Channels.Init_commands[i][k] = config_setting_get_string_elem(init_commands, k);
            k++;
          }
        }
      }
    }
  }

}

// ---------------------------------------------------------------------------------------------------
//
//
//
// ---------------------------------------------------------------------------------------------------
void draw_info_win()
{
  int i, counter, chan_count = 0, start_num;
  wmove(log_win, 0, 0);
  wmove(legend_win, 0, 0);
  wmove(help_win, 0, 0);
  wmove(channels_win, 1, 1);
  wprintw(channels_win, "%-7s %-40s %-15s %-15s", "Channel", "Device", "IP", "Data");
  wprintw(help_win, "  SPACE - pause, q - quit, r - refresh window, d - display ON/OFF  ");
  wprintw(legend_win, "Sample     Time     ");
  for (i = 0; i < channel_count_temp; ++i)
  {
    if(temperature_sensors[i].i2c_address > 0)
    {
      wprintw(legend_win, "Temp%i    ", i);
      wmove(channels_win, i + 2, 1);
      wprintw(channels_win, "T%-6i %-40s", i, temperature_sensors[i].device_temp_name);
      wrefresh(channels_win);
    }
  }

  wprintw(legend_win, "TEC-T  TEC-ITE ");

  for (i = 0; i < channel_count; ++i)
  {

    if(Channels.sub_channels_count[i] == 0)
    {
      start_num = 0;
    } else
    {
      start_num = 1;
    }
    for (counter = start_num; counter <= Channels.sub_channels_count[i]; ++counter)
    {

      chan_count++;
      if(Channels.sub_channels_count[i] == 0)
      {
        wmove(channels_win, total_temp_count + chan_count - 1 + 2 + 1, 1);
        wprintw(channels_win, "%-7i %-40s %-15s", chan_count, Channels.Device_name[i][0], Channels.IP[i]);
      } else
      {
        wmove(channels_win, total_temp_count + chan_count - 1 + 2 + 1, 1);
        wprintw(channels_win, "%-7i %-20s%-20s %-15s", chan_count, Channels.Device_name[i][0], Channels.Device_name[i][counter], Channels.IP[i]);
      }

      if(Channels.device[i] < 0)
      {
        wmove(channels_win, total_temp_count + chan_count - 1 + 2 + 1, 66);
        wprintw(channels_win, "Connection failed!");
        wrefresh(channels_win);

        wprintw(log_win, "Can't connect to %s\n", Channels.Device_name[i][0]);
      }

      wprintw(legend_win, "Channel%i            ", chan_count);
    }
  }

  wrefresh(log_win);
  wrefresh(legend_win);
  wrefresh(channels_win);
  wrefresh(help_win);
  wmove(log_win, 0, 0);
  // -------------------------------------------------------------
}

// ---------------------------------------------------------------------------------------------------
//
//
//
// ---------------------------------------------------------------------------------------------------
int main(int argc, char **argv)
{
  int i, k;
  int status_addr, status;
  struct timespec start, stop, start_tec, stop_tec, display_start, display_stop;
  double accum;
  char time_in_char[32], temp_in_char[32];
  int time_in_char_pos = 0, temp_in_char_pos = 0;
  lxi_init();                   // Initialize LXI library
  init_config();
  // Initialize Ncurses ------------------------------------------
  setlocale(LC_ALL, "");
  initscr();
  cbreak();
  noecho();
  nonl();
  intrflush(stdscr, FALSE);
  keypad(stdscr, TRUE);
  curs_set(0);
  scrollok(stdscr, TRUE);
  nodelay(stdscr, TRUE);
  refresh();
  start_color();
  init_pair(1, COLOR_BLACK, COLOR_GREEN);
  init_pair(2, COLOR_YELLOW, COLOR_BLACK);
  getmaxyx(stdscr, term_y, term_x);
  channels_win = newwin(total_channels_count + 3 + 1, 85, 0, 0);
  wattron(channels_win, COLOR_PAIR(2));
  box(channels_win, 0, 0);
  log_win = newwin(term_y - (total_channels_count + 3) - 1 - 1 - 1, term_x, total_channels_count + 3 + 1 + 1, 0);
  scrollok(log_win, TRUE);
  legend_win = newwin(term_y - (total_channels_count + 3) - 1 - 1 - 1, term_x, total_channels_count + 3 + 1, 0);
  scrollok(legend_win, TRUE);
  help_win = newwin(1, term_x, term_y - 1, 0);
  wattron(help_win, COLOR_PAIR(1));
  wmove(log_win, 0, 0);
  draw_info_win();
  for (i = 0; i < channel_count; ++i)
  {
    if(!(Channels.device[i] < 0))
    {                           // Send init commands to instruments
      k = 0;
      while (strlen(Channels.Init_commands[i][k]) > 0)
      {

        send_command_to_instrument(i, Channels.Init_commands[i][k]);
        wprintw(log_win, "%s send init: %s\n", Channels.Device_name[i][0], Channels.Init_commands[i][k]);
        wrefresh(log_win);
        k++;
      }
    }
  }

  fprintf(csv_file_descriptor, "\n");
  fprintf(js_file_descriptor, "  ];                                          \n");
  fclose(js_file_descriptor);
// Send display ON/OFF commands to instruments
  for (i = 0; i < channel_count; ++i)
    if(Channels.device[i] >= 0)
    {
      if(Settings.display_state == 0)
      {
        send_command_to_instrument(i, Channels.Display_off_command[i]);
        wprintw(log_win, "%s send init: %s\n", Channels.Device_name[i][0], Channels.Display_off_command[i]);
      } else
      {
        send_command_to_instrument(i, Channels.Display_on_command[i]);
        wprintw(log_win, "%s send init: %s\n", Channels.Device_name[i][0], Channels.Display_on_command[i]);
      }
    }
  wrefresh(log_win);
// -------------------------------------------------------------
// Start time measurement
  clock_gettime(CLOCK_REALTIME, &start);
  clock_gettime(CLOCK_REALTIME, &start_tec);
  clock_gettime(CLOCK_REALTIME, &display_start);
  pthread_create(&p_tec_read_thread, NULL, tec_read_thread, (void *) ((intptr_t) i));   // запуск чтения данных с TEC
// Read data ---------------------------------------------------
  while (exit_code == 0)
  {
    switch (getch())
    {
    case 'q':

      for (i = 0; i < channel_count; ++i)
        if(Channels.device[i] >= 0)
        {
          send_command_to_instrument(i, Channels.Exit_command[i]);
        }

      exit_code++;
      break;
    case 'd':

      if(Settings.display_state == 0)
      {
        Settings.display_state = 1;
      } else
      {
        Settings.display_state = 0;
      }

      wprintw(log_win, "\n");
      for (i = 0; i < channel_count; ++i)
        if(Channels.device[i] >= 0)
        {
          // Send display ON/OFF commands to instruments
          if(Settings.display_state == 0)
          {
            send_command_to_instrument(i, Channels.Display_off_command[i]);
            wprintw(log_win, "%s send init: %s\n", Channels.Device_name[i][0], Channels.Display_off_command[i]);
          } else
          {
            send_command_to_instrument(i, Channels.Display_on_command[i]);
            wprintw(log_win, "%s send init: %s\n", Channels.Device_name[i][0], Channels.Display_on_command[i]);
          }
        }
      wrefresh(log_win);
      break;
    case ' ':
      while (getch() != ' ')
        sleep(1);
      break;
    case 'r':
      getmaxyx(stdscr, term_y, term_x);
      wresize(log_win, term_y - (total_channels_count + 3) - 1 - 1, term_x);
      wresize(legend_win, 1, term_x);
      wresize(channels_win, total_channels_count + 3, 85);
      mvwin(help_win, term_y - 1, 0);
      wresize(help_win, 1, term_x);
      draw_info_win();
      box(channels_win, 0, 0);
      wrefresh(help_win);
      wrefresh(log_win);
      wrefresh(legend_win);
      wrefresh(channels_win);
      break;
    }

    if(exit_code != 0)
      break;
    sample_num++;
    for (i = 0; i < channel_count; ++i) // Start threads
    {
      if(Channels.device[i] >= 0)
      {
        pthread_create(&(Channels.tid[i]), NULL, measurement_thread, (void *) ((intptr_t) i));  // запуск LXI измерения
      } else
      {
        int counter;
        for (counter = 0; counter <= Channels.sub_channels_count[i]; counter++)
        {
          response_massive[i][counter][0] = '0';
          response_massive[i][counter][1] = '\0';
        }
        sleep(1);               //запуск невозможен, прибор не подключен.
      }
    }

    wprintw(log_win, "\n");
    // Calculate time
    clock_gettime(CLOCK_REALTIME, &stop);       // Fix clock
    accum = (stop.tv_sec - start.tv_sec) + (stop.tv_nsec - start.tv_nsec) / 1E9;
    for (i = 0; i < channel_count_temp; ++i)
    {
      if(temperature_sensors[i].i2c_address > 0)
        if(((accum - temperature_sensors[i].tmp117_last_read_time) > temperature_sensors[i].delay) || temperature_sensors[i].tmp117_last_read_time == 0)
        {
          temperature_sensors[i].tmp117_last_read_time = accum;
          read_temp(i, temperature_sensors[i].i2c_address);     // Read TMP117

        }
    }

    clock_gettime(CLOCK_REALTIME, &stop_tec);   // Fix clock
    accum = (stop_tec.tv_sec - start_tec.tv_sec) + (stop_tec.tv_nsec - start_tec.tv_nsec) / 1E9;
    if((accum - tec_last_read_time) > 1)        // зедержка 1 сек
    {
      tec_last_read_time = accum;
      status = pthread_join(p_tec_read_thread, (void **) &status_addr);
      if(status == SUCCESS)
      {
        pthread_create(&p_tec_read_thread, NULL, tec_read_thread, (void *) ((intptr_t) i));     // запуск чтения данных с TEC
      }

    }
    // Draw log table and save CSV

    time(&rawtime);
    timeinfo = localtime(&rawtime);
    strftime(time_buffer, 80, "%d/%m/%Y-%H:%M:%S%z", timeinfo);
    wprintw(log_win, "%-5u   ", sample_num);
    fprintf(csv_file_descriptor, "%llu%s%s%s", sample_num, Settings.csv_delimeter, time_buffer, Settings.csv_delimeter);
    sprintf(time_in_char, "%.4f", accum);
    time_in_char_pos = 0;
    while (strchr(",", time_in_char[time_in_char_pos]) == NULL)
      time_in_char_pos++;
    time_in_char[time_in_char_pos] = Settings.csv_dots[0];
    wprintw(log_win, "%9.4f  ", accum);
    fprintf(csv_file_descriptor, "%s%s", time_in_char, Settings.csv_delimeter);
    // Draw temperature
    for (i = 0; i < channel_count_temp; ++i)
    {
      if(temperature_sensors[i].i2c_address > 0)
      {
        sprintf(temp_in_char, "%.3f", temperature_sensors[i].temperature);
        temp_in_char_pos = 0;
        while (strchr(",", temp_in_char[temp_in_char_pos]) == NULL)
          temp_in_char_pos++;
        temp_in_char[temp_in_char_pos] = Settings.csv_dots[0];
        wprintw(log_win, "%7.3lf  ", temperature_sensors[i].temperature);
        fprintf(csv_file_descriptor, "%s%s", temp_in_char, Settings.csv_delimeter);
        mvwprintw(channels_win, i + 2, 66, "%7.3lf", temperature_sensors[i].temperature);       // Print response
      }
    }

    // Draw TEC data

    if(Arroyo_5305_TECSource_port != NULL)
    {
      wprintw(log_win, " %-5s  %-5s   ", tec_massive[0], tec_massive[1]);
      fprintf(csv_file_descriptor, "%s%s%s%s", tec_massive[0], Settings.csv_delimeter, tec_massive[1], Settings.csv_delimeter);
      mvwprintw(channels_win, total_temp_count + 2, 1, "TEC     Arroyo                                   T:%-8s       ITE:%-8s", tec_massive[0], tec_massive[1]);       // Print response
    }
    // Wait threads complete
    for (i = 0; i < channel_count; ++i)
    {
      if(Channels.device[i] >= 0)
      {
        status = pthread_join(Channels.tid[i], (void **) &status_addr);
        if(status != SUCCESS)
        {
          wprintw(log_win, "main error: can't join thread, status = %d\n", status);
          exit(ERROR_JOIN_THREAD);
        }

      }
    }


    // ---------------------------------------------------------


    // Draw measure
    for (i = 0; i < channel_count; ++i)
    {
      int counter, start_num;
      if(Channels.sub_channels_count[i] == 0)
      {
        start_num = 0;
      } else
      {
        start_num = 1;
      }

      for (counter = start_num; counter <= Channels.sub_channels_count[i]; counter++)
      {
        if(Channels.device[i] >= 0)
          mvwprintw(channels_win, total_temp_count + counter - start_num + i + 2 + 1, 66, "%-15s", response_massive[i][counter]);       // Print response
        wprintw(log_win, "%-20s", response_massive[i][counter]);
        fprintf(csv_file_descriptor, "%s%s", response_massive[i][counter], Settings.csv_delimeter);
      }

    }

    // finish
    fprintf(csv_file_descriptor, "\n");
    if(Settings.syncfs == 1)
    {
      fflush(csv_file_descriptor);
      syncfs(fileno(csv_file_descriptor));
    }

    clock_gettime(CLOCK_REALTIME, &display_stop);       // Fix clock
    accum = ((display_stop.tv_sec - display_start.tv_sec) + (display_stop.tv_nsec - display_start.tv_nsec) / 1E9) * 1000;
    if(accum >= REFRESH_SCREEN_TIMEOUT_MS)
    {
      wrefresh(channels_win);
      wrefresh(log_win);
      clock_gettime(CLOCK_REALTIME, &display_start);
    }
    // ---------------------------------------------------------


  }


////////////////////////////////////////////////////////////////////////////////
  for (i = 0; i < channel_count; ++i)   // закрытие потоков и инструментов
  {
    if(Channels.device[i] >= 0)
    {
      printw("Close: i=%i device=%i tid=%i\n", i, Channels.device[i], Channels.tid[i]);
      status = pthread_join(Channels.tid[i], NULL);
      printw("Closed!\n");
      lxi_disconnect(Channels.device[i]);
    }
  }

  fclose(csv_file_descriptor);
  pthread_exit(NULL);
  delwin(channels_win);
  delwin(log_win);
  delwin(help_win);
  endwin();
  config_destroy(&cfg);
  return (EXIT_SUCCESS);
}
