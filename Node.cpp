#include "Chord.h"

/* Delimiters and key values used in preparing messages */
#define K_DELIMETER ";"
#define K_EQUAL ":"
#define K_IP "ip"
#define K_PORT "port"
#define K_ID "id"
#define K_RQST "rqst"
#define K_IND "index"

/* 
* Constructor : paremeter network interface name
* default ID : 1
* default port: 2999
*/
Node :: Node(char* iface)
{
	id=1;
	inet_interface=iface;
	string s=get_local_addr(inet_interface);
	cout << "[INFO] ID: " << id << " IP: " << s << endl;
	server.set_server_ip(s);
	server.set_server_port(2999);
	thread(&Server::start_server,&server,this).detach();
}
/*
* Constructor: paremeter network interface name
* and ID
* default port: 2999
*/
Node :: Node(int i,char* iface)
{
	id=i;
	inet_interface=iface;
	string s=get_local_addr(inet_interface);
	cout << "[INFO] ID: " << id << " IP: " << s << endl;
	server.set_server_ip(s);
	server.set_server_port(2999);
	thread(&Server::start_server,&server,this).detach();
}
/*
* Constructor: paremeter network interface name
* and ID and port number
*/
Node :: Node(int port,int i,char* iface)
{
	id=i;
	inet_interface=iface;
	string s=get_local_addr(inet_interface);
	cout << "[INFO] ID: " << id << "\n[INFO] IP: " << s << endl;
	server.set_server_ip(s);
	server.set_server_port(port);
	thread(&Server::start_server,&server,this).detach();
}
/*
* Method to find out Node own IP address corresponding to an
* Interface passed as parameter  
*/
string Node :: get_local_addr(char* iface)
{
	struct ifaddrs * ifAddrStruct=NULL;
    struct ifaddrs * ifa=NULL;
    void * tmpAddrPtr=NULL;
    char addressBuffer[INET_ADDRSTRLEN];
    getifaddrs(&ifAddrStruct);
    for (ifa = ifAddrStruct; ifa != NULL; ifa = ifa->ifa_next) {
        if (!ifa->ifa_addr) {
            continue;
        }
        if (ifa->ifa_addr->sa_family == AF_INET) {
        	tmpAddrPtr=&((struct sockaddr_in *)ifa->ifa_addr)->sin_addr;
        	bzero(addressBuffer, INET_ADDRSTRLEN);
        	inet_ntop(AF_INET, tmpAddrPtr, addressBuffer, INET_ADDRSTRLEN);
        	if(strcmp(ifa->ifa_name,iface)==0)
        	{
        		string s(addressBuffer);
        		return s;
        	} 
        }
    }
	return NULL;
}

/*
* Method join: called when a Node wants to join a chord network
* Params: Known Node's IP, Port Number, IP
*/
void   Node :: join(string ip, int port, int id)
{
	int i;
	/* 
	* When it is the first node in Network i.e NO known node exists 
	* And passed parameters belong to this node itself
	*/
	if(this->id==id)
	{
		for(i=0;i<NUMBER_OF_BITS;i++)
		{
			fingerTable[i].intervalStart=(id + (2<<(i-1)))%(2<<(NUMBER_OF_BITS-1));
			if(i==0)
				fingerTable[i].intervalStart=(id + 1)%(2<<(NUMBER_OF_BITS-1));
			fingerTable[i].intervalEnd=(id + (2<<(i)))%(2<<(NUMBER_OF_BITS-1));
			fingerTable[i].node=id;
			fingerTable[i].port=server.get_server_port();
			fingerTable[i].ip=server.get_server_ip();
		}
		isInitialized=1;
		predecessor_id=id;
		predecessor_port=server.get_server_port();
		predecessor_ip=server.get_server_ip();
	}
	/*
	* When there exist a node in the network which is known to this Node
	* Methods are defined later
	*/
	else
	{
		init_finger_table(ip,port,id);
		isInitialized=1;
		// update_others();
		
	}
}

