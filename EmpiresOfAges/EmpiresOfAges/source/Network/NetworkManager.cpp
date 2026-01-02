#include "Network/NetworkManager.h"
#include "Network/NetServer.h" 
#include "Network/NetClient.h" 
#include "Network/LANDiscovery.h" 
#include <iostream>

NetworkManager::NetworkManager()
    : m_role(NetworkRole::None),
    m_server(nullptr),
    m_client(nullptr),
    m_discovery(new LANDiscovery())
{
    if (!m_logger) {
        m_logger = [](const std::string& msg) { std::cout << "[NetworkManager LOG] " << msg << std::endl; };
    }
}

NetworkManager::~NetworkManager()
{
    stop();
    delete m_discovery;
    m_discovery = nullptr;
}

void NetworkManager::stop()
{
    if (m_role != NetworkRole::None) {
        m_logger("Network stopped.");
    }

    if (m_server) {
        m_server->stop();
        delete m_server;
        m_server = nullptr;
    }

    if (m_client) {
        m_client->disconnect();
        delete m_client;
        m_client = nullptr;
    }

    if (m_discovery) {
        m_discovery->stop();
    }

    m_role = NetworkRole::None;
}

bool NetworkManager::startServer(unsigned short gamePort)
{
    stop();
    m_server = new NetServer();
    if (m_server->start(gamePort)) {
        m_role = NetworkRole::Server;
        m_logger("Server started successfully. Port: " + std::to_string(gamePort));
        return true;
    }
    else {
        m_logger("ERROR: Could not start server.");
        stop();
        return false;
    }
}

bool NetworkManager::startClient(const std::string& addr, unsigned short port)
{
    if (m_client) delete m_client;

    m_client = new NetClient();
    if (m_client->connect(addr, port)) {
        m_role = NetworkRole::Client;
        m_logger("Client ready to connect: " + addr + ":" + std::to_string(port));
        return true;
    }
    else {
        m_logger("ERROR: Client could not prepare connection.");
        delete m_client;
        m_client = nullptr;
        return false;
    }
}

bool NetworkManager::startServerWithDiscovery(unsigned short gamePort, unsigned short discoveryPort, const std::string& serverName)
{
    if (!startServer(gamePort)) {
        return false;
    }

    if (m_discovery) {
        if (!m_discovery->startServer(gamePort, discoveryPort, serverName)) {
            m_logger("LAN Discovery server listener could not be started.");
            stop();
            return false;
        }
        m_logger("LAN Discovery started. Discovery Port: " + std::to_string(discoveryPort));
    }

    return true;
}

bool NetworkManager::startClientWithDiscovery(unsigned short discoveryPort)
{
    stop();
    m_role = NetworkRole::Client;

    if (m_discovery) {
        if (!m_discovery->startClient(discoveryPort)) {
            m_logger("LAN Discovery client search could not be started.");
            m_role = NetworkRole::None;
            return false;
        }
    }

    m_logger("LAN Discovery client started. Search Port: " + std::to_string(discoveryPort));
    return true;
}

void NetworkManager::update(float DeltaSeconds)
{
    if (m_server) {
        m_server->update();
    }

    if (m_client) {
        m_client->update(DeltaSeconds);
    }

    if (m_discovery) {
        m_discovery->update();
    }
}

void NetworkManager::setLogger(LogFn logger)
{
    m_logger = std::move(logger);
}