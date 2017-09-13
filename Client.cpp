#include "Chord.h"

void   Client :: set_server_ip(string ip)
{
	SERVER_IP=ip;
}
string Client :: get_server_ip()
{
	return SERVER_IP;
}
void   Client :: set_server_port(int port)
{
	SERVER_PORT=port;
}
int    Client :: get_server_port()
{
	return SERVER_PORT;
}
string Client :: call_server_method(string methodName, string parameter)
{
	int n;
	char buffer[1024];
	strcpy(buffer,methodName.c_str());

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(SERVER_PORT);
    inet_pton(AF_INET, SERVER_IP.c_str(), &serv_addr.sin_addr);
    // printf("[DEBUG][INSIDE call_server_method] IP %s PORT %d INFO %s\n",SERVER_IP.c_str(),SERVER_PORT,
    	// methodName.c_str());
    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
        cout << "[ERROR] WHILE CONNECTING\n";

    n = write(sockfd,buffer,strlen(buffer));
    if (n < 0) 
         cout << "[ERROR] WHILE WRITNG TO SOCKET\n";

    bzero(buffer,1024);
    n = read(sockfd,buffer,1024);
    // printf("[DEBUG][INSIDE call_server_method] RECEIVED %s\n",buffer);
    close(sockfd);
    string res(buffer);
	return res;
}