#ifndef MODEM_DATA_H__
#define MODEM_DATA_H__

#include <gps.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

int request_battery_status();

int modem_time_get();

time_t get_current_time();

void set_current_time(struct gps_data gps_data);

#ifdef __cplusplus
}
#endif

#endif