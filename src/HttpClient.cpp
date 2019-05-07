#include "HttpClient.h"

std::string HttpClient::Post(std::string url, std::string token, std::string postBody)
{
    return HttpClient::Call("POST", url, token, postBody);
} // Post


std::string HttpClient::Get(std::string url, std::string token)
{
    return HttpClient::Call("GET", url, token);
} // Get


std::string HttpClient::Call(std::string verb, std::string url, std::string token, std::string postBody)
{
//    std::cout << "Starting " << verb << " request to " << url << std::endl;
    
    bool post = verb.compare("POST") ==  0;

    try {
		curlpp::Cleanup cleaner;
		curlpp::Easy request;

		request.setOpt(new curlpp::options::CustomRequest{verb});

		std::list<std::string> header;
		header.push_back("Content-Type: application/json");

		if(!token.empty())
		{
			// MS Auth
			header.push_back("ZUMO-API-VERSION: 2.0.0");
			header.push_back("X-ZUMO-AUTH: " + token);
		}

		request.setOpt(new curlpp::options::HttpHeader(header));

		request.setOpt(new curlpp::options::Url(url));	
		request.setOpt(new curlpp::options::Verbose(false));	
		if(post)
		{
			request.setOpt(new curlpp::options::PostFields(postBody));	
			request.setOpt(new curlpp::options::PostFieldSize(postBody.length()));
		}

		std::stringstream result;
		request.setOpt(curlpp::Options::WriteStream(&result));

		request.perform();
		
		/*
		if(post)
		{
			std::cout << "Response code: " 
				  << curlpp::infos::ResponseCode::get(request) 
				  << " - Full response: " << result.str() << std::endl;
		}
		*/
	        if(curlpp::infos::ResponseCode::get(request) == 200)
			return result.str();
		else
			return "";
	}
	catch(curlpp::LogicError & e){
		std::cout << e.what() << std::endl;
	}
	catch(curlpp::RuntimeError & e){
		std::cout << e.what() << std::endl;
	}
    return "";
} // Call
