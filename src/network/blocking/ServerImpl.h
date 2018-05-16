#ifndef AFINA_NETWORK_BLOCKING_SERVER_H
#define AFINA_NETWORK_BLOCKING_SERVER_H

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <pthread.h>
#include <unordered_set>

#include <afina/network/Server.h>
#include "./../../protocol/Parser.h"
#include <afina/execute/Command.h>
#include <afina/Executor.h>

namespace Afina {
namespace Network {
namespace Blocking {

/**
 * # Network resource manager implementation
 * Server that is spawning a separate thread for each connection
 */
class ServerImpl : public Server {
public:
    ServerImpl(std::shared_ptr<Afina::Storage> ps);
    ~ServerImpl();

    // See Server.h
    void Start(uint32_t port, uint16_t workers) override;

    // See Server.h
    void Stop() override;

    // See Server.h
    void Join() override;

protected:
    /**
     * Method is running in the connection acceptor thread
     */
    void RunAcceptor(int socket =0);

    /**
     * Methos is running for each connection
     */
    void RunConnection(int client_socket = 0);

private:
    //Function for pthread_create. pthread_create gets this pointer as parameter
    //and then this function calls RunAcceptor()/RunConnectionProxy
    //Template argument - pointer to function-member, that should be started
    //void* parameter should be ThreadParams structure, created with new (delete will be called by this function)
    // <void (ServerImpl::*function_for_start)(int)>
    static void *RunAcceptorProxy(void *p);
    static void *RunAcceptorProxyConnection(void *p);
    // Atomic flag to notify threads when it is time to stop. Note that
    // flag must be atomic in order to safely publisj changes cross thread
    // bounds
    std::atomic<bool> running;

    // Thread that is accepting new connections
    pthread_t accept_thread;

    // Maximum number of client allowed to exists concurrently
    // on server, permits access only from inside of accept_thread.
    // Read-only
    uint16_t max_workers;

    // Port to listen for new connections, permits access only from
    // inside of accept_thread
    // Read-only
    uint32_t listen_port;

    // Mutex used to access connections list
    std::mutex connections_mutex;

    // Conditional variable used to notify waiters about empty
    // connections list
    std::condition_variable connections_cv;

    // Threads that are processing connection data, permits
    // access only from inside of accept_thread
    std::unordered_set<pthread_t> connections;

    int server_socket;
    std::atomic<bool> finished;
    std::unordered_set<int> client_sockets;
    Executor executor;

};

} // namespace Blocking
} // namespace Network
} // namespace Afina

#endif // AFINA_NETWORK_BLOCKING_SERVER_H
