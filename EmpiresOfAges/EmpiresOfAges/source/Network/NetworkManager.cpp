// NetworkManager.cpp

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
	// m_logger, ba�lat�c� listesinde ba�lat�labilir, ancak burada da varsay�lan atanabilir.
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
	if (m_role != NetworkRole::None)
	{
		m_logger("Network durduruldu.");
	}

	if (m_server)
	{
		m_server->stop();
		delete m_server;
		m_server = nullptr;
	}

	if (m_client)
	{
		m_client->disconnect();
		delete m_client;
		m_client = nullptr;
	}

	// Hata C2065: m_discovery ��z�m�
	if (m_discovery)
	{
		m_discovery->stop();
	}

	m_role = NetworkRole::None; // Hata C2065: m_role ��z�m�
}

bool NetworkManager::startServer(unsigned short gamePort)
{
	stop();
	m_server = new NetServer();
	if (m_server->start(gamePort))
	{
		m_role = NetworkRole::Server;
		m_logger("Sunucu basariyla baslatildi. Port: " + std::to_string(gamePort));
		return true;
	}
	else
	{
		m_logger("HATA: Sunucu baslatilamadi.");
		stop();
		return false;
	}
}

bool NetworkManager::startClient(const std::string& addr, unsigned short port)
{
	// startClient metodu 2 arg�man al�r: IP adresi ve port (main.cpp'deki C2660 hatas� ��z�m�)
	if (m_client) delete m_client;

	m_client = new NetClient();
	if (m_client->connect(addr, port))
	{
		m_role = NetworkRole::Client;
		m_logger("Istemci baglantiya hazir: " + addr + ":" + std::to_string(port));
		return true;
	}
	else
	{
		m_logger("HATA: Istemci baglantiya hazirlanamadi.");
		delete m_client;
		m_client = nullptr;
		return false;
	}
}

bool NetworkManager::startServerWithDiscovery(unsigned short gamePort, unsigned short discoveryPort, const std::string& serverName)
{
	// 1. Temel Sunucuyu Ba�lat (Oyun Portunda)
	if (!startServer(gamePort))
	{
		return false;
	}

	// 2. Ke�if Dinleyicisini Ba�lat
	if (m_discovery) // Hata C2065: m_discovery ��z�m�
	{

		if (!m_discovery->startServer(gamePort, discoveryPort, serverName))
		{
			m_logger("LAN Discovery sunucu dinleyicisi baslatilamadi."); // Hata C3861: m_logger ��z�m�
			stop();
			return false;
		}
		m_logger("LAN Discovery dinlemesi baslatildi. Kesif Portu: " + std::to_string(discoveryPort));
	}

	return true;
}

bool NetworkManager::startClientWithDiscovery(unsigned short discoveryPort)
{
	stop();

	m_role = NetworkRole::Client; // Hata C2065: m_role ��z�m�

	if (m_discovery) // Hata C2065: m_discovery ��z�m�
	{

		if (!m_discovery->startClient(discoveryPort))
		{
			m_logger("LAN Discovery istemci aramasi baslatilamadi.");
			m_role = NetworkRole::None;
			return false;
		}
	}

	m_logger("LAN Discovery istemcisi baslatildi. Arama Portu: " + std::to_string(discoveryPort));
	return true;
}

void NetworkManager::update(float DeltaSeconds)
{
	if (m_server)
	{
		m_server->update();
	}

	if (m_client)
	{
		m_client->update(DeltaSeconds);
	}

	if (m_discovery)
	{
		m_discovery->update();
	}
}

void NetworkManager::setLogger(LogFn logger)
{
	m_logger = std::move(logger); // Hata C3861: m_logger ��z�m�
}