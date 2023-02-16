#include "Logger.hpp"

Logger& Logger:: getInstance(){
    static Logger logger;
    return logger;
}
void Logger::setLogLevel(int level){
    _logLevel = level;
}
void Logger::log(string msg){
    switch (_logLevel)
    {
    case INFO:
        cout << "[INFO]";
        break;
    case ERROR:
        cout << "[ERROR]";
        break; 
    case DEBUG:
        cout << "[DEBUG]";
        break;
    case FATAL:
        cout << "[FATAL]";
        break;   
    default:
        break;
    }
    cout << TimeStamp::now().toString() << ":" << msg << endl;
}