#pragma once
#include <SFML/Graphics.hpp>
#include <vector>

class Map {
public:
    Map();
    // Getter dua path terpisah untuk top dan bottom spawn
    const std::vector<sf::Vector2f>& getWaypointsTop()    const { return m_waypointsTop; }
    const std::vector<sf::Vector2f>& getWaypointsBottom() const { return m_waypointsBottom; }
    // Tetap ada untuk kompatibilitas (mengembalikan path top)
    const std::vector<sf::Vector2f>& getWaypoints()       const { return m_waypointsTop; }

    void draw(sf::RenderWindow& window) const;
    bool isDeployable(sf::Vector2f pos) const;

private:
    // Dua jalur terpisah: kiri atas dan kiri bawah
    std::vector<sf::Vector2f> m_waypointsTop;
    std::vector<sf::Vector2f> m_waypointsBottom;
    // Gabungan untuk cek nearPath saat deploy
    std::vector<sf::Vector2f> m_waypointsAll;

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
    // drawSpawnMarker dihilangkan supaya marker tidak muncul di map

    bool nearPath(sf::Vector2f pos) const;
    bool onEnvLayer(sf::Vector2f pos) const;
    bool onObjectLayer(sf::Vector2f pos) const;
    bool onBaseBounds(sf::Vector2f pos) const;
    float pointSegDist(sf::Vector2f p, sf::Vector2f a, sf::Vector2f b) const;
};