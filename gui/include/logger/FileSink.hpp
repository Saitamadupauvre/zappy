#pragma once
#include "ISink.hpp"
#include <fstream>
#include <string>
#include <chrono>
#include <iomanip>
#include <sstream>

namespace zappy {

class FileSink : public ISink
{
    public:
        explicit FileSink(const std::string& filename) {
            _file.open(filename, std::ios::out | std::ios::app);
            
            if (_file.is_open()) {
                writeSessionHeader();
            }
        }

        ~FileSink() {
            if (_file.is_open()) {
                _file << "====================================================================\n"
                      << "                     SESSION RUN TERMINATED                         \n"
                      << "====================================================================\n\n" 
                      << std::flush;
                _file.close();
            }
        }

        void write([[maybe_unused]] LogLevel level, std::string_view formattedMessage) override {
            if (_file.is_open()) {
                _file << formattedMessage << std::flush;
            }
        }

    private:
        std::ofstream _file;

        void writeSessionHeader() {
            auto now = std::chrono::system_clock::now();
            auto timeT = std::chrono::system_clock::to_time_t(now);
            struct tm timeInfo;
            localtime_r(&timeT, &timeInfo);

            std::ostringstream oss;
            oss << "\n"
                << "====================================================================\n"
                << "  NEW SESSION STARTED AT: " << std::put_time(&timeInfo, "%Y-%m-%d %H:%M:%S") << "\n"
                << "====================================================================\n";
            
            _file << oss.str() << std::flush;
        }
};

} // namespace zappy