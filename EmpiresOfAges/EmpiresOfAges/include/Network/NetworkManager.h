#pragma once

#include <functional>
#include <string>
#include "LANDiscovery.h" 

class NetServer;
class NetClient;

using LogFn = std::function<void(const std::string&)>;
enum class NetworkRole { None, Server, Client };

/**
 * @brief High-level manager for the networking system.
 * Handles the creation and lifecycle of Server, Client, and LAN Discovery modules.
 */
class NetworkManager {
public:
    NetworkManager();
    ~NetworkManager();

    bool startServer(unsigned short gamePort);
    bool startClient(const std::string& addr, unsigned short port);

    bool startServerWithDiscovery(unsigned short gamePort, unsigned short discoveryPort, const std::string& serverName);
    bool startClientWithDiscovery(unsigned short discoveryPort);

    void stop();
    void update(float DeltaSeconds);
    void setLogger(LogFn logger);

    NetServer* server() const { return m_server; }
    NetClient* client() const { return m_client; }
    LANDiscovery* discovery() const { return m_discovery; }

private:
    NetworkRole m_role;
    LogFn m_logger;

    NetServer* m_server;
    NetClient* m_client;
    LANDiscovery* m_discovery;
};