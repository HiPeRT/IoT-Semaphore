#ifndef SEND_H
#define SEND_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h> //socket
#include <arpa/inet.h>  //inet_addr
#include <unistd.h>     //write

#include "serialize.hpp"

//#define SOCKET_MODE SOCK_DGRAM //UDP
//#define SOCKET_MODE SOCK_STREAM //TCP

class Communicator{

    public: 
    static const int message_size = 50000;
    int sock;
    int port; 
    int socket_opened = 0;
    std::string ip;
    char client_message[message_size];
    struct sockaddr_in server;
    int SOCKET_MODE;

    Communicator(){
        SOCKET_MODE = SOCK_DGRAM;
    }

    Communicator(int operation_mode)
    {
        SOCKET_MODE = operation_mode;
    }

    int get_socket(){
        return sock;
    }

    void serialize_coords(Message *m, std::stringbuf* buf)
    {
        std::ostream os(buf);
        cereal::PortableBinaryOutputArchive archive(os);
        archive(*m);
    }

    void deserialize_coords(char *buffer, Message *m)
    {
        std::stringbuf buf(buffer);
        std::istream is(&buf);
        cereal::PortableBinaryInputArchive retrieve(is);
        retrieve(*m);
    }

    void deserialize_coords(std::string s, Message *m)
    {
        std::istringstream is(s);
        cereal::PortableBinaryInputArchive retrieve(is);
        try 
        {
            retrieve(*m);
        }
        catch (std::bad_alloc& ba)
        {
            std::cout << "Packet drop"<<std::endl; 
        }
    }
    

    int open_client_socket(char *ip, int port)
    {
        sock = socket(AF_INET, SOCKET_MODE, 0);
        if (sock == -1)
        {
            printf("Could not create socket");
        }
        puts("Socket created");

        this->ip = std::string(ip);
        this->port = port;
        server.sin_addr.s_addr = inet_addr(ip);
        server.sin_family = AF_INET;
        server.sin_port = htons(port);

        if (SOCKET_MODE == SOCK_DGRAM)
        {
            socket_opened = 1;
            return 1;
        }

        /*connect to remote server*/
        if (connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0)
        {
            perror("connect failed. Error");
            socket_opened = 0;
            return 0;
        }
        puts("Connected\n");
        socket_opened = 1;
        return 1;
    }

    int open_server_socket(int port)
    {
        sock = socket(AF_INET, SOCKET_MODE, 0);
        if (sock == -1)
        {
            printf("Could not create socket");
        }
        puts("Socket created");

        this->port = port;
        //Prepare the sockaddr_in structure
        server.sin_family = AF_INET;
        server.sin_addr.s_addr = INADDR_ANY;
        server.sin_port = htons(port);
        //Bind
        if (bind(sock, (struct sockaddr *)&server, sizeof(server)) < 0)
        {
            //print the error message
            perror("bind failed. Error");
            return 1;
        }
        puts("bind done");

        if (SOCKET_MODE == SOCK_DGRAM)
        {            
            return 1;
        }

        //Listen
        listen(sock, 3);
        return 1;
    }

    int send_message(Message *m)
    {
        /*serialize coords*/
        std::stringbuf *message = new std::stringbuf();
        serialize_coords(m, message);

        if (SOCKET_MODE == SOCK_STREAM)
        {
            /*open socket if not already opened*/
            if (socket_opened == 0)
            {
                std::vector<char> cstr(ip.c_str(), ip.c_str() + ip.size() + 1);
                int res = open_client_socket(cstr.data(), port);
                if(res)
                    printf("Socket opened!\n");
                else
                {
                    printf("Problem: socket NOT opened!\n");
                    return 0;
                }
            }


            /*send message to server*/
            if (send(sock, message->str().data(), message->str().length(), MSG_NOSIGNAL) < 0)
            {
                puts("Send failed");
                socket_opened = 0;
            }
        }
        else
        {
             sendto(sock, message->str().data(), message->str().length(), 0,
                (const struct sockaddr *) &server, sizeof(server));
        }
        

        delete message;
        return 1;
    }

    int receive_message(int socket_desc, Message *m)
    {
        int read_size;
        memset(client_message, 0, message_size);

        if (SOCKET_MODE == SOCK_STREAM)
        {
            while ((read_size = recv(socket_desc, client_message, message_size, MSG_NOSIGNAL)) > 0)
            {
                std::string s((char *)client_message, message_size);
                deserialize_coords(s, m);
                return 0;
            }

            if (read_size == 0)
            {
                puts("Client disconnected");
                fflush(stdout);
                return 1;
            }
            else if (read_size == -1)
            {
                perror("recv failed");
                return 1;
            }
        }
        else
        {
            int len;
            struct sockaddr_in cliaddr; 
            read_size = recvfrom(sock, client_message, message_size, 0,
                    ( struct sockaddr *) &cliaddr, (socklen_t *)&len);
            std::string s((char *)client_message, message_size);
            deserialize_coords(s, m);
            return 0;
        }
        
        return 1;
    }

};



#endif /*SEND_H*/
