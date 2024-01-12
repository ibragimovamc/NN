#include <iostream>
#include <chrono>
#include <ctime>
#include <cstdlib>

#if !defined (WIN32)
#	include <unistd.h>
#	include <time.h>
#endif

#include "my_serial.hpp"
#include "repository.hpp"
#include "help_utils.hpp"

#define LOG_FILE_ALL "log"
#define LOG_FILE_HOURS "log_h"
#define LOG_FILE_DAYS "log_d"
#define LOG_FILE_MONTH "log_m"
#define HOUR 1
#define HOURS_IN_DAY (HOUR * 24)
#define HOURS_IN_MONTH (HOURS_IN_DAY * 30)

int getTemperatureFromPort() {
    srand(static_cast<unsigned int>(time(nullptr)));
    int random_temperature = rand() % 21;

    return random_temperature;
}

int main(int argc, char **argv) {

    if (argc < 3) {
        std::cout << "Usage: sertest [port] [regime], where [regime] can be 'read' or 'write'" << std::endl;
        return -1;
    }

    bool read = true;

    if (!strcmp(argv[2], "write"))
        read = false;

    cplib::SerialPort smport(std::string(argv[1]), cplib::SerialPort::BAUDRATE_115200);

    if (!smport.IsOpen()) {
        std::cout << "Failed to open port '" << argv[1] << "'! Terminating..." << std::endl;
        return -2;
    }

    /* Класс работающий с лог-файлами. */
    Repository repository;
    repository.setTableFrom(LOG_FILE_ALL);

    auto last_updated_h = std::chrono::system_clock::now();
    auto last_updated_d = std::chrono::system_clock::now();
    auto last_updated_m = std::chrono::system_clock::now();

    std::string temperature_from_port;
    if (read) {
        smport.SetTimeout(1.0);
        for (;;) {
            auto now_h = std::chrono::system_clock::now();
            auto now_d = std::chrono::system_clock::now();
            auto now_m = std::chrono::system_clock::now();

            smport >> temperature_from_port;

            std::cout << "Got temperature: " << (temperature_from_port.empty() ? "nothing" : temperature_from_port)
                      << std::endl;

            if (temperature_from_port.empty()) {
                continue;
            }

            repository.saveTo(LOG_FILE_ALL, temperature_from_port);

            int average_temperature = repository.getAverageTemperatureByTimeInterval(last_updated_d, now_d);
            repository.saveTo(LOG_FILE_HOURS, HelpUtils::baseTypeToString(average_temperature));

            /* если прошел час */
            if (now_h - last_updated_h >= std::chrono::hours(HOUR)) {
                int average_temperature = repository.getAverageTemperatureByTimeInterval(last_updated_h,
                                                                                         now_h);
                repository.saveTo(LOG_FILE_HOURS, HelpUtils::baseTypeToString(average_temperature));
                last_updated_h = std::chrono::system_clock::now();
            }
            /* если прошел день */
            if (now_d - last_updated_d >= std::chrono::hours(HOURS_IN_DAY)) {
                int average_temperature = repository.getAverageTemperatureByTimeInterval(last_updated_d, now_d);
                repository.saveTo(LOG_FILE_DAYS, HelpUtils::baseTypeToString(average_temperature));
                last_updated_d = std::chrono::system_clock::now();
            }
            /* если прошел месяц */
            if (now_d - last_updated_m >= std::chrono::hours(HOURS_IN_MONTH)) {
                int average_temperature = repository.getAverageTemperatureByTimeInterval(last_updated_m, now_m);
                repository.saveTo(LOG_FILE_MONTH, HelpUtils::baseTypeToString(average_temperature));
                last_updated_m = std::chrono::system_clock::now();
            }
        }
    } else {
        for (;;) {
            temperature_from_port = HelpUtils::baseTypeToString(getTemperatureFromPort());
            smport << temperature_from_port;
            HelpUtils::sleep(1.0);
        }
    }
    return 0;
}