#ifndef HTTPCLIENT_H
#define HTTPCLIENT_H

#include <unistd.h>
#include <string>

#include <curlpp/cURLpp.hpp>
#include <curlpp/Easy.hpp>
#include <curlpp/Options.hpp>
#include <curlpp/Exception.hpp>
#include <curlpp/Infos.hpp>
#include <sstream>

class HttpClient{

    public:

        static std::string Post(std::string url, std::string token = "", std::string json = "");
        static std::string Get(std::string url, std::string token = "");

    protected:
        static std::string Call(std::string verb, std::string url, std::string token = "", std::string json = "");
};

#endif
