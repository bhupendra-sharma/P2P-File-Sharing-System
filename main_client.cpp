// This is client.cpp
#include<iostream>
#include<bits/stdc++.h>
#include<sys/socket.h>
#include<pthread.h>
#include<unistd.h>
#include<sys/types.h> 
#include<netinet/in.h> //struct sockaddr_in
#include<string.h>
#include<netdb.h> //gethostbyname()
#include<fstream> // ifstream
// #define CHUNK_SIZE 16384    //16KB
#define CHUNK_SIZE 1024    //1KB
using namespace std;
string id="",pswd="",group="";
string my_ip="",my_port="";
string tracker_ip="",tracker_port="";
unordered_map<string,string> file_details; //filename->path
int online=0;

void print_trackers(vector<pair<string,string>> trackers)
{
    for(int i=0;i<trackers.size();i++)
    cout<<trackers[i].first<<":"<<trackers[i].second<<endl;
}
// void* execute_commands(void* client_socket)
void execute_commands(int client_fd)
{
    char buffer[1024];
    // int client_fd=*(int *)client_socket;
    bzero(buffer,1024);
    int n=read(client_fd,buffer,1024);
    // cout<<"Msg from other peer:"<<buffer<<":END"<<endl;
    string filepath=buffer;
    bzero(buffer,1000);
    ifstream f(file_details[filepath]);
    ifstream f_temp(file_details[filepath]);
    long long start,end,size;
    start=f_temp.tellg();
    f_temp.seekg(0,ios::end);
    end=f_temp.tellg();
    size=end-start;
    int chunks=(size/CHUNK_SIZE)+1;
    strcpy(buffer,to_string(chunks).c_str());
    n=write(client_fd,buffer,1000);
    char f_buffer[CHUNK_SIZE];
    for(int i=0;i<chunks;i++)
    {
        if(i==chunks-1)
        {
            bzero(f_buffer,CHUNK_SIZE);
            long long last_chunk=size-(chunks-1)*CHUNK_SIZE;
            f.read(f_buffer,last_chunk);
            n=write(client_fd,f_buffer,last_chunk);
            continue;
        }
        f.read(f_buffer,CHUNK_SIZE);
        n=write(client_fd,f_buffer,CHUNK_SIZE);
    }
    if (n < 0) perror("Error while writing to socket");
}
void listen_from_peer()
{
     /* Establishing connection */
     
    int sockfd = socket(AF_INET, SOCK_STREAM, 0); //Socket Creation
    if(sockfd<0)
    {
        perror("Error while opening socket.\n");
        exit(0);
    }
    struct sockaddr_in serv_addr, cli_addr;
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family=AF_INET;
    serv_addr.sin_addr.s_addr=INADDR_ANY;
    serv_addr.sin_port=htons(atoi(my_port.c_str()));
    if(bind(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr))<0) //Binding
    {
        perror("Error while binding.\n");
        exit(0);
    }
    listen(sockfd,8); //Listening
    socklen_t client_len=sizeof(cli_addr);

    //Accepting requests from other peers and executing them parallely
    while(1)
    {
        int newsockfd = accept(sockfd,(struct sockaddr *) &cli_addr,&client_len); //Accepting
        if(sockfd<0)
        {
            perror("Error while accepting.\n");
            exit(0);
        }
        // pthread_t client_thread;
        int client_socket=newsockfd;
        // pthread_create(&client_thread,NULL,&execute_commands,&client_socket);
        execute_commands(client_socket);
    }
}
int main(int argc,char *argv[])
{
    if(argc!=3)
    {
        perror("Please provide IP:PORT along with 'tracker_info.txt'\n");
        exit(0);
    }
    
    /* Extracting IP and port from tracker_info.txt */
    fstream file;
    string word;
    file.open(argv[2]);
    vector<pair<string,string>> trackers;
    while(file>>word)
    {
        string ip="",port="";
        for(int i=0;i<word.length();i++)
        {
            if(word[i]==':')
            {
                ip=port;
                port="";
                continue;
            }
            port.push_back(word[i]);
        }
        trackers.push_back({ip,port});
    }
    tracker_ip=trackers[0].first;
    tracker_port=trackers[0].second;
    
    word=argv[1];
    for(int i=0;i<word.length();i++)
    {
        if(word[i]==':')
        {
            my_ip=my_port;
            my_port="";
            continue;
        }
        my_port.push_back(word[i]);
    }


    thread peer_server_thread(listen_from_peer);
	peer_server_thread.detach();
    // listen_from_peer();

    cout<<"My IP:PORT->"<<my_ip<<":"<<my_port<<endl;
    int sockfd = socket(AF_INET, SOCK_STREAM, 0); //Socket Creation
    if(sockfd<0)
    {
        perror("Error while opening socket.\n");
        exit(0);
    }
    struct hostent *server=gethostbyname(tracker_ip.c_str());
    if(server==NULL) 
    {
        perror("Error,Host not found.\n");
        exit(0);
    }
    struct sockaddr_in serv_addr;
    bzero((char *) &serv_addr,sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr,(char *)&serv_addr.sin_addr.s_addr,server->h_length);
    serv_addr.sin_port = htons(stoi(tracker_port));
    if (connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr))<0) 
    {
        perror("ERROR connecting");
        exit(0);
    }
    while(1)
    {
        printf("\nEnter Command$ ");
        char buffer[1024];
        bzero(buffer,256);
        fgets(buffer,255,stdin); 
        buffer[strlen(buffer)-1]='\0';
        string inp=buffer,temp="";
        while(inp[inp.length()-1]=='\n')
        inp.pop_back();
        vector<string> command;
        for(int i=0;i<inp.length();i++)
        {
            if(inp[i]==' ')
            {
                command.push_back(temp);
                temp="";
                continue;
            }
            temp+=inp[i];
        }
        if(temp!="")
        command.push_back(temp);
        
        // cout<<"COMMAND:";
        // for(int i=0;i<command.size();i++)
        // {
        //     cout<<command[i]<<" ";
        // }
        // cout<<endl;

        if(command[0]=="login")
        {
            if(online==1)
            {
                cout<<"You are already logged in as USER:"<<id<<endl;
            }
            else
            if(command.size()!=3)
            {
                cout<<"Invalid arguments\n";
            }
            else
            {
                int n=write(sockfd,buffer,strlen(buffer));
                if (n < 0) 
                {
                    perror("Error while writing to socket.\n");
                    exit(0);
                }
                bzero(buffer,1024);
                n=read(sockfd,buffer,1024);
                if(n<0) 
                {
                    perror("Error while reading from socket\n");
                    exit(0);
                }
                if(strcmp(buffer,"success")==0)
                {
                    online=1;
                    id=command[1];
                    pswd=command[2];
                    cout<<"Logged in successfully.\n";
                }
                else
                printf("Msg from Server:%s\n",buffer);
            }
        }
        else
        if(command[0]=="logout")
        {
            if(online==0)
            {
                cout<<"You are not online currently,please login.\n";
            }
            else
            {
                bzero(buffer,1024);
                strcpy(buffer,("logout "+id).c_str());
                // cout<<"Calling LOGOUT on server with "<<buffer<<endl;
                int n=write(sockfd,buffer,strlen(buffer));
                online=0;
                id="";
                pswd="";
                bzero(buffer,1024);
                n=read(sockfd,buffer,1024);
                if(n<0) 
                {
                    perror("Error while reading from socket\n");
                    exit(0);
                }
                printf("Msg from Server:%s\n",buffer);
            }
        }
        else 
        if(command[0]=="create_group")
        {
            if(online==0)
            {
                cout<<"Please login to create a group";
            }
            else
            if(command.size()!=2)
            {
                cout<<"Invalid arguments\n";
            }
            else
            {
                bzero(buffer,1024);
                strcpy(buffer,(inp+" "+id).c_str());
                int n=write(sockfd,buffer,strlen(buffer));           
                bzero(buffer,1024);
                n=read(sockfd,buffer,1024);
                if(n<0) 
                {
                    perror("Error while reading from socket\n");
                    exit(0);
                }
                if(strcmp(buffer,"success")==0)
                {
                    group=command[1];
                    cout<<"Group created successfully.\n";
                }
                else
                printf("Msg from Server:%s\n",buffer);
            }
        }
        else
        if(command[0]=="join_group")
        {
            if(online==0)
            {
                cout<<"Please login to join a group";
            }
            else
            if(command.size()!=2)
            {
                cout<<"Invalid arguments\n";
            }
            else
            {
                bzero(buffer,1024);
                strcpy(buffer,(inp+" "+id).c_str());
                int n=write(sockfd,buffer,strlen(buffer));           
                bzero(buffer,1024);
                n=read(sockfd,buffer,1024);              
                printf("Msg from Server:%s\n",buffer);
            }
        }
        else
        if(command[0]=="join_group")
        {
            if(online==0)
            {
                cout<<"Please login to join a group";
            }
            else
            if(command.size()!=2)
            {
                cout<<"Invalid arguments\n";
            }
            else
            {
                bzero(buffer,1024);
                strcpy(buffer,(inp+" "+id).c_str());
                int n=write(sockfd,buffer,strlen(buffer));           
                bzero(buffer,1024);
                n=read(sockfd,buffer,1024);              
                printf("Msg from Server:%s\n",buffer);
            }
        }
        else
        if(command[0]=="leave_group")
        {
            if(online==0)
            {
                cout<<"You are not logged in.Please login";
            }
            else
            if(command.size()!=2)
            {
                cout<<"Invalid arguments\n";
            }
            else
            {
                bzero(buffer,1024);
                strcpy(buffer,(inp+" "+id).c_str());
                int n=write(sockfd,buffer,strlen(buffer));           
                bzero(buffer,1024);
                n=read(sockfd,buffer,1024);              
                printf("Msg from Server:%s\n",buffer);
            }
        }
        else
        if(command[0]=="accept_request")
        {
            if(online==0)
            {
                cout<<"You are not logged in,Please login.\n";
            }
            else
            if(command.size()!=3)
            {
                cout<<"Invalid arguments\n";
            }
            else
            {
                strcpy(buffer,(inp+" "+id).c_str());
                int n=write(sockfd,buffer,strlen(buffer));
                if (n < 0) 
                {
                    perror("Error while writing to socket.\n");
                    exit(0);
                }
                bzero(buffer,1024);
                n=read(sockfd,buffer,1024);
                if(n<0) 
                {
                    perror("Error while reading from socket\n");
                    exit(0);
                }
                printf("Msg from Server:%s\n",buffer);
            }
        }
        else
        if(command[0]=="upload_file")
        {
            string filename="",filepath=command[1];
            bool isCorrect=false;
            for(int i=filepath.length()-1;i>=0;i--)
            {
                if(filepath[i]=='/')
                {
                    isCorrect=true;
                    break;
                }
                filename.insert(0,1,filepath[i]);
            }
            if(!isCorrect)
            cout<<"Incorrect Filename/path.";
            else
            if(online==0)
            {
                cout<<"You are not logged in,Please login.\n";
            }
            else
            if(command.size()!=3)
            {
                cout<<"Invalid arguments\n";
            }
            else
            if(file_details.find(filename)!=file_details.end())
            cout<<filename+" already uploaded with PATH:"+file_details[filename];
            else
            {
                file_details[filename]=command[1];
                strcpy(buffer,(inp+" "+id+" "+my_ip+" "+my_port).c_str());
                int n=write(sockfd,buffer,strlen(buffer));
                if (n < 0) 
                {
                    perror("Error while writing to socket.\n");
                    exit(0);
                }
                bzero(buffer,1024);
                n=read(sockfd,buffer,1024);
                if(n<0) 
                {
                    perror("Error while reading from socket\n");
                    exit(0);
                }
                printf("Msg from Server:%s\n",buffer);
            }
        }
        else
        if(command[0]=="download_file")
        {
            if(online==0)
            {
                cout<<"You are not logged in,Please login.\n";
            }
            else
            if(command.size()!=4)
            {
                cout<<"Invalid arguments\n";
            }
            else
            {
                strcpy(buffer,(inp+" "+id).c_str());
                int n=write(sockfd,buffer,strlen(buffer));
                if (n < 0) 
                {
                    perror("Error while writing to socket.\n");
                    exit(0);
                }
                bzero(buffer,1024);
                n=read(sockfd,buffer,1024);
                if(n<0) 
                {
                    perror("Error while reading from socket\n");
                    exit(0);
                }
                if(buffer[strlen(buffer)-1]!='#')   //received error log
                printf("Msg from Server:%s\n",buffer);
                else    //received success msg with peer ip,port having # as indicator in end
                {
                    string peer_ip="",peer_port="";
                    for(int i=0;i<strlen(buffer);i++)
                    {
                        if(buffer[i]==' ')
                        {
                            peer_ip=peer_port;
                            peer_port="";
                            continue;
                        }
                        peer_port.push_back(buffer[i]);
                    }
                    peer_port.pop_back();   //removing '#' indicator
                    int sockfd_peer = socket(AF_INET, SOCK_STREAM, 0); //Socket Creation
                    if(sockfd_peer<0)
                    {
                        perror("Error while opening socket.\n");
                        exit(0);
                    }
                    struct hostent *server_peer=gethostbyname(peer_ip.c_str());
                    if(server_peer==NULL) 
                    {
                        perror("Error,Host not found.\n");
                        exit(0);
                    }
                    struct sockaddr_in serv_addr_peer;
                    bzero((char *) &serv_addr_peer,sizeof(serv_addr_peer));
                    serv_addr_peer.sin_family = AF_INET;
                    bcopy((char *)server_peer->h_addr,(char *)&serv_addr_peer.sin_addr.s_addr,server->h_length);
                    serv_addr_peer.sin_port = htons(stoi(peer_port));
                    if (connect(sockfd_peer,(struct sockaddr *)&serv_addr_peer,sizeof(serv_addr_peer))<0) 
                    {
                        perror("ERROR connecting");
                        exit(0);
                    }
                    strcpy(buffer,command[2].c_str());
                    cout<<"Sending:"<<buffer<<endl;
                    int n=write(sockfd_peer,buffer,strlen(buffer)); //sending filepath
                    if(n<0) perror("ERROR writing to socket");
                    bzero(buffer,1024);
                    n=read(sockfd_peer,buffer,1024); //receiving chunk size/error
                    if (n < 0) perror("ERROR reading from socket");
                    printf("Msg from other peer:%s\n",buffer);
                    char f_buffer[CHUNK_SIZE];
                    fstream f;
                    string filepath=command[3]+'/'+command[2];
                    f.open(filepath.c_str(),fstream::out);
                    for(int i=0;i<stoi(buffer);i++)
                    {
                        n=read(sockfd_peer,f_buffer,CHUNK_SIZE);
                        f.write(f_buffer,CHUNK_SIZE);
                    }
                    // f.close();
                }
            }
        }
        else
        {
            int n=write(sockfd,buffer,strlen(buffer));
            if(n<0) 
            {
                perror("ERROR writing to socket");
                exit(0);
            }
            bzero(buffer,1024);
            n=read(sockfd,buffer,1024);
            if (n < 0) 
            {
                 perror("ERROR reading from socket");
                exit(0);
            }
            printf("Msg from Server:%s\n",buffer);
        }
    }
    close(sockfd);    
    return 0;
}