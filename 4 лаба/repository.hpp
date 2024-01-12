#pragma once

#include <sstream>
#include <iostream>
#include <fstream>
#include <chrono>
#include <ctime>
#include <iomanip>
#include "help_utils.hpp"

#if !defined (WIN32)
#	include <unistd.h>
#	include <time.h>
#endif

#include <cstdlib>

#define LOG_SEPARATOR '#'
#define DATETIME_FORMAT "%Y-%m-%d %H:%M:%S"


class Repository {
public:
    /* Задать источник считывания данных. */
    void setFileFrom(const std::string &from_file_name) {
        _from_file_name = from_file_name;
    }

    /* Сохранить в указанный файл. */
    void saveTo(const std::string &log_file_name, const std::string &temperature) {
        if (!HelpUtils::isNumber(temperature)) {
            return;
        }
        auto now = std::chrono::system_clock::now();
        std::time_t cur_time = std::chrono::system_clock::to_time_t(now);

        std::stringstream ss;
        ss << std::put_time(std::localtime(&cur_time), DATETIME_FORMAT);
        std::string dateTimeStr = ss.str();

        std::ofstream logfile;
        logfile.open(log_file_name, std::ios::app);

        if (logfile.is_open()) {
            logfile << temperature << LOG_SEPARATOR << dateTimeStr << std::endl;
            logfile.close();
        } else {
            std::cerr << "Failed to open log file: " << log_file_name << std::endl;
        }
    }

    /* Получить в среднюю температуру за указанный временной интервал. */
    int getAverageTemperatureByTimeInterval(
            const std::chrono::system_clock::time_point &start_date_time,
            const std::chrono::system_clock::time_point &end_date_time) {

        if (_from_file_name.empty()) {
            std::cerr << "Set file from get info! " << std::endl;
            return -1;
        }
        int summary_temperatures = 0;
        int count_values = 0;

        std::ifstream log_file(_from_file_name);
        if (!log_file) {
            std::cerr << "Failed to open log file: " << _from_file_name << std::endl;
            return -1;
        }

        std::string line;

        while (std::getline(log_file, line)) {
            if (line.empty()) {
                continue;
            }
            std::istringstream ss(line.substr(line.find(LOG_SEPARATOR) + 1));
            std::string datetime_str = ss.str();

            std::tm time_info = HelpUtils::convertStringToTm(datetime_str);
            auto temperature_created_date_time = std::chrono::system_clock::from_time_t(std::mktime(&time_info));

            if (temperature_created_date_time >= start_date_time && temperature_created_date_time <= end_date_time) {
                count_values++;
                std::string temperature_str = line.substr(0, line.find(LOG_SEPARATOR));
                summary_temperatures += std::stoi(temperature_str);
            }
        }
        if (count_values == 0) {
            return 0;
        }

        std::cout << "average temperature = " << summary_temperatures / count_values << std::endl;
        log_file.close();
        return summary_temperatures / count_values;
    }

private:
    std::string _from_file_name;
};