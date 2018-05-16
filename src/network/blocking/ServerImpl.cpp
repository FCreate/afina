#include "ServerImpl.h"

#include <cassert>
#include <cstring>
#include <iostream>
#include <memory>
#include <stdexcept>

#include <pthread.h>
#include <signal.h>

#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>

#include <afina/Storage.h>


#define NETWORK_DEBUG(X) std::cout << "network debug: " << X << std::endl
#define NETWORK_PROCESS_DEBUG(PID, MESSAGE) NETWORK_DEBUG("Process PID = " << PID << ": " << MESSAGE)
#define NETWORK_PROCESS_MESSAGE(MESSAGE) std::cout << "Process PID = " << pthread_self() << ": " << MESSAGE
#define LOCK_CONNECTIONS_MUTEX std::lock_guard<std::mutex> lock(connections_mutex)

#define READING_PORTION 1024

namespace Afina {
namespace Network {
namespace Blocking {
//run acceptor for connections
void *ServerImpl::RunAcceptorProxy(void *p) {
    ServerImpl *srv = reinterpret_cast<ServerImpl *>(p);
    try {
        srv->RunAcceptor();
    } catch (std::runtime_error &ex) {
        std::cerr << "Acceptor proxy fails: " << ex.what() << std::endl;
    }
    return 0;
}
//run connections
void *ServerImpl::RunAcceptorProxyConnection(void *p){
    ServerImpl *srv;
    int socket;
    std::tie(srv, socket) = *reinterpret_cast<std::pair<ServerImpl*, int>*>(p);
    try {
        srv->RunConnection(socket);
    } catch (std::runtime_error &ex) {
        std::cerr << "Acceptor proxy connection fails: " << ex.what() << std::endl;
    }
    return 0;
}
// See Server.h
ServerImpl::ServerImpl(std::shared_ptr<Afina::Storage> ps) :
        Server(ps), server_socket(-1), running(false), finished(false), max_workers(5), listen_port(0) {}

// See Server.h
ServerImpl::~ServerImpl() {
        Stop();//correctly stop work
    }

// See Server.h
void ServerImpl::Start(uint32_t port, uint16_t n_workers) {
    std::cout << "network debug: " << __PRETTY_FUNCTION__ << std::endl;

    // If a client closes a connection, this will generally produce a SIGPIPE
    // signal that will kill the process. We want to ignore this signal, so send()
    // just returns -1 when this happens.
    sigset_t sig_mask;
    sigemptyset(&sig_mask);
    sigaddset(&sig_mask, SIGPIPE);
    if (pthread_sigmask(SIG_BLOCK, &sig_mask, NULL) != 0) {
        throw std::runtime_error("Unable to mask SIGPIPE");
    }

    // Setup server parameters BEFORE thread created, that will guarantee
    // variable value visibility
    max_workers = n_workers;
    listen_port = port;

    // The pthread_create function creates a new thread.
    //
    // The first parameter is a pointer to a pthread_t variable, which we can use
    // in the remainder of the program to manage this thread.
    //
    // The second parameter is used to specify the attributes of this new thread
    // (e.g., its stack size). We can leave it NULL here.
    //
    // The third parameter is the function this thread will run. This function *must*
    // have the following prototype:
    //    void *f(void *args);
    //
    // Note how the function expects a single parameter of type void*. We are using it to
    // pass this pointer in order to proxy call to the class member function. The fourth
    // parameter to pthread_create is used to specify this parameter value.
    //
    // The thread we are creating here is the "server thread", which will be
    // responsible for listening on port 23300 for incoming connections. This thread,
    // in turn, will spawn threads to service each incoming connection, allowing
    // multiple clients to connect simultaneously.
    // Note that, in this particular example, creating a "server thread" is redundant,
    // since there will only be one server thread, and the program's main thread (the
    // one running main()) could fulfill this purpose.

    //At this point executor start
    running.store(true);
    //if (pthread_create(&accept_thread, NULL, ServerImpl::RunAcceptorProxy, this)<0) {
    //    throw std::runtime_error("Could not create server thread");
    //}
    executor.Execute(ServerImpl::RunAcceptorProxy, this);
}

// See Server.h
void ServerImpl::Stop() {
    std::cout << "network debug: " << __PRETTY_FUNCTION__ << std::endl;
    running.store(false);
    shutdown(server_socket, SHUT_RDWR);
    executor.Stop();
    /*if (!running.load() || finished.load()) { return; }
    finished.store(true);*/
    // Shutdown listening socket and all client sockets
    // sockets will be closed by their threads
    /*Correcly shutdown listening sockets
     * and client sockets. They must be closed by
     * their threads*/
    /*{
        LOCK_CONNECTIONS_MUTEX;
        for (auto it = client_sockets.begin(); it != client_sockets.end(); it++) {
            shutdown(*it, SHUT_RDWR);
        }
    }

    while (!connections.empty()) {
        std::unordered_set<pthread_t>::iterator first_thread;
        {
            LOCK_CONNECTIONS_MUTEX;
            first_thread = connections.begin();
            if (first_thread == connections.end()) { break; }
        }
        pthread_join(*first_thread, nullptr); //block thread until the thread completes
    }
    //Shut down main thread
    shutdown(server_socket, SHUT_RDWR);
    pthread_join(accept_thread, 0);
    //and make false all atomic and condition variables
    running.store(false);
    finished.store(false);
    connections_cv.notify_all();*/
}

// See Server.h
void ServerImpl::Join() {
    std::cout << "network debug: " << __PRETTY_FUNCTION__ << std::endl;
    pthread_join(accept_thread, 0);
    executor.Join();
    /*std::cout << "network debug: " << __PRETTY_FUNCTION__ << std::endl;
    pthread_join(accept_thread, 0);
    executor.Join();*/
}

// See Server.h
void ServerImpl::RunAcceptor(int) {
    std::cout << "network debug: " << __PRETTY_FUNCTION__ << std::endl;

    // For IPv4 we use struct sockaddr_in:
    // struct sockaddr_in {
    //     short int          sin_family;  // Address family, AF_INET
    //     unsigned short int sin_port;    // Port number
    //     struct in_addr     sin_addr;    // Internet address
    //     unsigned char      sin_zero[8]; // Same size as struct sockaddr
    // };
    //
    // Note we need to convert the port to network order
    sockaddr_in server_addr = {};
    std::memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;          // IPv4
    server_addr.sin_port = htons(listen_port); // TCP port number
    server_addr.sin_addr.s_addr = INADDR_ANY;  // Bind to any address
    // Arguments are:
    // - Family: IPv4
    // - Type: Full-duplex stream (reliable)
    // - Protocol: TCP
    server_socket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (server_socket == -1) {
        throw std::runtime_error("Failed to open socket");
    }
    // when the server closes the socket,the connection must stay in the TIME_WAIT state to
    // make sure the client received the acknowledgement that the connection has been terminated.
    // During this time, this port is unavailable to other processes, unless we specify this option
    //
    // This option let kernel knows that we are OK that multiple threads/processes are listen on the
    // same port. In a such case kernel will balance input traffic between all listeners (except those who
    // are closed already)
    int opts = 1;
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opts, sizeof(opts)) == -1) {
        close(server_socket);
        throw std::runtime_error("Socket setsockopt() failed");
    }

