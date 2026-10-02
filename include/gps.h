#ifndef GPS_H
#define GPS_H


struct GPS_DATA
{
    double lat;
    double lon;

    uint32_t sat;

    bool fix;

    int hour;
    int minute;
    int second;
};


void GPS_Init();

void GPS_Task(void *pvParameters);

GPS_DATA GPS_Get();



#endif