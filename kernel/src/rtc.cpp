/***
 *                                                                                      
 *      ,ad8888ba,    ,ad8888ba,    88888888ba         db         88      888888888888  
 *     d8"'    `"8b  d8"'    `"8b   88      "8b       d88b        88           88       
 *    d8'           d8'        `8b  88      ,8P      d8'`8b       88           88       
 *    88            88          88  88aaaaaa8P'     d8'  `8b      88           88       
 *    88            88          88  88""""""8b,    d8YaaaaY8b     88           88       
 *    Y8,           Y8,        ,8P  88      `8b   d8""""""""8b    88           88       
 *     Y8a.    .a8P  Y8a.    .a8P   88      a8P  d8'        `8b   88           88       
 *      `"Y8888Y"'    `"Y8888Y"'    88888888P"  d8'          `8b  88888888888  88       
 *  Cobalt is a UNIX-like operating system forked from Dennis Wölfing's Dennix operating
 *  system, which can be found at https://github.com/dennis95/dennix. Cobalt is licensed
 *  under the ISC license, which can be found at the file called LICENSE at the root
 *  directory of the project.
 */

#include <cobalt/kernel/clock.h>
#include <cobalt/kernel/portio.h>
#include <cobalt/kernel/rtc.h>

#define CMOS_ADDRESS_PORT 0x70
#define CMOS_DATA_PORT 0x71

#define RTC_STATUSB_24HOUR 2
#define RTC_STATUSB_BINARY_MODE 4

static uint8_t readCmosRegister(uint8_t reg) {
    outb(CMOS_ADDRESS_PORT, reg);
    return inb(CMOS_DATA_PORT);
}

struct RtcData {
    uint8_t seconds;
    uint8_t minutes;
    uint8_t hours;
    uint8_t day;
    uint8_t month;
    uint8_t year;
    uint8_t century;
    uint8_t statusB;
};

static RtcData readRtc() {
    RtcData result;

    result.seconds = readCmosRegister(0x00);
    result.minutes = readCmosRegister(0x02);
    result.hours = readCmosRegister(0x04);
    result.day = readCmosRegister(0x07);
    result.month = readCmosRegister(0x08);
    result.year = readCmosRegister(0x09);
    result.century = readCmosRegister(0x32);
    result.statusB = readCmosRegister(0x0B);

    return result;
}

static bool rtcEquals(const RtcData& data1, const RtcData& data2) {
    return data1.seconds == data2.seconds
            && data1.minutes == data2.minutes
            && data1.hours == data2.hours
            && data1.day == data2.day
            && data1.month == data2.month
            && data1.year == data2.year
            && data1.century == data2.century
            && data1.statusB == data2.statusB;
}

static uint8_t decodeBcd(uint8_t bcd) {
    return bcd / 16 * 10 + (bcd & 0x0F);
}

void Rtc::initialize() {
    // To avoid reading data that was partially updated, we read the RTC twice
    // and check whether the result was the same.
    RtcData data1 = readRtc();
    RtcData data2 = readRtc();

    while (!rtcEquals(data1, data2)) {
        // If an update was in progress, try again.
        data1 = data2;
        data2 = readRtc();
    }

    bool hour12 = !(data1.statusB & RTC_STATUSB_24HOUR);
    bool pm = hour12 && (data1.hours & 0x80);

    data1.hours &= 0x7F;

    if (!(data1.statusB & RTC_STATUSB_BINARY_MODE)) {
        data1.seconds = decodeBcd(data1.seconds);
        data1.minutes = decodeBcd(data1.minutes);
        data1.hours = decodeBcd(data1.hours);
        data1.day = decodeBcd(data1.day);
        data1.month = decodeBcd(data1.month);
        data1.year = decodeBcd(data1.year);
        data1.century = decodeBcd(data1.century);
    }

    if (pm) {
        data1.hours = (data1.hours + 12) % 24;
    }

    struct tm tm;
    tm.tm_sec = data1.seconds;
    tm.tm_min = data1.minutes;
    tm.tm_hour = data1.hours;
    tm.tm_mday = data1.day;
    tm.tm_mon = data1.month - 1;
    tm.tm_year = data1.century * 100 + data1.year - 1900;
    tm.tm_isdst = -1;

    time_t time = timegm(&tm);

    struct timespec ts;
    ts.tv_sec = time;
    ts.tv_nsec = 0;
    Clock::get(CLOCK_REALTIME)->setTime(&ts);
}