    // Bind the socket to the address. In other words let kernel know data for what address we'd
    // like to see in the socket
    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        close(server_socket);
        throw std::runtime_error("Socket bind() failed");
    }

    // Start listening. The second parameter is the "backlog", or the maximum number of
    // connections that we'll allow to queue up. Note that listen() doesn't block until
    // incoming connections arrive. It just makesthe OS aware that this process is willing
    // to accept connections on this socket (which is bound to a specific IP and port)
    if (listen(server_socket, 5) == -1) {
        close(server_socket);
        throw std::runtime_error("Socket listen() failed");
    }

    int client_socket = -1;
    sockaddr_in client_addr = {};
    std::memset(&client_addr, 0, sizeof(client_addr));
    socklen_t sinSize = sizeof(sockaddr_in);
    while (running.load()) {
        std::cout << "DEBUG Listen incoming connection" << std::endl;

        //Accept incoming connections, and block it, until the task haven't done
        //Accept function do it
        client_socket = accept(server_socket, (sockaddr *) &client_addr, &sinSize);
        if (client_socket == -1) {
            if (finished.load()) { break; } //Incoming connection is finished
            close(server_socket);
            throw std::runtime_error("Socket accept() failed.");
        }
        std::cout<<"Connection accepted"<<std::endl;
        //Check max_workers limit
        std::cout<<"Max workers  "<<max_workers<<std::endl;
        /*if (connections.size() >= max_workers) {
            std::string message = "ERROR ON SERVER There is max_workers limit is achieved";
            if (send(client_socket, message.data(), message.size(), 0) <= 0) {
                close(client_socket); //Closes only client socket
            }
            if(!executor.Execute(ServerImpl::RunAcceptorProxyConnection, &args)){
                close(server_socket);
                close(client_socket);
                throw std::runtime_error("Could not create connection thread");
                //std::cout<<"Max workers limit has been achieved"<<std::endl;
                continue;
        }*/

        //Create new thread for connection
        {
            LOCK_CONNECTIONS_MUTEX;
            auto args = std::make_pair(this, client_socket);
            if(!executor.Execute(ServerImpl::RunAcceptorProxyConnection, &args)) {
                close(server_socket);
                close(client_socket);
                throw std::runtime_error("Could not create connection thread");
            }
            else {
                std::cout << "SUCCESSFUL CONNECTION" << std::endl;
            }

            //connections.insert(client_thread);
            client_sockets.insert(client_socket);
        }
    }

