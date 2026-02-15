#include <httplib.h>
#include <time.h>
#include <iostream>
#include <fstream>
#include <functional>

#define LOG_FILE_PATH "/tmp/file_server_log_file"
#define RANDOM_CHARS_IN_FILENAME 32

std::string getCurrentDateTime(bool spaceInRes = true)
{
    time_t time_ = time(nullptr);
    tm datetime = *localtime(&time_);
    std::string res;
    if (datetime.tm_mday < 10)
    {
        res += "0";
    }
    res += std::to_string(datetime.tm_mday) + ".";
    if (datetime.tm_mon + 1 < 10)
    {
        res += "0";
    }
    res += std::to_string(datetime.tm_mon + 1) + ".";
    res += std::to_string(datetime.tm_year + 1900) + (spaceInRes ? " " : "_");
    if (datetime.tm_hour < 10)
    {
        res += "0";
    }
    res += std::to_string(datetime.tm_hour) + ":";
    if (datetime.tm_min < 10)
    {
        res += "0";
    }
    res += std::to_string(datetime.tm_min) + ":";
    if (datetime.tm_sec < 10)
    {
        res += "0";
    }
    res += std::to_string(datetime.tm_sec);
    return res;
}

std::string generateRandomFileName()
{
    std::string res = getCurrentDateTime(false);
    const char* hexChars = "0123456789abcdef";
    std::random_device randomDevice;
    for (int i = 0; i < RANDOM_CHARS_IN_FILENAME; ++i)
    {
        res.push_back(hexChars[randomDevice() % 16]);
    }
    return res;
}

int main()
{
    if (getuid())
    {
        std::cout << "The program must be run as root\n";
        return 1;
    }
    httplib::Server server;
    std::function<void(const httplib::Request&, httplib::Response&)> startPage 
        = [](const httplib::Request&, httplib::Response& response)
    {
        response.set_content(
            "Navigation:\n"
            "/ or /info - show this page\n"
            "/log - show logs\n"
            "/upload - send POST method to upload files in server /tmp "
                "directory\n", 
            "text/plain");
    };
    server.Get("/", startPage);
    server.Get("/info", startPage);
    server.Get("/log", [](const httplib::Request&, httplib::Response& response)
        {
            std::ifstream fin;
            fin.open(LOG_FILE_PATH);
            std::string logs;
            if (fin.is_open())
            {
                char ch = '\0';
                while (fin.get(ch))
                {
                    logs.push_back(ch);
                }
            }
            else
            {
                logs = "Error when opening log file";
            }
            fin.close();
            response.set_content(logs, "text/plain");
        });
    server.Post("/upload", 
        [](const httplib::Request& request, 
           httplib::Response& /*response*/, 
           const httplib::ContentReader& contentReader)
        {
            if (request.is_multipart_form_data())
            {
                httplib::MultipartFormDataItems files;
                contentReader([&](const httplib::MultipartFormData& file)
                {
                    files.push_back(file);
                    return true;
                }, 
                [&](const char* data, size_t size_)
                {
                    files.back().content.append(data, size_);
                    return true;
                });
                for (auto& file : files)
                {
                    if (!file.filename.empty())
                    {
                        std::ofstream fout;
                        fout.open("/tmp/" + file.filename);
                        fout << file.content;
                        fout.close();
                    }
                }
            }
            else
            {
                // file is not multipart, it's name cannot be received
                std::string filePath = "/tmp/" + generateRandomFileName();
                std::string fileContent;
                contentReader([&](const char* data, size_t size_)
                {
                    fileContent.append(data, size_);
                    std::ofstream fout;
                    fout.open(filePath);
                    fout << fileContent;
                    fout.close();
                    return true;
                });
            }
        });
    server.set_logger(
        [](const httplib::Request& request, const httplib::Response& response)
        {
            std::ofstream fout;
            fout.open(LOG_FILE_PATH, std::ofstream::app);
            std::string logStr = 
                getCurrentDateTime() + " " + 
                request.method + " " + 
                request.path + " " + 
                std::to_string(response.status) + "\n";
            fout << logStr;
            fout.close();
            std::cout << logStr;
        });
    std::cout << "Server is working\n";
    server.listen("0.0.0.0", 1616);
}

