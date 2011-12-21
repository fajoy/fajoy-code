
using namespace std;
#include <map>
#include <string>

//SCRIPT_NAME = /~fajoy/cgi-bin/printenv.cgi
//SERVER_NAME = ubu.fajoy.co.cc
//SERVER_ADMIN = webmaster@localhost
//HTTP_ACCEPT_ENCODING = gzip,deflate,sdch
//HTTP_CONNECTION = keep-alive
//REQUEST_METHOD = GET
//HTTP_ACCEPT = text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8
//SCRIPT_FILENAME = /home/fajoy/public_html/cgi-bin/printenv.cgi
//SERVER_SOFTWARE = Apache/2.2.16 (Ubuntu)
//HTTP_ACCEPT_CHARSET = Big5,utf-8;q=0.7,*;q=0.3
//QUERY_STRING = h1=111&p1=111&f1=111&h2=11&p2=11&f2=11&h3=2&p3=2&f3=3&h4=4&p4=5&f4=6&h5=7&p5=8&f5=9
//REMOTE_PORT = 2487
//HTTP_USER_AGENT = Mozilla/5.0 (Windows NT 6.1) AppleWebKit/535.7 (KHTML, like Gecko) Chrome/16.0.912.63 Safari/535.7
//SERVER_PORT = 80
//SERVER_SIGNATURE =Apache/2.2.16 (Ubuntu) Server at ubu.fajoy.co.cc Port 80
//
//HTTP_ACCEPT_LANGUAGE = zh-TW,zh;q=0.8,en-US;q=0.6,en;q=0.4
//REMOTE_ADDR = 10.0.2.2
//SERVER_PROTOCOL = HTTP/1.1
//PATH = /usr/local/bin:/usr/bin:/bin
//REQUEST_URI = /~fajoy/cgi-bin/printenv.cgi?h1=111&p1=111&f1=111&h2=11&p2=11&f2=11&h3=2&p3=2&f3=3&h4=4&p4=5&f4=6&h5=7&p5=8&f5=9
//GATEWAY_INTERFACE = CGI/1.1
//SERVER_ADDR = 10.0.2.15
//DOCUMENT_ROOT = /var/www
//HTTP_HOST = ubu.fajoy.co.cc


class Request {
public:
	string _url;
	string _file;
	string _path;
	map<string, string> Variables;
	Request(){
		Variables["SERVER_ADDR"]="";
		Variables["REMOTE_ADDR"]="";
		Variables["SERVER_PORT"]="";
		Variables["REMOTE_PORT"]="";
		Variables["SERVER_PROTOCOL"]="";
		Variables["PATH"]="";
		Variables["REQUEST_URI"]="";
		Variables["DOCUMENT_ROOT"]="";
		Variables["HTTP_HOST"]="";
		Variables["QUERY_STRING"]="";
		Variables["SERVER_PROTOCOL"]="";
		Variables["REQUEST_METHOD"]="";
	}

	string& REQUEST_METHOD() {
			return Variables["REQUEST_METHOD"];
		}
	string& SERVER_PROTOCOL() {
		return Variables["SERVER_PROTOCOL"];
	}
	string& SERVER_ADDR() {
		return Variables["SERVER_ADDR"];
	}
	string& REMOTE_ADDR() {
		return Variables["REMOTE_ADDR"];
	}
	string& SERVER_PORT() {
		return Variables["SERVER_PORT"];
	}
	string& REMOTE_PORT() {
		return Variables["REMOTE_PORT"];
	}
	string& REQUEST_URI() {
		return Variables["REQUEST_URI"];
	}

	string& DOCUMENT_ROOT() {
			return Variables["DOCUMENT_ROOT"];
		}
	string& HTTP_HOST() {
			return Variables["HTTP_HOST"];
	}
	string& PATH() {
			return Variables["PATH"];
	}
	string& QUERY_STRING() {
			return Variables["QUERY_STRING"];
	}

	string& Page(){
		size_t urlsize=Variables["REQUEST_URI"].find("?");
		_url=Variables["REQUEST_URI"].substr(0,urlsize);
		QUERY_STRING()=Variables["REQUEST_URI"].substr(urlsize+1);

		size_t found;
		string str=_url;
		found=str.find_last_of("/\\");
		_path=str.substr(0,found);
		_file=str.substr(found+1);
		return _url;
	}

	bool isCgi(){
		Page();
		string str=_file;
		int pos= str.find(".cgi",str.length()-4);
		if (pos==-1)
			return false;
		else
			return true;
	}
	void setvar(){

		DOCUMENT_ROOT()=getenv("PWD");
		PATH()=getenv("PATH");


		clearenv();
		map<string,string>::iterator it;
		for (it = Variables.begin(); it != Variables.end(); it++) {
			setenv( (*it).first.c_str(), (*it).second.c_str(),1);
		}

	}



};
