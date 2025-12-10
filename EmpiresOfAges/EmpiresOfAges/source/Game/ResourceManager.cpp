#include "Game/ResourceManager.h"

ResourceManager::ResourceManager() {
    // Baþlangýç kaynaklarý (Ýstersen burada 0 býrakýp Player'da artýrabilirsin)
    Wood = 0;
    Gold = 0;
    Stone = 0;
    Food = 0;
}

ResourceManager::~ResourceManager() {
    // Map otomatik temizlenir
}

// --- KAYNAK YÖNETÝMÝ ---


// GENEL GETTER
int ResourceManager::getAmount(ResourceType type) const {
    switch (type) {
    case ResourceType::Wood: return Wood;
    case ResourceType::Food: return Food;
    case ResourceType::Gold: return Gold;
    case ResourceType::Stone: return Stone;
    default: return 0;
    }
}


// GENEL EKLEME FONKSÝYONU
void ResourceManager::add(ResourceType type, int amount) {
    switch (type) {
    case ResourceType::Wood:  Wood += amount; break;
    case ResourceType::Food:  Food += amount; break;
    case ResourceType::Gold:  Gold += amount; break;
    case ResourceType::Stone: Stone += amount; break;
    }
}

// GENEL HARCAMA FONKSÝYONU
bool ResourceManager::spend(ResourceType type, int amount) {
    // Önce yeterli kaynak var mý kontrol edelim
    switch (type) {
    case ResourceType::Wood:
        if (Wood >= amount) {
            Wood -= amount;
            return true; // Ýþlem baþarýlý
        }
        break;
    case ResourceType::Food:
        if (Food >= amount) {
            Food -= amount;
            return true;
        }
        break;
    case ResourceType::Gold:
        if (Gold >= amount) {
            Gold -= amount;
            return true;
        }
        break;
    case ResourceType::Stone:
        if (Stone >= amount) {
            Stone -= amount;
            return true;
        }
        break;
    }
}

// --- TEXTURE YÖNETÝMÝ ---
void ResourceManager::loadTexture(const std::string& name, const std::string& fileName) {
    sf::Texture tex;
    if (tex.loadFromFile(fileName)) {
        textures[name] = tex;
        std::cout << "[INFO] Texture loaded: " << name << std::endl;
    }
    else {
        std::cout << "[ERROR] Failed to load texture: " << fileName << std::endl;
    }
}

sf::Texture& ResourceManager::getTexture(const std::string& name) {
    if (textures.find(name) != textures.end()) {
        return textures[name];
    }
    // Hata durumunda program çökmesin diye fallback (ilk texture) veya boþ döndürülebilir
    // Not: Burasý production'da daha güvenli hale getirilmeli.
    static sf::Texture empty;
    std::cout << "[WARN] Texture not found: " << name << std::endl;
    return empty;
}