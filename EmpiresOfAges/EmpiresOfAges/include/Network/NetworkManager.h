// NetworkManager.h
#pragma once

#include <functional>
#include <string>
#include "LANDiscovery.h" 

// �leri Bildirimler (Forward Declarations)
// Header dosyas�nda s�n�f� tan�mlamadan �nce pointer olarak kullanmak i�in
class NetServer;
class NetClient;

using LogFn = std::function<void(const std::string&)>;
enum class NetworkRole { None, Server, Client };

class NetworkManager {
public:
    NetworkManager();
    ~NetworkManager();

    // Temel A� Metotlar� (main.cpp ve NetworkManager.cpp'de �a�r�lanlar)
    bool startServer(unsigned short gamePort);
    bool startClient(const std::string& addr, unsigned short port);

    bool startServerWithDiscovery(unsigned short gamePort, unsigned short discoveryPort, const std::string& serverName); // Hata C2039 ��z�m�
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
