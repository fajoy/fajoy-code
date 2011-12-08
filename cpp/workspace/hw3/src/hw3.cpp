#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
using namespace std;


typedef struct reqdata{
	char h1[255];
	char h2[255];
	char h3[255];
	char h4[255];
	char h5[255];
	char p1[5];
	char p2[5];
	char p3[5];
	char p4[5];
	char p5[5];
	char f1[255];
	char f2[255];
	char f3[255];
	char f4[255];
	char f5[255];
}reqdata;
char *query;
reqdata req;
void init_req(){
	query = getenv("QUERY_STRING");
	if (query == NULL) {
	char	testquery[] ="h1=127.0.0.1&p1=800&f1=test1.txt&h2=&p2=&f2=&h3=&p3=&f3=&h4=&p4=&f4=&h5=&p5=&f5=";
	query = testquery;
	}
	memset(&req,0,sizeof(req));

	sscanf(query,"h1=%[^&]&p1=%[^&]&f1=%[^&]&"
			"h2=%[^&]&p2=%[^&]&f2=%[^&]&"
			"h3=%[^&]&p3=%[^&]&f3=%[^&]&"
			"h4=%[^&]&p4=%[^&]&f4=%[^&]&"
			"h5=%[^&]&p5=%[^&]&f5=%[^&]",
			&req.h1[0],&req.p1[0],&req.f1[0],
			&req.h2[0],&req.p2[0],&req.f2[0],
			&req.h3[0],&req.p3[0],&req.f3[0],
			&req.h4[0],&req.p4[0],&req.f4[0],
			&req.h5[0],&req.p5[0],&req.f5[0]
			);


}

class SerCon {
public:
	char host[255];
	int port;
	char filename[255];
	SerCon(void);
	void parse(char *,  char *,  char *);
	void show(void);
	void js(int,char *);
};

SerCon::SerCon(void) {
	memset(host, 0, sizeof(host));
	memset(filename, 0, sizeof(filename));
	port = 0;
}
void SerCon::parse(char *shost, char* sport,char *sfilename) {
	strcpy(host,shost);
	strcpy(filename,sfilename);
	port = atoi(sport);
}

void SerCon::js(int i,char *line) {
	printf("<script>document.all['m%d'].innerHTML += \"%s<br>\";</script>\n",i,line);
}
void SerCon::show(void) {
	printf("h=%s,p=%d,f=%s<br />", host, port, filename);
}
SerCon sc[5];

void print_table_row(){
	int i=0;
	printf("<tr>");
	for(i=0;i<5;i++){
		if(strlen(sc[i].host)!=0)
		{
			printf("<td>%s</td>",sc[i].host);
		}
	}
	printf("</tr>");

	printf("<tr>");
		for(i=0;i<5;i++){
			if(strlen(sc[i].host)!=0)
			{
				printf("<td valign=\"top\" id=\"m%d\"></td>",i);
			}
		}
		printf("</tr>");

}
void begin_html(){
	printf("<html>"
			"<head>"
			"<meta http-equiv=\"Content-Type\" content=\"text/html; charset=big5\" />"
			"</head>"
			"<body bgcolor=#336699>"
			"<font face=\"Courier New\" size=2 color=#FFFF99>"
					"<table width=\"800\" border=\"1\">");
	print_table_row();
	printf("</table>");
	/*
    <html>
    <head>
    <meta http-equiv="Content-Type" content="text/html; charset=big5" />
    <title>Network Programming Homework 3</title>
    </head>
    <body bgcolor=#336699>
    <font face="Courier New" size=2 color=#FFFF99>
    <table width="800" border="1">
    <tr>
    <td>140.113.210.145</td><td>140.113.210.145</td><td>140.113.210.145</td></tr>
    <tr>
    <td valign="top" id="m0"></td><td valign="top" id="m1"></td><td valign="top" id="m2"></td></tr>
    </table>
    <script>document.all['m0'].innerHTML += "****************************************************************<br>";</script>
    <script>document.all['m1'].innerHTML += "****************************************************************<br>";</script>
    <script>document.all['m0'].innerHTML += "** Welcome to the information server, dist5.csie.nctu.edu.tw. **<br>";</script>
    <script>document.all['m0'].innerHTML += "** You are in the directory, /.<br>";</script>
    <script>document.all['m1'].innerHTML += "** You are in the directory, /.<br>";</script>
    <script>document.all['m2'].innerHTML += "****************************************************************<br>";</script>
    <script>document.all['m1'].innerHTML += "% <b>removetag test.html</b><br>";</script>
    <script>document.all['m1'].innerHTML += "<br>";</script>
    <script>document.all['m1'].innerHTML += "Test<br>";</script>
    <script>document.all['m1'].innerHTML += "This is a test program<br>";</script>
    <script>document.all['m1'].innerHTML += "for ras.<br>";</script>
    <script>document.all['m1'].innerHTML += "<br>";</script>
    <script>document.all['m1'].innerHTML += "<br>";</script>
    <script>document.all['m1'].innerHTML += "% <b></b><br>";</script>
    ....
    </font>
    </body>
    </html>*/

}
void end_html(){
printf("</font>"
    "</body>"
    "</html>");
}



void initSc(){


		try {
			sc[0].parse(&req.h1[0],&req.p1[0],&req.f1[0]);
			sc[1].parse( &req.h2[0],&req.p2[0],&req.f2[0]);
			sc[2].parse( &req.h3[0],&req.p3[0],&req.f3[0]);
			sc[3].parse( &req.h4[0],&req.p4[0],&req.f4[0]);
			sc[4].parse( &req.h5[0],&req.p5[0],&req.f5[0]);
		} catch (...) {
			cout << "parse error!";
		}
}

int main(int argc, char* argv[], char *envp[]) {
	init_req();

 close(0);
 fcntl(1, F_SETFL, O_NONBLOCK);
 //printf("HTTP/1.1 200 OK\n");

 printf("Cache-Control: max-age=0\n");
 printf("Content-type: text/html\n\n");
 fflush(stdout);
	initSc();
	begin_html();
	for(int i='a';i<'d';i++){
		for(int j=0;j<5;j++){
			char msg[]="a";
			msg[0]=i;
			sc[j].js(j,&msg[0]);
			fflush(stdout);
			usleep(100000);
		}
	}

	//sc[0].show();

	end_html();
	return 0;
}