    // Cleanup on Fxit...
    close(server_socket);
}

// See Server.h
void ServerImpl::RunConnection(int client_socket) {
    std::cout << "network debug: " << __PRETTY_FUNCTION__ << std::endl;
    // TODO: All connection work is here
    Afina::Protocol::Parser parser;
    std::string prev_string;
    while(running.load()) {
        char new_string[READING_PORTION];
        if (recv(client_socket, new_string, READING_PORTION * sizeof(char), 0) <= 0) {
            break;
        }
        prev_string.append(new_string);
        memset(new_string, 0, READING_PORTION * sizeof(char));
        size_t parsed = 0;
        bool command_bool = false;
        try {
            command_bool = parser.Parse(prev_string, parsed);
        }
        catch (std::exception &e) {
            std::string message_error = "Parsing error: ";
            message_error += e.what();
            message_error += "\r\n";
            send(client_socket, message_error.c_str(), message_error.size(), 0);
            prev_string = "";
            parser.Reset();
            continue;
        }
        prev_string = prev_string.substr(parsed);
        if (!command_bool) {
            continue;
        }
        
        uint32_t temp_arg = 0;
        auto command = parser.Build(temp_arg);
        if (temp_arg != 0) {
            temp_arg += 2;//\r\n
        }
        //Take args
        if (temp_arg > prev_string.size()) {
            if (recv(client_socket, new_string, temp_arg * sizeof(char), MSG_WAITALL) <= 0) {
                NETWORK_PROCESS_MESSAGE("There is some errors while get args from socket");
                break;
            }
            prev_string.append(new_string);
        }
        std::string take_args;
        if (temp_arg > 2) {
            take_args = prev_string.substr(0, temp_arg - 2);
            prev_string = prev_string.substr(temp_arg);
        }
        std::string server_out;
        
        try {
            command->Execute(*pStorage, take_args, server_out);
        }
        catch (std::exception &e) {
            server_out = "ERROR ON SERVER";
            server_out += e.what();
        }
        server_out += "\r\n";
        //Send message to client
        size_t len_sended = send(client_socket, server_out.c_str(), server_out.size(), 0);
        if (len_sended < server_out.size()) {
            NETWORK_PROCESS_MESSAGE("Server don't sent all data");
            break;
        }
        parser.Reset();
    }

    NETWORK_PROCESS_DEBUG(pthread_self(), "This process will be finished");
    close(client_socket);
}

} // namespace Blocking
} // namespace Network
} // namespace Afina
