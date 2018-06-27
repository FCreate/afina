//
// Created by micheal on 21.06.18.
//

#ifndef AFINA_CONNECTION_CPP
#define AFINA_CONNECTION_CPP
#include <iostream>
#include <sys/socket.h>
#include <cstring>
#include <unistd.h>


#include <afina/Storage.h>
#include <protocol/Parser.h>
#include <afina/execute/Command.h>

namespace Afina {
    namespace Network {
        namespace NonBlocking {
            class Connection {
            public:
                enum class State {
                    // Connection could executed
                            Run,

                    // Tasks are shutting down
                            Stopping
                };
                Connection() {
                };
                Connection(std::shared_ptr<Afina::Storage> ps, int sock): pStorage(ps), socket(sock) {
                    //Initialze parser - constructor will be run
                    parser = Protocol::Parser();
                    //Make the flag is_parsed false
                    is_parsed = false;
                    //State should be swithced to Run
                    cState = State::Run;
                };
                //Destructor is simple
                ~Connection(){};
                //Handler function. Here data is processed.
                void handler() {
                    //Initialize buf_size 1024
                    auto buf_size = 1024;
                    //Make the array, named buffer. Size is buf_size
                    char buffer[buf_size];
                    //Create out string. These string we send to user.
                    std::string out;
                    //How many bites are parsed.
                    size_t parsed = 0;
                    //Current position
                    size_t curr_pos = 0;
                    //How much data we received
                    ssize_t n_read = 0;
                    //Body size for Parser Build
                    uint32_t body_size = 0;

                    // Receive data from current socket
                            //These calls return the number of bytes received, or -1 if an error
                    //       occurred.  In the event of an error, errno is set to indicate the
                    //       error.
                    //
                    //       When a stream socket peer has performed an orderly shutdown, the
                    //       return value will be 0 (the traditional "end-of-file" return).
                    //
                    //       Datagram sockets in various domains (e.g., the UNIX and Internet
                    //       domains) permit zero-length datagrams.  When such a datagram is
                    //       received, the return value is 0.
                    //
                    //       The value 0 may also be returned if the requested number of bytes to
                    //       receive from a stream socket was 0.recv()
                    //       The recv() call is normally used only on a connected socket (see
                    //       connect(2)).  It is equivalent to the call:
                    //
                    //           recvfrom(fd, buf, len, flags, NULL, 0);

                    n_read = recv(socket, buffer + curr_pos, buf_size - curr_pos, 0);

                    if (n_read == 0){
                        //User disconnected.
                        close(socket);
                        throw std::runtime_error("User disconnected");
                    }
                    if (n_read == -1){
                        //Call return -1, if an error occured. Errrno is set to indicate the error.
                        //Resouce unavailable or would be blocked
                        // The socket is marked nonblocking and the receive operation
                        //              would block, or a receive timeout had been set and the timeout
                        //              expired before data was received.  POSIX.1 allows either error
                        //              to be returned for this case, and does not require these
                        //              constants to have the same value, so a portable application
                        //              should check for both possibilities.
                        if (errno == EAGAIN || errno == EWOULDBLOCK){

                            if (cState == State::Stopping){
                                out = "Server is shutting down.\n";
                                if (send(socket, out.data(), out.size(), 0) <= 0) {
                                    throw std::runtime_error("Socket send() failed\n");
                                }
                                close(socket);
                                return;
                            }
                            return;
                        }else{
                            //EBADF  The argument sockfd is an invalid file descriptor.
                            //
                            //       ECONNREFUSED
                            //              A remote host refused to allow the network connection
                            //              (typically because it is not running the requested service).
                            //
                            //       EFAULT The receive buffer pointer(s) point outside the process's
                            //              address space.
                            //
                            //       EINTR  The receive was interrupted by delivery of a signal before any
                            //              data were available; see signal(7).
                            //
                            //       EINVAL Invalid argument passed.
                            //
                            //       ENOMEM Could not allocate memory for recvmsg().
                            //
                            //       ENOTCONN
                            //              The socket is associated with a connection-oriented protocol
                            //              and has not been connected (see connect(2) and accept(2)).
                            //
                            //       ENOTSOCK
                            //              The file descriptor sockfd does not refer to a socket.
                            //User disconnected in this case
                            close(socket);
                            throw std::runtime_error("User disconnected");
                        }
                    }
                    //On this step all errors was processed and we sholud process another step
                    //We must shift position
                    curr_pos += n_read;
                    while (parsed < curr_pos) {
                        try {
                            //Try parse received expression
                            is_parsed = parser.Parse(buffer, curr_pos, parsed);
                        }catch (std::runtime_error &err) {//If some errors accuired, they will be send to user
                            //We caught error of parsing
                            out = std::string("SERVER_ERROR : ") + err.what() + "\r\n";
                            //Send it
                            if (send(socket, out.data(), out.size(), 0) <= 0) {
                                throw std::runtime_error("Socket send() failed\n");
                            }
                            return;
                        }
                        //If parsing is success, then we must process it
                        if (is_parsed) {
                            //body_read - how many data we read
                            size_t body_read = curr_pos - parsed;
                            //Copy body_read elements
                            memcpy(buffer, buffer + parsed, body_read);
                            //Set another bytes to 0
                            memset(buffer + body_read, 0, parsed);
                            //Set curr_pos to body_read
                            curr_pos = body_read;
                            //Build expression, body_size init in Build method, this bytes are blocked
                            auto cmd = parser.Build(body_size);

                            // Try read command. If command read it will be execute
                            if (body_size <= curr_pos) {
                                char args[body_size + 1];
                                memcpy(args, buffer, body_size);
                                args[body_size] = '\0';
                                //We must count rn
                                if (body_size) {
                                    memcpy(buffer, buffer + body_size + 2, curr_pos - body_size - 2);
                                    memset(buffer + curr_pos - body_size - 2, 0, body_size);
                                    curr_pos -= body_size + 2;
                                }
                                //Try execute
                                try {
                                    //Execute this one
                                    cmd->Execute(*(pStorage.get()), args, out);
                                    out += "\r\n";
                                } catch (std::runtime_error &err) {
                                    out = std::string("SERVER_ERROR : ") + err.what() + "\r\n";
                                }
                                //And send it
                                if (send(socket, out.data(), out.size(), 0) <= 0) {
                                    throw std::runtime_error("Socket send() failed\n");
                                }
                                //Reset parser
                                parser.Reset();
                                is_parsed = false;
                                parsed = 0;
                            }
                        }



                    }
                    if (cState == State::Stopping){
                        out = "Server is shutting down.\n";
                        if (send(socket, out.data(), out.size(), 0) <= 0) {
                            throw std::runtime_error("Socket send() failed\n");
                        }
                        close(socket);
                    }
                };
                //We must cState publice to avoid
                State cState = State::Run;
            private:
                std::shared_ptr<Afina::Storage> pStorage;
                Protocol::Parser parser;
                int socket;
                bool is_parsed = false;
            };

        }
    }
}
#endif //AFINA_CONNECTION_CPP