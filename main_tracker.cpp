// This is tracker.cpp
#include<iostream>
#include<bits/stdc++.h>
#include<sys/socket.h>
#include<pthread.h>
#include<unistd.h>
#include<sys/types.h> 
#include<netinet/in.h> //struct sockaddr_in
using namespace std;
unordered_map<string,pair<string,pair<string,bool>>> user;//user[id]={pswd,{gid,isOnline()}}
unordered_map<string,set<string>> group;//group[gid]={uid1,uid2...}
unordered_map<string,string> admin;//admin[groupid]=admin_uid
unordered_map<string,set<string>> requests;
unordered_map<string,pair<pair<string,string>,pair<string,string>>> file_details;//file[filename]={{filepath,groupid},{ip,port}}
string process_command(vector<string> command)
{
    if(command[0]=="create_user"&&command.size()==3)
    {
        if(user.find(command[1])==user.end())
        {
            user[command[1]].first=command[2];
            user[command[1]].second.first="";
            user[command[1]].second.second=false;
            return "User created Successfully!";
        }
        else
        {
            return "User already exist!";
        }
    }
    else
    if(command[0]=="login"&&command.size()==3)
    {
        if(user.find(command[1])!=user.end())
        {
            if(user[command[1]].second.second)
            {
                return "User already online!";
            }
            else
            if(user[command[1]].first==command[2])
            {
                user[command[1]].second.second=true;
                return "success";
            }
            else
            {
                return "Incorrect Password";
            }
        }
        else
        {
            return "No user found!";
        }
    }
    else
    if(command[0]=="create_group"&&command.size()==3)
    {
        if(group.find(command[1])==group.end())
        {
            group[command[1]].insert(command[2]);
            admin[command[1]]=command[2];
            return "success";
        }
        else
        {
            return "Group already exist";
        }
    }
    else
    if(command[0]=="join_group"&&command.size()==3)
    {
        if(group.find(command[1])==group.end())
        {
            return "The Group that you are requesting to join does not exist.";
        }
        if(group[command[1]].find(command[2])!=group[command[1]].end())
        {
            return "You are already part of this group";
        }
        else
        {
            requests[command[1]].insert(command[2]);
            return "Joing group request has been created.You will be added to group once admin approve your joining request";
        }
    }
    else
    if(command[0]=="leave_group"&&command.size()==3)
    {
        if(group.find(command[1])==group.end())
        {
            return "The Group that you are requesting to leave does not exist.";
        }
        if(group[command[1]].find(command[2])!=group[command[1]].end())
        {
            if(admin[command[1]]!=command[2])
            {
                group[command[1]].erase(command[2]);
                return "You have left the group";
            }
            else
            {
                if(group[command[1]].size()==1)
                {
                    group.erase(command[1]);
                    return "You have left the group. Since you were the admin and only member present in the group,group has also been deleted.";
                }
                else
                {
                    group[command[1]].erase(command[2]);
                    admin[command[1]]=*group[command[1]].begin();
                    return "You have left the group. Since you were the admin of group,New admin will be USER:"+admin[command[1]];

                }
            }
        }
        else
        {
            return "You are not a part of this group.";
        }
    }
    else
    if(command[0]=="list_requests"&&command.size()==2)
    {
        if(group.find(command[1])==group.end())
        return "Group not found";
        if(requests[command[1]].empty())
        return "No requests found.";
        string req_list="";
        vector<string> temp;
        for(auto i=requests[command[1]].begin();i!=requests[command[1]].end();i++)
        {
            temp.push_back(*i);
        }             
        sort(temp.begin(),temp.end());
        for(int i=0;i<temp.size();i++)
            req_list+="\n"+temp[i];
        return req_list;
    }
    else
    if(command[0]=="accept_request"&&command.size()==4)
    {
        if(requests.find(command[1])==requests.end())
        {
            return "Group not found";
        }
        else
        if(requests[command[1]].find(command[2])==requests[command[1]].end())
        {
            return "no request found for USER:"+command[2]+" in GROUP:"+command[1];
        }
        else
        if(admin[command[1]]==command[3])
        {
            requests[command[1]].erase(command[2]);
            group[command[1]].insert(command[2]);
            return "User:"+command[2]+" has been added to GROUP:"+command[1];
        }
        else
        {
            return "Only admin can accept group joining request,you("+command[3]+") are not admin of GROUP:"+command[1];
        }
    }
    else
    if(command[0]=="list_groups"&&command.size()==1)
    {
        if(group.size()==0)
        return "No group found";
        string group_list="";
        vector<string> temp;
        for(auto i=group.begin();i!=group.end();i++)
        {
            temp.push_back((*i).first);
        }
        sort(temp.begin(),temp.end());
        for(int i=0;i<temp.size();i++)
            group_list+="\n"+temp[i];
        return group_list;
    }
    else
    if(command[0]=="list_files"&&command.size()==2)
    {
        if(group.find(command[1])==group.end())
        return "Group does not exist.";
        string ans="\n|FileName|\n";
        if(file_details.size()==0)
        return "No files uploaded in this group.";
        bool check=false;
        for(auto i=file_details.begin();i!=file_details.end();i++)
        {
            string key=(*i).first;
            if(file_details[key].first.second==command[1])
            {
                check=true;
                ans+=key+'\n';
            }
        }
        if(!check)
        return "No files uploaded in this group.";
        return ans;
    }
    else
    if(command[0]=="upload_file"&&command.size()==6)// upload_file <filepath> <gid> <uid> <ip> <port>
    {
        if(group.find(command[2])==group.end())
        return "Group does not exist.";
        else
        if(group[command[2]].find(command[3])==group[command[2]].end())
        return "You are not a part of this group,Please joing before uploading this file.";
        else
        {
            string filepath=command[1];
            string filename="";
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
            return "Incorrect Filename/path.";
            file_details[filename].first.first=filename;
            file_details[filename].first.second=command[2];
            file_details[filename].second.first=command[4];
            file_details[filename].second.second=command[5];
            cout<<"<UPLOAD> IP:PORT"+command[4]+":"+command[5]<<endl;
            return "File:"+filename+" uploaded successfully to Group:"+command[2];
        }
    }
    else
    if(command[0]=="download_file"&&command.size()==5)
    {
        if(group.find(command[1])==group.end())
        return "Group does not exist.";
        if(group[command[1]].find(command[4])==group[command[1]].end())
        return "You do not belong to Group:"+command[1];
        if(file_details.find(command[2])==file_details.end())
        return "File not found.";
        if(file_details[command[2]].first.second!=command[1])
        return "File does not belong to Group:"+command[1];
        return file_details[command[2]].second.first+":"+file_details[command[2]].second.second;
    }
    else
    if(command[0]=="logout"&&command.size()==2)
    {
        user[command[1]].second.second=false;
        return "Logged out successfully!";    
    }
    else
    if(command[0]=="show_downloads"&&command.size()==1)
    {}
    else
    if(command[0]=="stop_share"&&command.size()==3)
    {}
    else
    {
        return "Invalid Command";
    }
    return "Invalid Command";
}
void* execute_commands(void* client_socket)
{
    char buffer[1024];
    int client_fd=*(int *)client_socket;
    // free(client_socket);
    while(1)
    {
        bzero(buffer,1024);
        int n=read(client_fd,buffer,1024);
        cout<<"Msg from client:"<<buffer<<":END"<<endl;
        if(strcmp(buffer,"close")==0)
        {
            // exit(0);
            break;
        }
        vector<string> command;
        string temp="";
        for(int i=0;i<strlen(buffer);i++)
        {
            if(buffer[i]==' ')
            {
                command.push_back(temp);
                temp="";
                continue;
            }
            temp+=buffer[i];
        }
        command.push_back(temp);
        bzero(buffer,1000);
        strcpy(buffer,process_command(command).c_str());
        n=write(client_fd,buffer,1000);
     if (n < 0) perror("Error while writing to socket");
    }
}
void print_trackers(vector<pair<string,string>> trackers)
{
    for(int i=0;i<trackers.size();i++)
    cout<<trackers[i].first<<":"<<trackers[i].second<<endl;
}
int main(int argc,char *argv[])
{
    if(argc!=2)
    {
        perror("Please provide 'tracker_info.txt'\n");
        exit(0);
    }
    
    /* Extracting IP and port from tracker_info.txt */
    fstream file;
    string word;
    file.open(argv[1]);
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
    print_trackers(trackers);


    /* Establishing connection */
    int port=atoi(trackers[0].second.c_str());
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
    serv_addr.sin_port=htons(port);
    if(bind(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr))<0) //Binding
    {
        perror("Error while binding.\n");
        exit(0);
    }
    listen(sockfd,8); //Listening
    socklen_t client_len=sizeof(cli_addr);

    //Accepting requests from clients and executing them parallely
    while(1)
    {
        int newsockfd = accept(sockfd,(struct sockaddr *) &cli_addr,&client_len); //Accepting
        if(sockfd<0)
        {
            perror("Error while accepting.\n");
            exit(0);
        }
        pthread_t client_thread;
        int client_socket=newsockfd;
        pthread_create(&client_thread,NULL,&execute_commands,&client_socket);
    }
    return 0;
}