/* Initialize finger table using Known Node */
void   Node :: init_finger_table(string remote_ip, int remote_port, int remote_id)
{
	/* Set first finger of finger table, successor and predecessor of this node*/
	fingerTable[0].intervalStart=(id + 1)%(2<<(NUMBER_OF_BITS-1));
	fingerTable[0].intervalEnd=(id + (2<<(0)))%(2<<(NUMBER_OF_BITS-1));
	/* 
	* Prepare message to call remote method Find_successor
	* Format: "rqst:find_successor;id:2"
	*/
	string s;
	s=s.append(K_RQST).append(K_EQUAL).append("find_succesor").append(K_DELIMETER)
		.append(K_ID).append(K_EQUAL).append(to_string(fingerTable[0].intervalStart))
		.append(K_DELIMETER);
	/*
	* setup a client thread to communicate with remote Node
	* This thread will be responsible for Remote Procedure Call (RPC)
	* call_server_method returns the response of Remote Procedure
	*/
	client.set_server_ip(remote_ip);
	client.set_server_port(remote_port);
	string resp = client.call_server_method(s,"NOT YET IMPLEMENTED");
	fingerTable[0].node=atoi(get_value(K_ID,resp).c_str());
	fingerTable[0].port=atoi(get_value(K_PORT,resp).c_str());
	fingerTable[0].ip = get_value(K_IP,resp);
	/* 
	* Prepare message to call remote method Get_predecessor
	* Format: "rqst:find_successor;id:2"
	*/
	s.clear();
	s=s.append(K_RQST).append(K_EQUAL).append("get_predecessor").append(K_DELIMETER);
	client.set_server_ip(fingerTable[0].ip);
	client.set_server_port(fingerTable[0].port);
	resp=client.call_server_method(s,"NOT YET IMPLEMENTED");
	/* 
	* Parese response and store predecessor details
	*/
	predecessor_id=atoi(get_value(K_ID,resp).c_str());
	predecessor_ip=get_value(K_IP,resp);
	predecessor_port=atoi(get_value(K_PORT,resp).c_str());
	/* 
	* Prepare message to call remote method Update_predecessor
	* Format: "rqst:update_predecessor;id:2;ip:10.42.0.1;port:2099......"
	*/
	s.clear();
	s=s=s.append(K_RQST).append(K_EQUAL).append("update_predecessor").append(K_DELIMETER)
		.append(K_ID).append(K_EQUAL).append(to_string(this->id)).append(K_DELIMETER)
		.append(K_IP).append(K_EQUAL).append(server.get_server_ip()).append(K_DELIMETER)
		.append(K_PORT).append(K_EQUAL).append(to_string(server.get_server_port())).append(K_DELIMETER);

	client.set_server_ip(fingerTable[0].ip);
	client.set_server_port(fingerTable[0].port);
	resp=client.call_server_method(s,"NOT YET IMPLEMENTED");
	/* 
	* update rest of the fingers in the same manner as done for first finger
	*/
	int i;
	for(i=1;i<NUMBER_OF_BITS;i++)
	{
		
		fingerTable[i].intervalStart=(id + (2<<(i-1)))%(2<<(NUMBER_OF_BITS-1));
		fingerTable[i].intervalEnd=(id + (2<<(i)))%(2<<(NUMBER_OF_BITS-1));
		/*
		* IF i'th intervalStart belongs between Current_Node_ID and Previous_Finger_Node
		* THEN No need to find i'th Node since this Previous_Finger_Node will be Node
		* of current finger as well.
		*/
		if(does_belong(fingerTable[i].intervalStart,this->id,fingerTable[i-1].node-1))
		{
			fingerTable[i].node=fingerTable[i-1].node;
			fingerTable[i].ip=fingerTable[i-1].ip;
			fingerTable[i].port=fingerTable[i-1].port;
		}
		/*
		* IF i'th intervalStart does not reside between Current_Node_ID & Previous_Finger_Node
		* THEN query Existing Known Node to fill up details
		*/
		else
		{
			s.clear();
			s=s.append(K_RQST).append(K_EQUAL).append("find_succesor").append(K_DELIMETER)
				.append(K_ID).append(K_EQUAL).append(to_string(fingerTable[i].intervalStart))
				.append(K_DELIMETER);
			resp = client.call_server_method(s,"NOT YET IMPLEMENTED");
			fingerTable[i].node=atoi(get_value(K_ID,resp).c_str());
			fingerTable[i].port=atoi(get_value(K_PORT,resp).c_str());
			fingerTable[i].ip = get_value(K_IP,resp);
		}
	}


}

