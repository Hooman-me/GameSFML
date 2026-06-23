#include "Map.hpp"
#include "Constants.hpp"
#include <fstream>
#include <cmath>
#include <nlohmann/json.hpp>
using json = nlohmann::json;

Map::Map() {
    m_floorOk  = m_texFloor.loadFromFile(TILESET_FLOOR_PATH);
    m_waterOk  = m_texWater.loadFromFile(TILESET_WATER_PATH);
    m_natureOk = m_texNature.loadFromFile(TILESET_NATURE_PATH);
    m_houseOk  = m_texHouse.loadFromFile(TILESET_HOUSE_PATH);
    loadTMJ();
    buildWaypoints();
}

void Map::loadTMJ() {
    std::ifstream f(MAP_TMJ_PATH);
    if (!f.is_open()) return;
    try {
        json data = json::parse(f);
        int mapW = data["width"].get<int>();
        int mapH = data["height"].get<int>();

        // *** Tileset mapping — sesuaikan firstgid/lastgid dari TMJ ***
        // TMJ Map 1.2: jalur=1, TilesetFloor=76, TilesetWater=648,
        //              TilesetNature=1124, BASE=1628, tes(House)=1635
        struct RawTS { int fgid; std::string name; int cols; };
        std::vector<RawTS> rawTS;
        for (auto& ts : data["tilesets"]) {
            RawTS r;
            r.fgid = ts["firstgid"].get<int>();
            r.name = ts.contains("name") ? ts["name"].get<std::string>() : "";
            r.cols = ts.contains("columns") ? ts["columns"].get<int>() : 0;
            rawTS.push_back(r);
        }
        // Sort by firstgid to compute lastgid
        std::sort(rawTS.begin(), rawTS.end(),
            [](const RawTS& a, const RawTS& b){ return a.fgid < b.fgid; });

        for (int i=0; i<(int)rawTS.size(); ++i) {
            TilesetInfo ti;
            ti.firstgid = rawTS[i].fgid;
            ti.lastgid  = (i+1 < (int)rawTS.size()) ? rawTS[i+1].fgid-1 : 99999;
            ti.columns  = rawTS[i].cols;
            ti.tileW    = 16; ti.tileH = 16;

            const std::string& nm = rawTS[i].name;
            if (nm == "TilesetFloor"  && m_floorOk)  ti.tex = &m_texFloor;
            if (nm == "TilesetWater"  && m_waterOk)  ti.tex = &m_texWater;
            if (nm == "TilesetNature" && m_natureOk) ti.tex = &m_texNature;
            if (nm == "tes"           && m_houseOk)  ti.tex = &m_texHouse;
            // "BASE" (big images) & "jalur.tsx" — skip render, no valid texture
            m_tilesets.push_back(ti);
        }

        // Load tile layers
        for (auto& layer : data["layers"]) {
            std::string type = layer["type"].get<std::string>();
            if (type == "tilelayer") {
                TileLayer tl;
                tl.name   = layer["name"].get<std::string>();
                tl.width  = mapW;
                tl.height = mapH;
                for (auto& v : layer["data"]) tl.data.push_back(v.get<int>());
                m_layers.push_back(std::move(tl));
            }
        }
        m_mapLoaded = true;
    } catch (...) { m_mapLoaded = false; }
}

// *** DIPERBAIKI: waypoints sesuai Map 1.2 layout yang terlihat di screenshot ***
// Path layout (dari analisis tile Layer 2):
// Spawn kiri -> kanan (top band row4-6, y=88)
// -> turun (right vertical col27, x=440)
// -> kiri (bottom band row22-24, y=376)
// -> naik (inner left col2, x=24)
// -> kanan (middle band row12-15, y=232)
// -> base (rumah col54-57 row11-15, center x=896 y=216)
void Map::buildWaypoints() {
    m_waypoints = {
        {   0.f,  88.f },   // [0] SPAWN – masuk dari kiri atas
        { 440.f,  88.f },   // [1] belokan kanan atas
        { 440.f, 376.f },   // [2] belokan kanan bawah
        {  24.f, 376.f },   // [3] belokan kiri bawah
        {  24.f, 232.f },   // [4] naik ke tengah, ketemu middle band
        { 440.f, 232.f },   // [5] right junction – gabung ke jalur tengah
        { 848.f, 232.f },   // [6] ujung exit path (col53)
        { BASE_CENTER_X, BASE_CENTER_Y }, // [7] BASE – tengah rumah
    };
}

void Map::draw(sf::RenderWindow& window) const {
    if (m_mapLoaded) {
        for (auto& layer : m_layers)
            drawTileLayer(window, layer);
    } else {
        // Fallback background
        sf::RectangleShape bg({(float)WINDOW_WIDTH,(float)GAME_AREA_HEIGHT});
        bg.setFillColor(sf::Color(55,80,40));
        window.draw(bg);
    }
    drawSpawnMarker(window);
}

