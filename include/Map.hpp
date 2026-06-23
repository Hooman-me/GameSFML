#pragma once
#include <SFML/Graphics.hpp>
#include <vector>

class Map {
public:
    Map();
    const std::vector<sf::Vector2f>& getWaypoints() const { return m_waypoints; }
    void draw(sf::RenderWindow& window) const;
    // Cek apakah titik layak deploy:
    // - tidak di atas jalur
    // - tidak di atas tile environment (Layer 3)
    // - tidak di atas tile Layer 4 (pohon/batu/base)
    // - tidak di atas base (rumah)
    bool isDeployable(sf::Vector2f pos) const;

private:
    std::vector<sf::Vector2f> m_waypoints;

    // Tileset textures
    sf::Texture m_texFloor, m_texWater, m_texNature, m_texHouse;
    bool        m_floorOk=false, m_waterOk=false, m_natureOk=false, m_houseOk=false;

    struct TileLayer {
        std::vector<int> data;
        int width=0, height=0;
        std::string name;
    };
    std::vector<TileLayer> m_layers;

    struct TilesetInfo {
        int firstgid=1, lastgid=0;
        int columns=0, tileW=16, tileH=16;
        sf::Texture* tex=nullptr;
    };
    std::vector<TilesetInfo> m_tilesets;
    bool m_mapLoaded=false;

    void loadTMJ();
    void buildWaypoints();
    void drawTileLayer(sf::RenderWindow& window, const TileLayer& layer) const;
    void drawBase(sf::RenderWindow& window) const;
    void drawSpawnMarker(sf::RenderWindow& window) const;

    bool nearPath(sf::Vector2f pos) const;
    bool onEnvLayer(sf::Vector2f pos) const;     // Layer 3
    bool onObjectLayer(sf::Vector2f pos) const;  // Layer 4 (pohon/batu/base)
    bool onBaseBounds(sf::Vector2f pos) const;
    float pointSegDist(sf::Vector2f p, sf::Vector2f a, sf::Vector2f b) const;
};