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
using namespace std;
string id="",pswd="",group="";
int online=0;

void print_trackers(vector<pair<string,string>> trackers)
{
    for(int i=0;i<trackers.size();i++)
    cout<<trackers[i].first<<":"<<trackers[i].second<<endl;
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

    string ip_str="",port_str="";
    word=argv[1];
    for(int i=0;i<word.length();i++)
    {
        if(word[i]==':')
        {
            ip_str=port_str;
            port_str="";
            continue;
        }
        port_str.push_back(word[i]);
    }
    cout<<"IP:PORT->"<<ip_str<<":"<<port_str<<endl;
    int sockfd = socket(AF_INET, SOCK_STREAM, 0); //Socket Creation
    if(sockfd<0)
    {
        perror("Error while opening socket.\n");
        exit(0);
    }
    struct hostent *server=gethostbyname(ip_str.c_str());
    if(server==NULL) 
    {
        perror("Error,Host not found.\n");
        exit(0);
    }
    struct sockaddr_in serv_addr;
    bzero((char *) &serv_addr,sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr,(char *)&serv_addr.sin_addr.s_addr,server->h_length);
    serv_addr.sin_port = htons(stoi(port_str));
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