void Map::drawTileLayer(sf::RenderWindow& window, const TileLayer& layer) const {
    const int TS = 16;
    for (int row=0; row<layer.height; ++row) {
        for (int col=0; col<layer.width; ++col) {
            int gid = layer.data[row*layer.width+col];
            if (gid == 0) continue;
            const TilesetInfo* ts = nullptr;
            for (int i=(int)m_tilesets.size()-1; i>=0; --i) {
                if (m_tilesets[i].firstgid <= gid) { ts=&m_tilesets[i]; break; }
            }
            if (!ts || !ts->tex || ts->columns<=0) continue;
            int lid  = gid - ts->firstgid;
            int tcol = lid % ts->columns;
            int trow = lid / ts->columns;
            sf::Sprite sp;
            sp.setTexture(*ts->tex);
            sp.setTextureRect(sf::IntRect(tcol*ts->tileW, trow*ts->tileH, ts->tileW, ts->tileH));
            sp.setPosition((float)(col*TS), (float)(row*TS));
            window.draw(sp);
        }
    }
}

void Map::drawSpawnMarker(sf::RenderWindow& window) const {
    if (m_waypoints.empty()) return;
    sf::CircleShape mark(10.f);
    mark.setFillColor(sf::Color(220,50,50,180));
    mark.setOutlineColor(sf::Color::White);
    mark.setOutlineThickness(1.5f);
    mark.setOrigin(10.f,10.f);
    mark.setPosition(m_waypoints.front());
    window.draw(mark);
}

// ---------------------------------------------------------------
bool Map::isDeployable(sf::Vector2f pos) const {
    // Batas layar
    if (pos.x < 8.f || pos.x > WINDOW_WIDTH-8.f) return false;
    if (pos.y < 8.f || pos.y > GAME_AREA_HEIGHT-8.f) return false;

    // Cek di atas jalur (Layer 2 tiles)
    if (nearPath(pos)) return false;

    // Cek Layer 3 (environment dekorasi)
    if (onEnvLayer(pos)) return false;

    // Cek Layer 4 (pohon besar, batu, base/rumah)
    if (onObjectLayer(pos)) return false;

    // Cek kolisi dengan base box
    if (onBaseBounds(pos)) return false;

    return true;
}

bool Map::nearPath(sf::Vector2f pos) const {
    for (size_t i=0; i+1<m_waypoints.size(); ++i) {
        if (pointSegDist(pos, m_waypoints[i], m_waypoints[i+1]) < 26.f)
            return true;
    }
    return false;
}

// Cek apakah tile di Layer 3 (environment) ada di posisi ini
bool Map::onEnvLayer(sf::Vector2f pos) const {
    const int TS = 16;
    for (auto& layer : m_layers) {
        if (layer.name != "Tile Layer 3") continue;
        // Check pos and neighbors (radius ~20px = 1.25 tiles)
        for (int dy=-1; dy<=1; ++dy) {
            for (int dx=-1; dx<=1; ++dx) {
                int col = (int)((pos.x + dx*20.f) / TS);
                int row = (int)((pos.y + dy*20.f) / TS);
                if (col<0||row<0||col>=layer.width||row>=layer.height) continue;
                if (layer.data[row*layer.width+col] != 0) return true;
            }
        }
    }
    return false;
}

// Cek apakah tile di Layer 4 (pohon besar/batu/base) ada di posisi ini
bool Map::onObjectLayer(sf::Vector2f pos) const {
    const int TS = 16;
    for (auto& layer : m_layers) {
        if (layer.name != "Tile Layer 4") continue;
        for (int dy=-1; dy<=1; ++dy) {
            for (int dx=-1; dx<=1; ++dx) {
                int col = (int)((pos.x + dx*20.f) / TS);
                int row = (int)((pos.y + dy*20.f) / TS);
                if (col<0||row<0||col>=layer.width||row>=layer.height) continue;
                if (layer.data[row*layer.width+col] != 0) return true;
            }
        }
    }
    return false;
}

bool Map::onBaseBounds(sf::Vector2f pos) const {
    float hw = BASE_SIZE_W * .5f + 10.f;
    float hh = BASE_SIZE_H * .5f + 10.f;
    return (std::abs(pos.x - BASE_CENTER_X) < hw &&
            std::abs(pos.y - BASE_CENTER_Y) < hh);
}

float Map::pointSegDist(sf::Vector2f p, sf::Vector2f a, sf::Vector2f b) const {
    sf::Vector2f ab=b-a, ap=p-a;
    float len2 = ab.x*ab.x+ab.y*ab.y;
    if (len2<1e-6f) {
        float dx=p.x-a.x,dy=p.y-a.y; return std::sqrt(dx*dx+dy*dy);
    }
    float t=std::max(0.f,std::min(1.f,(ap.x*ab.x+ap.y*ab.y)/len2));
    sf::Vector2f cl={a.x+t*ab.x,a.y+t*ab.y};
    float dx=p.x-cl.x,dy=p.y-cl.y;
    return std::sqrt(dx*dx+dy*dy);
}