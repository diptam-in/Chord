#include <iostream>
#include <thread>
#include <string>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <netdb.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <ifaddrs.h>

#define NUMBER_OF_BITS 3

using namespace std;

class Node;

class Client
{
private:
	string SERVER_IP;
	int SERVER_PORT;
	struct sockaddr_in serv_addr;
    int sockfd, portno, n;
public:
	void   set_server_ip(string ip);
	string get_server_ip();
	void   set_server_port(int port);
	int    get_server_port();
	string call_server_method(string methodName, string parameter);
};


class Server
{
private:
	string SERVER_IP;
	int    SERVER_PORT,sockfd,newsockfd;
	struct sockaddr_in serv_addr, cli_addr;
	socklen_t clilen;
public:
	Server(void);
	Server(string ip,int port);
	void   set_server_ip(string ip);
	string get_server_ip();
	void   set_server_port(int port);
	int    get_server_port();
	void   start_server(Node *node);
	void   handle_conncetion(int socketnum,Node *node);
	void   close_server();
};


struct FingerTableEntry
{
	int intervalStart;
	int intervalEnd;
	int node;
	string ip;
	int port;
};

typedef FingerTableEntry FTEntry;

class Node
{
private:
	char* inet_interface;
	int isInitialized=0;
	int id;
	int successor_id,successor_port;
	string successor_ip;
	int predecessor_id,predecessor_port;
	string predecessor_ip;
	Server server;
	Client client;
	FTEntry fingerTable[NUMBER_OF_BITS];
public:
	Node(char* iface);
	Node(int i, char* iface);
	Node(int port,int i,char* iface);
	int    cli(int i);
	bool   does_belong(int id,int start, int end);
	void   join(string ip, int port, int id);
	void   init_finger_table(string remote_ip, int remote_port, int remote_id);
	void   update_others();
	void   display_finger_table();
	string update_predecessor(string info);
	string update_finger_table(string info);
	string find_succesor(string info);
	string find_predecessor(string info);
	string get_successor();
	string get_predecessor();
	string get_value(string key, string msg); 
	string closest_preceding_finger(string info);
	string rpc_handler(string cmd,string info);
	string get_local_addr(char* iface);
};