/*
* Method: update other's finger table when Own finger table is updated 
*/
void   Node :: update_others()
{
	int i;
	int p=1,num;
	string s;
	string uip;
	int uport;
	display_finger_table();

	for(i=0;i<NUMBER_OF_BITS;i++)
	{
		s.clear();
		num=((2<<(NUMBER_OF_BITS-1))+(this->id-p))%(2<<(NUMBER_OF_BITS-1));
		s=s.append(K_ID).append(K_EQUAL).append(to_string(num)).append(K_DELIMETER);
		s=find_predecessor(s);
		uip=get_value(K_IP,s);
		uport=atoi(get_value(K_PORT,s).c_str());
		client.set_server_ip(uip);
		client.set_server_port(uport);
		s.clear();
		s=s.append(K_ID).append(K_EQUAL).append(to_string(this->id)).append(K_DELIMETER)
			.append(K_IP).append(K_EQUAL).append(server.get_server_ip()).append(K_DELIMETER)
			.append(K_PORT).append(K_EQUAL).append(to_string(server.get_server_port()))
			.append(K_DELIMETER)
			.append(K_IND).append(K_EQUAL).append(to_string(i)).append(K_DELIMETER)
			.append(K_RQST).append(K_EQUAL).append("update_finger_table").append(K_DELIMETER);
		client.call_server_method(s,"NOT YET IMPLEMENTED");
		p=p*2;
	}
}

