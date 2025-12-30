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
    std::cout <<    "�stemci ba�ar�yla ba�lat�ld� ve sunucu adresi kaydedildi: "
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

    for (auto& pair : m_pendingPackets) {
        // Eğer paket gönderileli 200ms geçmişse ve hala onay gelmemişse tekrar gönder
        if (pair.second.timer.getElapsedTime().asMilliseconds() > 200) {
            m_socket.send(pair.second.packet, m_serverAddress, m_serverPort);
            pair.second.timer.restart(); // Sayacı sıfırla ki bir 200ms daha beklesin
        }
    }
}

// --- Gelen Paket ��leme ---

void NetClient::handleIncomingPacket(sf::Packet& packet) {
    // 1. Paketin tipini oku
    sf::Uint8 typeRaw;
    if (!(packet >> typeRaw)) return; // Boş paket koruması

    PacketType type = static_cast<PacketType>(typeRaw);

    // 2. ACK İşlemi (Sistem mesajı, oyuna gitmez)
    if (type == PacketType::ACK) {
        sf::Uint32 confirmedSeq;
        if (packet >> confirmedSeq) {
            m_pendingPackets.erase(confirmedSeq);
        }
        return;
    }

    // 3. Reliable İşlemi
    if (type == PacketType::Reliable) {
        sf::Uint32 seq;
        if (packet >> seq) {
            // Sunucuya "Aldım" (ACK) gönder
            sf::Packet ackPkt;
            ackPkt << static_cast<sf::Uint8>(PacketType::ACK) << seq;
            m_socket.send(ackPkt, m_serverAddress, m_serverPort);

            // Buradan sonra packet'in imleci payload'ın başındadır.
            // Oyun mantığına devam et.
        }
        else {
            return; // Hatalı paket
        }
    }

    // 4. Unreliable İşlemi
    // (type == Unreliable ise sadece typeRaw okundu, imleç verinin başında, devam et)

    // 5. Oyun Mantığına Teslim Et
    if (m_onPacketCallback) {
        // Packet şu an tam olarak Command ID'nin olduğu yerde duruyor.
        m_onPacketCallback(packet);
    }
}


// --- Veri G�nderme ---

bool NetClient::send(sf::Packet& pkt) {
    if (!m_connected) return false;

    // 1. Yeni bir paket oluştur
    sf::Packet finalPacket;

    // 2. Başlık ekle (Unreliable = 0)
    finalPacket << static_cast<sf::Uint8>(PacketType::Unreliable);

    // 3. Orijinal veriyi ekle
    finalPacket.append(pkt.getData(), pkt.getDataSize());

    // 4. Gönder
    sf::Socket::Status status = m_socket.send(finalPacket, m_serverAddress, m_serverPort);
    return status == sf::Socket::Done;
}

// source/Network/NetClient.cpp içine eklenecek fonksiyon
bool NetClient::sendReliable(sf::Packet& pkt) {
    if (!m_connected) return false;

    uint32_t seq = ++m_lastSequenceSent;

    sf::Packet reliablePkt;
    // Header: Reliable (1) + Sequence
    reliablePkt << static_cast<sf::Uint8>(PacketType::Reliable) << seq;

    // Orijinal veriyi ekle
    reliablePkt.append(pkt.getData(), pkt.getDataSize());

    PendingPacket pending;
    pending.packet = reliablePkt;
    pending.sequence = seq;
    pending.timer.restart();
    m_pendingPackets[seq] = pending;

    return m_socket.send(reliablePkt, m_serverAddress, m_serverPort) == sf::Socket::Done;
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