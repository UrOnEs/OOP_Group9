#include "UI/ResourceBar.h"
#include "UI/AssetManager.h"

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

    sf::Texture& barTex = AssetManager::getTexture("assets/ui/resourcePanel_b.png");
    if (barTex.getSize().x > 0) {
        barTex.setRepeated(true);
        barSprite.setTexture(barTex);
        hasTexture = true;
        barSprite.setPosition(0, 0);
        // setWidth çaðrýldýðýnda scale edilecek
    }

    auto setupIcon = [&](sf::Sprite& sprite, const std::string& path) {
        sf::Texture& tex = AssetManager::getTexture(path);
        if (tex.getSize().x > 0) {
            sprite.setTexture(tex);
            float iconSize = 32.0f;
            sprite.setScale(iconSize / tex.getSize().x, iconSize / tex.getSize().y);
        }
    };

    setupIcon(iconWood, "assets/ui/icon_wood.png");
    setupIcon(iconFood, "assets/ui/icon_food.png");
    setupIcon(iconGold, "assets/ui/icon_gold.png");
    setupIcon(iconStone, "assets/ui/icon_stone.png");

    // Ortak Ayarlar Ýçin Lambda veya Döngü kullanabiliriz ama 
    // senin için anlaþýlýr olsun diye tek tek yazýyorum.

    unsigned int fontSize = 18; // Daha kibar bir boyut
    float startX = 20.f;        // Soldan boþluk
    float spacing = 200.f;      // Her yazý arasý boþluk

    auto setupText = [&](sf::Text& txt, sf::Color color) {
        txt.setFont(font);
        txt.setCharacterSize(fontSize);
        txt.setFillColor(color);
        txt.setOutlineThickness(1.5f);
        txt.setOutlineColor(sf::Color::Black);
        };

    setupText(woodText, sf::Color(255, 255, 255)); // Artýk ikon olacaðý için yazýlar beyaz olabilir
    setupText(foodText, sf::Color(255, 255, 255));
    setupText(goldText, sf::Color(255, 255, 255));
    setupText(stoneText, sf::Color(255, 255, 255));
    setupText(populationText, sf::Color(255, 255, 255));

}

void ResourceBar::setWidth(float width) {
    backgroundBar.setSize(sf::Vector2f(width, 40.f));

    if (hasTexture) {
        // Texture referansýný tekrar alalým (Boyut hesaplamak için)
        const sf::Texture* tex = barSprite.getTexture();
        if (tex) {
            float targetHeight = 40.0f;
            float scale = targetHeight / (float)tex->getSize().y;

            barSprite.setScale(scale, scale);

            // 3. TextureRect ayarla (Döþeme Ýþlemi)
            // Mantýk: "Ekrana sýðmak için bu resimden yan yana kaç tane lazým?"
            // width / scale iþlemi, texture koordinat sisteminde ne kadar geniþlik gerektiðini bulur.
            barSprite.setTextureRect(sf::IntRect(0, 0, (int)(width / scale), tex->getSize().y));
        }
    }

    // Hizalama
    float startX = 30.f;
    float spacing = width / 6.0f; // Gruplar arasý mesafe
    float iconOffset = 40.0f;     // Ýkon ile yazý arasý mesafe

    auto placeGroup = [&](sf::Sprite& icon, sf::Text& text, int index) {
        float groupX = startX + (spacing * index);
        icon.setPosition(groupX, 4); // Y=8 ortalamak için
        text.setPosition(groupX + iconOffset, 8);
    };
    placeGroup(iconWood, woodText, 0);
    placeGroup(iconFood, foodText, 1);
    placeGroup(iconGold, goldText, 2);
    placeGroup(iconStone, stoneText, 3);
    populationText.setPosition(startX + (spacing * 4), 8);
}

void ResourceBar::updateResources(int wood, int food, int gold, int stone, Player player) {
    // Yazý formatýný da güzelleþtirelim
    // Örnek çýktý: "Wood: 150" yerine "[ Wood: 150 ]" gibi
    woodText.setString(std::to_string(wood));
    foodText.setString(std::to_string(food));
    goldText.setString(std::to_string(gold));
    stoneText.setString(std::to_string(stone));
    populationText.setString(std::to_string(player.getUnitCount()) + "  /  " + std::to_string(player.getUnitLimit()));
    
}

void ResourceBar::draw(sf::RenderWindow& window) {
    if (hasTexture) window.draw(barSprite);
    else window.draw(backgroundBar);

    window.draw(iconWood); window.draw(woodText);
    window.draw(iconFood); window.draw(foodText);
    window.draw(iconGold); window.draw(goldText);
    window.draw(iconStone); window.draw(stoneText);
    window.draw(populationText);
}