/*
* Method: to update entry in finger table, mostly this method is called from remote Node.
* Local finger table is initialized by initialize_finger_table
*/
string Node :: update_finger_table(string info)
{
	int i =  atoi(get_value(K_IND,info).c_str());
	int id = atoi(get_value(K_ID,info).c_str());
	if(does_belong(id,this->id,fingerTable[i].node-1))
	{
		fingerTable[i].node = id;
		fingerTable[i].ip = get_value(K_IP,info);
		fingerTable[i].port = atoi(get_value(K_PORT,info).c_str());
		client.set_server_ip(this->predecessor_ip);
		client.set_server_port(this->predecessor_port);
		client.call_server_method(info,"NOT YET IMPLEMENTED");
	}
	return "done";
}
/*
* Method to find successor of a Node 
* Node ID is passed as parameter
*/
string Node :: find_succesor(string info)
{
	string s;
	string predecessor = find_predecessor(info);
	string predecessor_ip = get_value(K_IP,predecessor);
	int predecessor_port = atoi(get_value(K_PORT,predecessor).c_str());
	s = s.append(K_RQST).append(K_EQUAL).append("get_successor").append(K_DELIMETER);
	client.set_server_port(predecessor_port);
	client.set_server_ip(predecessor_ip);
	string resp = client.call_server_method(s,"NOT YET IMPLEMENTED");
	return resp;
}
/*
* Method to set predecessor credentials
*/
string Node :: update_predecessor(string info)
{
	predecessor_ip=get_value(K_IP,info);
	predecessor_port=atoi(get_value(K_PORT,info).c_str());
	predecessor_id=atoi(get_value(K_ID,info).c_str());
	return "done";
}
/*
* Method to find predecessor of a Node 
* Node ID is passed as parameter
* Refer to paper for description of this method 
*/
string Node :: find_predecessor(string info)
{
	/* set target_ID= the ID for which predecessor is to be found */
	int targetid = atoi(get_value(K_ID,info).c_str());
	int myid = this->id;
	int mysuccessor_id = fingerTable[0].node;
	int my_port = server.get_server_port();
	string my_ip = server.get_server_ip();
	string s_targetid=to_string(targetid);
	string s;
	/*
	* LOOP UNTIL target_ID does not belong to CURRENT_ID and CURRENT_SUCCESSOR
	* IF target_ID belongs to CURRENT_ID and CURRENT_SUCCESSOR
	* THEN CURRENT_ID is the predecessor of target_ID
	*/
	while(!does_belong(targetid, mysuccessor_id+1,myid))
	{
		s.clear();
		s = s.append(K_ID).append(K_EQUAL).append(s_targetid).append(K_DELIMETER)
			.append(K_RQST).append(K_EQUAL).append("closest_preceding_finger")
			.append(K_DELIMETER);
		client.set_server_ip(my_ip);
		client.set_server_port(my_port);
		s = client.call_server_method(s,"NOT YET IMPLEMENTED");
		/*
		* Update CURRENT_ID with the new NODE's INFORMATION
		* i.e Update CURRENT_ID with a closer NODE to the target ID 
		*/
		myid = atoi(get_value(K_ID,s).c_str());
		my_ip= get_value(K_IP,s);
		my_port = atoi(get_value(K_PORT,s).c_str());
		s.clear();
		s = s.append(K_RQST).append(K_EQUAL).append("get_successor").append(K_DELIMETER);
		client.set_server_ip(my_ip);
		client.set_server_port(my_port);
		s=client.call_server_method(s,"NOT YET IMPLEMENTED");
		/*
		* Update the succesor as well
		*/
		mysuccessor_id=atoi(get_value(K_ID,s).c_str());
		
	}

	string resp;
	resp = resp.append(K_ID).append(K_EQUAL).append(to_string(myid)).append(K_DELIMETER)
			.append(K_IP).append(K_EQUAL).append(my_ip).append(K_DELIMETER)
			.append(K_PORT).append(K_EQUAL).append(to_string(my_port)).append(K_DELIMETER);
	return resp;
}
/* Method that returns successor of current_node; mostly called from remote Node*/
string Node :: get_successor()
{
		string s;
		s=	s.append(K_ID).append(K_EQUAL).append(to_string(fingerTable[0].node))
			.append(K_DELIMETER)
			.append(K_IP).append(K_EQUAL).append(fingerTable[0].ip)
			.append(K_DELIMETER)
			.append(K_PORT).append(K_EQUAL).append(to_string(fingerTable[0].port))
			.append(K_DELIMETER);
		return s;
}
/* Method that returns predecessor of current_node; mostly called from remote Node*/
string Node :: get_predecessor()
{
		string s;
		s=	s.append(K_ID).append(K_EQUAL).append(to_string(predecessor_id))
			.append(K_DELIMETER)
			.append(K_IP).append(K_EQUAL).append(predecessor_ip)
			.append(K_DELIMETER)
			.append(K_PORT).append(K_EQUAL).append(to_string(predecessor_port))
			.append(K_DELIMETER);
		return s;
}

string Node :: closest_preceding_finger(string info)
{
	int i;
	int id = atoi(get_value(K_ID,info).c_str());
	string s;
	for (i=0;i<NUMBER_OF_BITS;i++)
	{
		if(does_belong(fingerTable[i].node,this->id+1,id-1))
		{
			s.clear();
			s=	s.append(K_ID).append(K_EQUAL).append(to_string(fingerTable[i].node))
				.append(K_DELIMETER)
				.append(K_IP).append(K_EQUAL).append(fingerTable[i].ip)
				.append(K_DELIMETER)
				.append(K_PORT).append(K_EQUAL).append(to_string(fingerTable[i].port))
				.append(K_DELIMETER);
			return s;
		}
	}

	s.clear();
	s=	s.append(K_ID).append(K_EQUAL).append(to_string(this->id))
		.append(K_DELIMETER)
		.append(K_IP).append(K_EQUAL).append(server.get_server_ip())
		.append(K_DELIMETER)
		.append(K_PORT).append(K_EQUAL).append(to_string(server.get_server_port()))
		.append(K_DELIMETER);
	return s;
}
/* Method to display finger table */
void   Node :: display_finger_table()
{
	if(isInitialized)
	{
		int i;
		cout << "FINEGR TABLE\n";
		for(i=0;i<NUMBER_OF_BITS;i++)
		{
			cout << i << "] (" 
					<<fingerTable[i].intervalStart << " "
					<< fingerTable[i].intervalEnd << ") "
					<< fingerTable[i].node << " <"
					<< fingerTable[i].ip << ":"
					<< fingerTable[i].port <<">"<<endl;
		}
	}
	else
	{
		cout << "[INFO] TABLE NOT INITIALIZED\n";
	}
}
/*
* Parser method: parses data passed over network
* to extract information .
* Params: key = for which we are looking for the value
* msg = the whole string of data
*/
string Node :: get_value(string key, string msg)
{
	string k,s;
	int ind;
	int i=0,j = msg.find(K_DELIMETER);
	while(j!=string::npos)
	{ 
		s=msg.substr(i,j-i);
		ind=s.find(K_EQUAL);
		k=s.substr(0,ind);
		if(k.compare(key)==0)
			return s.substr(ind+1,s.length()-ind);
		i=j+1;
		j=msg.find(K_DELIMETER,j+1);
	}
	return NULL;
}
/*
* checks whether a given ID belongs to a specific range
* Params: ID, start_of_range, end_of_range
*/
bool   Node :: does_belong(int id,int start, int end)
{
	start=((2<<(NUMBER_OF_BITS-1))+(start))%(2<<(NUMBER_OF_BITS-1));
	end=((2<<(NUMBER_OF_BITS-1))+(end))%(2<<(NUMBER_OF_BITS-1));
	if(end<start)
		return !(id > end && id < start);

	else
		return (id >= start && id <= end);

}

