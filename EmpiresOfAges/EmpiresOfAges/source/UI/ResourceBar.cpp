#include "UI/ResourceBar.h"

ResourceBar::ResourceBar() {
    // 1. Font Y?kleme (Hata kontrol? ekledim, font yoksa ??kmesin)
    if (!font.loadFromFile("assets/fonts/arial.ttf")) {
        // Font y?klenemezse varsay?lan bir ?eyler yap?labilir ama ?imdilik ge?iyoruz
    }

    // 2. Arka Plan ?eridi (Simsiyah de?il, yar? saydam gri)
    // 1920 geni?lik, 40 y?kseklik. Ekran?n tepesini kaplar.
    backgroundBar.setSize(sf::Vector2f(1920.f, 40.f));
    backgroundBar.setFillColor(sf::Color(0, 0, 0, 150)); // Son parametre (150) saydaml?k (Alpha)
    backgroundBar.setPosition(0, 0);

    // Ortak Ayarlar ??in Lambda veya D?ng? kullanabiliriz ama 
    // senin i?in anla??l?r olsun diye tek tek yaz?yorum.

    unsigned int fontSize = 18; // Daha kibar bir boyut
    float startX = 20.f;        // Soldan bo?luk
    float spacing = 200.f;      // Her yaz? aras? bo?luk

    // --- ODUN (Kahverengi tonu) ---
    woodText.setFont(font);
    woodText.setCharacterSize(fontSize);
    woodText.setFillColor(sf::Color(222, 184, 135)); // Burlywood (Ah?ap rengi)
    woodText.setPosition(startX, 10);

    // --- YEMEK (Turuncu/K?rm?z? tonu) ---
    foodText.setFont(font);
    foodText.setCharacterSize(fontSize);
    foodText.setFillColor(sf::Color(255, 99, 71)); // Tomato (Domates rengi)
    foodText.setPosition(startX + spacing, 10);

    // --- ALTIN (Sar?) ---
    goldText.setFont(font);
    goldText.setCharacterSize(fontSize);
    goldText.setFillColor(sf::Color(255, 215, 0)); // Gold
    goldText.setPosition(startX + spacing * 2, 10);

    // --- TA? (Gri) ---
    stoneText.setFont(font);
    stoneText.setCharacterSize(fontSize);
    stoneText.setFillColor(sf::Color(192, 192, 192)); // Silver/Gri
    stoneText.setPosition(startX + spacing * 3, 10);
}

void ResourceBar::updateResources(int wood, int food, int gold, int stone) {
    // Yaz? format?n? da g?zelle?tirelim
    // ?rnek ??kt?: "Wood: 150" yerine "[ Wood: 150 ]" gibi
    woodText.setString("Wood: " + std::to_string(wood));
    foodText.setString("Food: " + std::to_string(food));
    goldText.setString("Gold: " + std::to_string(gold));
    stoneText.setString("Stone: " + std::to_string(stone));
}

void ResourceBar::draw(sf::RenderWindow& window) {
    // ?nce arka plan? ?iz (yaz?lar ?st?ne gelsin)
    window.draw(backgroundBar);

    window.draw(woodText);
    window.draw(foodText);
    window.draw(goldText);
    window.draw(stoneText);
}
