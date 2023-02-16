#include "TimeStamp.hpp"
#include <time.h>
TimeStamp::TimeStamp():_mircoSeconds(0){}
TimeStamp::TimeStamp(int64_t mircoSeconds):_mircoSeconds(mircoSeconds){}
string TimeStamp::toString() const{
    tm* tmP = localtime(&_mircoSeconds);
    char buf[128] = {0};
    snprintf(buf, 128, "%d-%d-%d  %d:%d:%d", tmP->tm_year+1900, tmP->tm_mon+1, tmP->tm_mday, tmP->tm_hour, tmP->tm_min, tmP->tm_sec);
    return buf;
}
TimeStamp TimeStamp:: now(){
    return TimeStamp(time(NULL));
}