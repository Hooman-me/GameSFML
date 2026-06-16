#pragma once
#include <SFML/Graphics.hpp>
#include <vector>

// ============================================================
//  MAP.HPP  –  Merender tilemap dari TMJ + waypoints
// ============================================================

class Map {
public:
    Map();

    const std::vector<sf::Vector2f>& getWaypoints() const { return m_waypoints; }

    void draw(sf::RenderWindow& window) const;
    bool isDeployable(sf::Vector2f pos) const;

private:
    std::vector<sf::Vector2f> m_waypoints;

    // Tileset textures untuk render
    sf::Texture m_texFloor;
    sf::Texture m_texNature;
    bool        m_floorLoaded  = false;
    bool        m_natureLoaded = false;

    // Tile layer data (dari TMJ)
    struct TileLayer {
        std::vector<int> data;
        int width = 0, height = 0;
    };
    std::vector<TileLayer> m_layers;

    // Tileset info
    struct TilesetInfo {
        int firstgid = 1;
        int columns  = 0;
        int tileW    = 16;
        int tileH    = 16;
        sf::Texture* tex = nullptr;
    };
    std::vector<TilesetInfo> m_tilesets;

    bool m_mapLoaded = false;

    void loadTMJ();
    void buildWaypoints();
    void drawTileLayer(sf::RenderWindow& window, const TileLayer& layer) const;
    void drawBase(sf::RenderWindow& window) const;
    void drawSpawnMarker(sf::RenderWindow& window) const;

    bool nearPath(sf::Vector2f pos, float margin = 30.f) const;
    float pointSegDist(sf::Vector2f p, sf::Vector2f a, sf::Vector2f b) const;
};