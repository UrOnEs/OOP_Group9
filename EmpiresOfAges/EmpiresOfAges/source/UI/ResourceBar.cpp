#include "UI/ResourceBar.h"

ResourceBar::ResourceBar() {
    // 1. Font Yükleme (Hata kontrolü ekledim, font yoksa çökmesin)
    if (!font.loadFromFile("assets/fonts/arial.ttf")) {
        // Font yüklenemezse varsayýlan bir þeyler yapýlabilir ama þimdilik geçiyoruz
    }

    // 2. Arka Plan Þeridi (Simsiyah deðil, yarý saydam gri)
    // 1920 geniþlik, 40 yükseklik. Ekranýn tepesini kaplar.
    backgroundBar.setSize(sf::Vector2f(1920.f, 40.f));
    backgroundBar.setFillColor(sf::Color(0, 0, 0, 150));
    backgroundBar.setPosition(0, 0);

    // Ortak Ayarlar Ýçin Lambda veya Döngü kullanabiliriz ama 
    // senin için anlaþýlýr olsun diye tek tek yazýyorum.

    unsigned int fontSize = 18; // Daha kibar bir boyut
    float startX = 20.f;        // Soldan boþluk
    float spacing = 200.f;      // Her yazý arasý boþluk

    // --- ODUN (Kahverengi tonu) ---
    woodText.setFont(font);
    woodText.setCharacterSize(fontSize);
    woodText.setFillColor(sf::Color(222, 184, 135)); // Burlywood (Ahþap rengi)
    woodText.setPosition(startX, 10);

    // --- YEMEK (Turuncu/Kýrmýzý tonu) ---
    foodText.setFont(font);
    foodText.setCharacterSize(fontSize);
    foodText.setFillColor(sf::Color(255, 99, 71)); // Tomato (Domates rengi)
    foodText.setPosition(startX + spacing, 10);

    // --- ALTIN (Sarý) ---
    goldText.setFont(font);
    goldText.setCharacterSize(fontSize);
    goldText.setFillColor(sf::Color(255, 215, 0)); // Gold
    goldText.setPosition(startX + spacing * 2, 10);

    // --- TAÞ (Gri) ---
    stoneText.setFont(font);
    stoneText.setCharacterSize(fontSize);
    stoneText.setFillColor(sf::Color(192, 192, 192)); // Silver/Gri
    stoneText.setPosition(startX + spacing * 3, 10);

    // --- POPULATÝON ---
    populationText.setFont(font);
    populationText.setCharacterSize(fontSize);
    populationText.setFillColor(sf::Color(256, 256, 256)); //siyah (sanýrým)
    populationText.setPosition(startX + spacing * 4, 10);
}

void ResourceBar::setWidth(float width) {
    backgroundBar.setSize(sf::Vector2f(width, 40.f));

    // Yazýlarý ekrana orantýlý yaymak için:
    float startX = 20.f;
    float spacing = width / 6.0f; // Ekraný parçalara bölerek hizala

    woodText.setPosition(startX, 10);
    foodText.setPosition(startX + spacing, 10);
    goldText.setPosition(startX + spacing * 2, 10);
    stoneText.setPosition(startX + spacing * 3, 10);
    populationText.setPosition(startX + spacing * 4, 10);
}

void ResourceBar::updateResources(int wood, int food, int gold, int stone, Player player) {
    // Yazý formatýný da güzelleþtirelim
    // Örnek çýktý: "Wood: 150" yerine "[ Wood: 150 ]" gibi
    woodText.setString("Wood: " + std::to_string(wood));
    foodText.setString("Food: " + std::to_string(food));
    goldText.setString("Gold: " + std::to_string(gold));
    stoneText.setString("Stone: " + std::to_string(stone));
    populationText.setString(std::to_string(player.getUnitCount()) + "  /  " + std::to_string(player.getUnitLimit()));
    
}

void ResourceBar::draw(sf::RenderWindow& window) {
    // Önce arka planý çiz (yazýlar üstüne gelsin)
    window.draw(backgroundBar);

    window.draw(woodText);
    window.draw(foodText);
    window.draw(goldText);
    window.draw(stoneText);
    window.draw(populationText);
}