/*
* Similar to marshalling-unmarshalling of RPC
* IT finds method_name from the DATA passed over 
* Network and calls corresponding method of local system.
*/
string Node :: rpc_handler(string cmd,string info)
{
	string rqst = get_value(K_RQST,cmd);
	if(rqst.compare("get_successor")==0)
		return get_successor();
	else if(rqst.compare("closest_preceding_finger")==0)
		return closest_preceding_finger(cmd);
	else if(rqst.compare("find_succesor")==0)
		return find_succesor(cmd);
	else if(rqst.compare("get_predecessor")==0)
		return get_predecessor();
	else if(rqst.compare("update_finger_table")==0)
		return update_finger_table(cmd);
	else if(rqst.compare("update_predecessor")==0)
		return update_predecessor(cmd);
	
	return cmd;
}
/*
* For command line interaction with the user
*/
int Node :: cli(int i)
{
	string ip,res,s;
	int p,id;
	switch(i)
	{
		case 0: 	cout << "[*] ENTER IP: ";
					cin >> ip;
					cout << "[*] ENTER PORT: ";
					cin >> p;
					cout << "[*] ENTER ID: ";
					cin >> id;
					join(ip,p,id);
					return 1;
		case 1:		cout << "[*] ENTER ELEMENT NAME\n";
					s=s.append(K_ID).append(K_EQUAL).append(to_string(1)).append(K_DELIMETER);
					cout << s << endl;
					cout << find_succesor(s) << endl;;
					return 1;
		case 2:		cout << "[*] ENTER ELEMENT NAME\n";
					return 1;
		case 3:		cout << "[INFO] STOPPING SERVER\n";
					server.close_server();
					return 0;
		case 4:		display_finger_table();
					return 1;
		default : 	cout << "[WARNING] WRONG INPUT\n";
					return 1;
	}
}

/*
* Driver method to start the node
* Params:
* arg1: PORT # on which the NODE will listen
* arg2: ID 
* arg3: INTERFACE NAME through which the node will communicate
*/
int main(int argc, char *argv[])
{
	cout << "*************************** CHORD **************************" <<endl;
	int ret;
	string cmd;

	if(argc==0)
	{
		cout << "[ERROR] REQUIRED PORT NUMBER IS NOT PRVIDED\n";
		exit(0);
	}

	Node node(atoi(argv[1]),atoi(argv[2]),argv[3]);
	
	
	while(true)
	{
		cout << "chord_prompt$ ";
		cin >> cmd;

		if(cmd=="join")
			ret=node.cli(0);
		else if(cmd=="get")
			ret=node.cli(1);
		else if(cmd=="put")
			ret=node.cli(2);
		else if(cmd=="stop")
			ret=node.cli(3);
		else if(cmd=="display")
			ret=node.cli(4);
		if(ret==0)
			break;
	}
	return 0;
}