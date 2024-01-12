#pragma once
#include <sstream>
#include <iostream>
#include <fstream>
#include <chrono>
#include <ctime>
#include <iomanip>

#if !defined (WIN32)
#	include <unistd.h>
#	include <time.h>
#endif

#include <cstdlib>

class HelpUtils {
public:
    static bool isNumber(const std::string &str) {
        try {
            std::size_t pos = 0;
            std::stoi(str, &pos);
            return pos == str.length();
        } catch (const std::exception &) {
            return false;
        }
    }

    template<class T>
    static std::string baseTypeToString(const T &v) {
        std::ostringstream ss;
        ss << v;
        return ss.str();
    }

    static std::tm convertStringToTm(const std::string &dateTimeStr) {
        std::tm resultTimeInfo = {};

        std::istringstream ss(dateTimeStr);
        ss >> std::get_time(&resultTimeInfo, "%Y-%m-%d %H:%M:%S");

        if (ss.fail()) {
            std::cerr << "Date parsing failed!" << std::endl;
        }
        std::put_time(&resultTimeInfo, "%Y-%m-%d %H:%M:%S");

        return resultTimeInfo;
    }

    static void sleep(double timeout) {
#if defined (WIN32)
        if (timeout <= 0.0)
            ::Sleep(INFINITE);
        else
            ::Sleep((DWORD) (timeout * 1e3));
#else
        if (timeout <= 0.0)
            pause();
        else {
            struct timespec t;
            t.tv_sec = (int)timeout;
            t.tv_nsec = (int)((timeout - t.tv_sec)*1e9);
            nanosleep(&t, NULL);
        }
#endif
    }
};
