/*
 * Logger.h
 *
 * @author zocker_160
 * @license MIT
 * @description simple header only logger class
 */
#pragma once

#include <Windows.h>
#include <iostream>
#include <string>

namespace Logging {

typedef std::ostream& (*Manip1)(std::ostream&);

class Logger {
public:
    explicit Logger(std::string module, bool console = false) {
        this->module = module;

        if (console)
            setupConsole();
    }

    explicit Logger(std::string module, std::string logfile, bool console = false) {
        this->module = module;
        remove(logfile.c_str());

        if (console)
            setupConsole();
        else
            setupLogfile(logfile);
    }

    template<typename T>
    Logger& operator<<(T t) {
        std::cout << t;
        return *this;
    }
    Logger& operator<<(Manip1 fp) {
        std::cout << fp;
        return *this;
    }

    Logger& debug() {
        return printSelf("DEBUG");
    }
    void debug(std::string msg) {
        debug() << msg << std::endl;
    }

    Logger& info() {
        return printSelf("INFO");
    }
    void info(std::string msg) {
        info() << msg << std::endl;
    }

    Logger& warn() {
        return printSelf("WARN");
    }
    void warn(std::string msg) {
        warn() << msg << std::endl;
    }

    Logger& error() {
        return printSelf("ERROR");
    }
    void error(std::string msg) {
        error() << msg << std::endl;
    }

    Logger& naked() {
        return *this;
    }
    void naked(std::string msg) {
        naked() << msg << std::endl;
    }

    friend std::ostream& operator<<(std::ostream& out, const Logger& logger) {
        out << "[" << logger.module << "] " << logger.level << ":";
        return out;
    }

private:
    std::string module;
    std::string level = "DEBUG";

    FILE* fp;
    FILE* flog;

    Logger& printSelf(std::string level) {
        this->level = level;
        std::cout << *this << "\t";
        return *this;
    }

    void setupConsole() {
        AllocConsole();
        freopen_s(&fp, "CONOUT$", "w", stdout);
    }

    void setupLogfile(std::string logfile) {
        freopen_s(&flog, logfile.c_str(), "w", stdout);
    }
};

}
