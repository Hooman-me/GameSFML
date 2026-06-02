#pragma once
#include <SFML/Graphics.hpp>
#include <vector>

// ============================================================
//  MAP.HPP  –  Jalur, waypoints, dan visual peta
// ============================================================

class Map {
public:
    Map();

    // Daftar titik belok jalur musuh (pixel bebas, gaya BTD6)
    // *** Ubah koordinat ini untuk mengubah bentuk jalur ***
    const std::vector<sf::Vector2f>& getWaypoints() const { return m_waypoints; }

    void draw(sf::RenderWindow& window) const;

    // Cek apakah posisi berada di area yang bisa di-deploy troop (bukan di atas jalur/base)
    bool isDeployable(sf::Vector2f pos) const;

private:
    std::vector<sf::Vector2f> m_waypoints;
    float m_pathWidth = 55.f; // *** lebar visual jalur emas ***

    void buildWaypoints();
    void drawPath(sf::RenderWindow& window) const;
    void drawBase(sf::RenderWindow& window) const;
    void drawSpawnMarker(sf::RenderWindow& window) const;

    // Cek apakah titik terlalu dekat ke segmen jalur
    bool nearPath(sf::Vector2f pos, float margin = 30.f) const;
    float pointToSegmentDist(sf::Vector2f p,
                             sf::Vector2f a,
                             sf::Vector2f b) const;
};