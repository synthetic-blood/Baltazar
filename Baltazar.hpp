#include<map>
#include<vector>
#include<string>
#include"sana.hpp"
#include<asio/ip/tcp.hpp>
void Log(const char* Message) {
	std::cout << "Log: " << Message << std::endl;
}
namespace Baltazar {
	
	using RequestMap = std::map<std::string, std::string>;
	std::map<std::string, std::string> MimeTypes;
	void LoadMimeTypes() {
		MimeTypes[".html"] = "text/html;charset=utf-8";
		MimeTypes[".htm"] = "text/html;charset=utf-8";
		MimeTypes[".css"] = "text/css";
		MimeTypes[".js"] = "text/javascript";
		MimeTypes[".png"] = "image/png";
		MimeTypes[".jpg"] = "image/jpg";
		MimeTypes[".gif"] = "image/gif";
		MimeTypes[".svg"] = "image/svg";
		MimeTypes[".webp"] = "image/webp";
		MimeTypes[".ico"] = "image/ico";
		MimeTypes[".woff"] = "font/woff";
		MimeTypes[".woff2"] = "font/woff2";
		MimeTypes[".ttf"] = "font/ttf";
	}

	RequestMap GetRequestProperties(const std::string& RawRequest) {
		RequestMap Values;
		std::stringstream SS(RawRequest);
		std::string Line;
		while (std::getline(SS, Line)) {
			if (Line.find("HTTP/") != std::string::npos) {
				std::string uri = Line.substr(Line.find('/') + 1, Line.find_last_of(' ') - (Line.find('/') + 1));
				Values["URI"] = uri == "" ? "index.html" : uri;
			}
			else {
				std::string Key, Value;
				Key = Line.substr(0, Line.find(':'));
				Value = Line.substr(Line.find(':') + 2, Line.size() - Key.size() - 3);
				Values[Key] = Value;
			}
		}
		return Values;
	}
	
	std::vector<char> ComposeResponse(RequestMap Request) {
		std::vector<char> RawBinaries;
		std::ostringstream Response;
		const fs::path WWWPath("www");
		fs::path AbsoluteResourcePath = WWWPath / fs::path(Request["Host"]) / fs::path(Request["URI"]);

		if (std::filesystem::exists(AbsoluteResourcePath)) {
			Response << "HTTP/1.1 200 OK\r\n";
		}
		else {
			Request["URI"] = "404.html";
			AbsoluteResourcePath.replace_filename("404.html");
			Response << "HTTP/1.1 404 Not Found\r\n";
			Response << "Content-Type: text/html; charset=utf-8";
			Response << "Content-Length: ";
		}

		std::string MimeRequest = Baltazar::MimeTypes[AbsoluteResourcePath.filename().extension().string()];
		
		RawBinaries = sana::ReadFile(AbsoluteResourcePath.string());
		Response << "Content-Type: " << MimeRequest << "\r\n";
		Response << "Content-Length: " << RawBinaries.size() << "\r\n";
		Response << "Server: Baltazar I\r\n";
		Response << "\r\n";
		std::vector<char> RawResponse;
		for (const char c : Response.str()) {
			RawResponse.push_back(c);
		}
		RawResponse.insert(RawResponse.end(), RawBinaries.begin(), RawBinaries.end());
		return RawResponse;
	}
	std::map<std::string, std::string> LocalDirecory;
}