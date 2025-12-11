// NetClient.cpp

#include "Network/NetClient.h"
#include <iostream>
#include <thread>
#include <chrono>

// --- Kurucu ve Y�k�c� ---

NetClient::NetClient()
    : m_serverPort(0), m_connected(false)
{
    // Soketi engellemeyen (non-blocking) moda ayarl�yoruz.
    m_socket.setBlocking(false);
}

NetClient::~NetClient() {
    disconnect();
}

// --- A� Kontrol� ---

bool NetClient::connect(const std::string& addr, unsigned short port) {
    if (isConnected()) {
        disconnect();
    }

    // 1. Sunucu adresini ve portunu kaydet
    m_serverAddress = sf::IpAddress(addr);
    m_serverPort = port;

    // 2. Kendi soketimizi rasgele bir porta ba�la (bind)
    // UDP istemcisinin bir portu dinlemesi gerekir. 0 verirsek rastgele port se�er.
    if (m_socket.bind(sf::Socket::AnyPort) != sf::Socket::Done) {
        std::cerr << "Hata: �stemci soketi rasgele bir porta ba�lanamad�." << std::endl;
        m_serverPort = 0;
        return false;
    }

    // 3. Ba�ar�l� oldu
    m_connected = true;
    std::cout << "�stemci ba�ar�yla ba�lat�ld� ve sunucu adresi kaydedildi: "
        << addr << ":" << port << std::endl;

    // Ba�lant� callback'ini �a��r (UDP'de bu, ileti�im ba�latmaya haz�r oldu�umuz anlam�na gelir)
    if (m_onConnectedCallback) {
        m_onConnectedCallback();
    }

    // Not: Ger�ek ba�lant� kontrol� (Sunucunun hayatta olup olmad���) ilk 'send' i�leminden sonra anla��l�r.
    return true;
}

void NetClient::disconnect() {
    if (m_connected) {
        m_socket.unbind();
        m_serverPort = 0;
        m_connected = false;

        std::cout << "�stemci ba�lant�s� kesildi." << std::endl;

        if (m_onDisconnectedCallback) {
            m_onDisconnectedCallback();
        }
    }
}

// --- Ana G�ncelleme D�ng�s� ---

void NetClient::update(float dt) {
    if (!m_connected) return;

    sf::Packet receivedPacket;
    sf::IpAddress senderAddress;
    unsigned short senderPort;

    // T�m mevcut paketleri al
    while (m_socket.receive(receivedPacket, senderAddress, senderPort) == sf::Socket::Done) {

        // Sadece bekledi�imiz sunucudan geliyorsa i�le
        if (senderAddress == m_serverAddress && senderPort == m_serverPort) {
            handleIncomingPacket(receivedPacket);
        }
        else {
            // Bilinmeyen bir adresten gelen paketi yok say (g�venlik)
            std::cerr << "Uyar�: Bilinmeyen adresten paket al�nd�: "
                << senderAddress.toString() << ":" << senderPort << std::endl;
        }

        receivedPacket.clear(); // Paketi bir sonraki al�m i�in temizle
    }

    // Not: Bu k�sma Keep-Alive/Timeout kontrol� eklenebilir.
}

// --- Gelen Paket ��leme ---

void NetClient::handleIncomingPacket(sf::Packet& packet) {
    if (m_onPacketCallback) {
        // Callback'e paketi ilet. Oyun mant��� burada veriyi okuyacakt�r.
        m_onPacketCallback(packet);
    }
    // �rn: sf::Int32 commandType; if (packet >> commandType) { ... }
}


// --- Veri G�nderme ---

bool NetClient::send(sf::Packet& pkt) {
    if (!m_connected) return false;

    // UDP soketi �zerinden, kay�tl� sunucu adresine ve portuna g�nder.
    sf::Socket::Status status = m_socket.send(pkt, m_serverAddress, m_serverPort);

    return status == sf::Socket::Done;
}

// --- Yard�mc� Fonksiyonlar ve Callback'ler ---

bool NetClient::isConnected() const {
    return m_connected;
}

void NetClient::setOnPacket(OnPacketFn cb) {
    m_onPacketCallback = std::move(cb);
}

void NetClient::setOnConnected(std::function<void()> cb) {
    m_onConnectedCallback = std::move(cb);
}

void NetClient::setOnDisconnected(std::function<void()> cb) {
    m_onDisconnectedCallback = std::move(cb);
}