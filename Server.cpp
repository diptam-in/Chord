#include "Chord.h"

Server :: Server(void)
{
	SERVER_IP="localhost";
	SERVER_PORT=2999;
}
Server :: Server(string ip,int port)
{
	SERVER_IP=ip;
	SERVER_PORT=port;
}
void   Server :: set_server_ip(string ip)
{
	SERVER_IP=ip;
}
string Server :: get_server_ip()
{
	return SERVER_IP;
}
void   Server :: set_server_port(int port)
{
	SERVER_PORT=port;
}
int    Server :: get_server_port()
{
	return SERVER_PORT;
}
void   Server :: start_server(Node *node)
{
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	bzero((char *) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(SERVER_PORT);
    
    

    if (bind(sockfd, (struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0)
    	cout << "[ERROR] COULD NOT BIND";

    listen(sockfd,15);

    cout << "[INFO] SERVER IS UP\n";
    while(true)
    {
	    clilen = sizeof(cli_addr);
	    newsockfd = accept(sockfd,(struct sockaddr *) &cli_addr, &clilen);
	    thread t(&Server::handle_conncetion,this,newsockfd,node);
	    t.detach();
	}
}

void   Server :: handle_conncetion(int socketnum,Node *node)
{
	char buffer[1024];
	const char* resp;
	string cmd;
	bzero(buffer,1024);
    int n = read(socketnum,buffer,1024);
    cmd=buffer;
    resp = (node->rpc_handler(cmd,"NOT YET IMPLEMENTED")).c_str();
    n = write(socketnum,resp,strlen(resp));
    close(socketnum);
}

void   Server :: close_server()
{
    close(sockfd);
}