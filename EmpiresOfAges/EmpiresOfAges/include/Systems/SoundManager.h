#pragma once
#include <SFML/Audio.hpp>
#include <map>
#include <list>
#include <string>

class SoundManager {
public:
    // Sesi yükler ve hafýzada tutar
    static void loadSound(const std::string& name, const std::string& filename);

    // Sesi çalar (Fire and Forget)
    static void playSound(const std::string& name);

    // Gereksiz sesleri temizlemek için (Game loop'ta çaðrýlmalý)
    static void update();

private:
    // Ses dosyalarýnýn verisi (RAM'de durur)
    static std::map<std::string, sf::SoundBuffer> soundBuffers;

    // O an çalan sesler (Çalma bitince listeden silinecek)
    static std::list<sf::Sound> activeSounds;
};