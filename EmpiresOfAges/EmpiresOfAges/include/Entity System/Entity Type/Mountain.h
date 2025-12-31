#pragma once
#include "Building.h"
#include <iostream>

class Mountain : public Building {
public:
    Mountain() {
        buildingType = BuildTypes::Mountain;
        isConstructed = true; // Doðal oluþum
        health = 100000;      // Yýkýlamaz gibi
    }

    std::string getInfo() override {
        return "Mountain";
    }

    int getMaxHealth() const override {
        return 100000;
    }

    // Tileset üzerindeki doðru varyasyonu seçen fonksiyon
    // mask: 0-15 arasý bir deðer (Komþuluk durumu)
    // tileSize: Oyunun kare boyutu (Örn: 32 veya 64)
    void setVariation(int mask, int tileSize) {
        if (!hasTexture) return;

        // 4x4'lük bir Tileset varsayýyoruz (Standart Autotile)
        // Sütun = mask % 4
        // Satýr = mask / 4
        int col = mask % 4;
        int row = mask / 4;

        // Texture üzerindeki dikdörtgen alaný (Rect) ayarla
        // Not: Eðer kaynak görselinizdeki kareler (tile) oyun içindeki tileSize'dan
        // farklý boyuttaysa, buradaki tileSize yerine görselin tile boyutunu yazmalýsýnýz.
        sf::IntRect rect(col * tileSize, row * tileSize, tileSize, tileSize);
        sprite.setTextureRect(rect);

        // Sprite'ýn origin'ini (merkez noktasýný) düzeltmek gerekebilir
        // çünkü TextureRect deðiþince boyut algýsý deðiþebilir.
        sprite.setOrigin(tileSize / 2.0f, tileSize / 2.0f);
    }
};