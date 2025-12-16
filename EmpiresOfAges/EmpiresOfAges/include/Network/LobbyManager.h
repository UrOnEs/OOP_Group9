// LobbyManager.h

#pragma once

#include <vector>
#include <string>
#include <map>
#include <functional>
#include <SFML/Network/Packet.hpp>
#include <SFML/Network/IpAddress.hpp>

// İleri Bildirim (Forward Declarations)
class NetworkManager;

// Lobi İletişim Komutları
enum class LobbyCommand : sf::Int32 {
    JoinRequest,
    LobbyStateSync,
    ToggleReady,      // Hazır durumunu değiştirmek (EVET/HAYIR)
    StartGameSignal,
    LeaveLobby        // <-- Lobiden ayrılma isteği
    // ... diğer komutlar
};

// Oyuncu Bilgisi Yapısı
struct PlayerInfo {
    uint64_t id;
    std::string name;
    bool ready;
    // ... diğer lobi bilgileri
};

class LobbyManager {
public:
    // Yapıcı
    LobbyManager(NetworkManager* netManager, bool isHost);

    // Ağ ve Lobi Başlatma
    void start(uint64_t selfId, const std::string& name);

    // Dış dünya ile etkileşim
    void toggleReady(bool isReady);
    void startGame();

    // Gelen Paket İşleme
    void handleIncomingPacket(uint64_t senderId, sf::Packet& pkt);

    // Getter'lar
    uint64_t selfId() const { return m_selfId; }
    const std::vector<PlayerInfo>& players() const { return m_players; }
    bool canStartGame() const;

    // KRİTİK GETTER: netManager'a erişim sağlar
    NetworkManager* netManager() const { return m_netManager; }

    // Callback Ayarları
    using PlayerChangeCallback = std::function<void()>;
    using GameStartCallback = std::function<void()>;
    void setOnPlayerChange(PlayerChangeCallback cb);
    void setOnGameStart(GameStartCallback cb);

private:
    // Lobi Yönetimi (Sunucu Tarafı)
    void processJoinRequest(uint64_t senderId, sf::Packet& pkt);
    void processToggleReady(uint64_t senderId, sf::Packet& pkt);
    void syncLobbyToClients(); // Tüm istemcilere lobi durumunu gönder

    // Yardımcı Metotlar
    void addPlayer(uint64_t id, const std::string& name, bool ready = false);
    void removePlayer(uint64_t id);
    void setReady(uint64_t id, bool ready);

    // Üye Değişkenleri
    NetworkManager* m_netManager;
    bool m_isHost;
    uint64_t m_selfId; // Kendi ID'miz (Host için 1, Client için Sunucudan gelen ID)
    std::vector<PlayerInfo> m_players;

    // Callback'ler
    PlayerChangeCallback m_onChange;
    GameStartCallback m_onGameStart;
    bool m_isGameStarted = false;
};