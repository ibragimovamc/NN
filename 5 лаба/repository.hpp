#pragma once

#include <sstream>
#include <iostream>
#include <fstream>
#include <chrono>
#include <ctime>
#include <iomanip>
#include "help_utils.hpp"
#include "sqlite3.h"

const char* INITIAL_SQL = "CREATE TABLE IF NOT EXISTS log(id INTEGER PRIMARY KEY, value INT, date TEXT);"
                  "CREATE TABLE IF NOT EXISTS log_h(id INTEGER PRIMARY KEY, value INT, date TEXT);"
                  "CREATE TABLE IF NOT EXISTS log_d(id INTEGER PRIMARY KEY, value INT, date TEXT);"
                  "CREATE TABLE IF NOT EXISTS log_m(id INTEGER PRIMARY KEY, value INT, date TEXT);";

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
    void setTableFrom(const std::string &from_table_name) {
        this->_from_table_name = from_table_name;
    }

    /* Сохранить в указанный файл. */
    void saveTo(const std::string &log_table_name, const std::string &temperature) {

        sqlite3 *db = 0;
        char *err = 0;

        if( sqlite3_open("log_database.dblite", &db) ) {
            fprintf(stderr, "Ошибка открытия/создания БД: %s\n", sqlite3_errmsg(db));
        }
        else if (sqlite3_exec(db, INITIAL_SQL, 0, 0, &err)) {
            fprintf(stderr, "Ошибка SQL21: %sn", err);
            sqlite3_free(err);
        }

        if (!HelpUtils::isNumber(temperature)) {
            return;
        }
        auto now = std::chrono::system_clock::now();
        std::time_t cur_time = std::chrono::system_clock::to_time_t(now);

        std::stringstream ss;
        ss << std::put_time(std::localtime(&cur_time), DATETIME_FORMAT);
        std::string dateTimeStr = ss.str();

        std::ofstream logfile;
        logfile.open(log_table_name + ".txt", std::ios::app);

        std::ostringstream WRITE_TO_LOG_SQL;
        WRITE_TO_LOG_SQL << "INSERT INTO " << log_table_name << "(value, date) VALUES (" << temperature << ", " << "'"<<dateTimeStr<<"'" <<");";
        if (sqlite3_exec(db, WRITE_TO_LOG_SQL.str().c_str(), 0, 0, &err)) {
            fprintf(stderr, "Ошибка SQL: %sn", err);
            sqlite3_free(err);
        }

        if (logfile.is_open()) {
            logfile << temperature << LOG_SEPARATOR << dateTimeStr << std::endl;
            logfile.close();
        } else {
            std::cerr << "Failed to open log file: " << log_table_name << std::endl;
        }

        sqlite3_close(db);
    }

    /* Получить в среднюю температуру за указанный временной интервал. */
    int getAverageTemperatureByTimeInterval(
            const std::chrono::system_clock::time_point &start_date_time,
            const std::chrono::system_clock::time_point &end_date_time) {

        if (_from_table_name.empty()) {
            std::cerr << "Set table from get info! " << std::endl;
            return -1;
        }
        int summary_temperatures = 0;
        int count_values = 0;

        sqlite3 *db = 0;
        sqlite3_stmt *stmt;
        std::ostringstream query;
        query << "SELECT * FROM " << this->_from_table_name << ";";

        if( sqlite3_open("log_database.dblite", &db) ) {
            fprintf(stderr, "Ошибка открытия/создания БД: %s\n", sqlite3_errmsg(db));
        }
        sqlite3_prepare_v2(db, query.str().c_str(), -1, &stmt, 0);

        while ( sqlite3_step(stmt) == SQLITE_ROW ){
            std::tm time_info = HelpUtils::convertStringToTm((char*)sqlite3_column_text(stmt, 2));
            auto temperature_created_date_time = std::chrono::system_clock::from_time_t(std::mktime(&time_info));
            if (temperature_created_date_time >= start_date_time && temperature_created_date_time <= end_date_time) {
                count_values++;
                summary_temperatures += sqlite3_column_int(stmt, 1);
            }
        }
        sqlite3_finalize(stmt);
        sqlite3_close(db);

        std::string line;
        if (count_values == 0) {
            return 0;
        }

        std::cout << "average temperature = " << summary_temperatures / count_values<< std::endl;
        return summary_temperatures / count_values;
    }

private:
    std::string _from_table_name;
};