#include "Map.hpp"
#include "Constants.hpp"
#include <fstream>
#include <sstream>
#include <cmath>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

Map::Map() {
    m_floorLoaded  = m_texFloor.loadFromFile(TILESET_FLOOR_PATH);
    m_natureLoaded = m_texNature.loadFromFile(TILESET_NATURE_PATH);
    loadTMJ();
    buildWaypoints();
}

void Map::loadTMJ() {
    std::ifstream f(MAP_TMJ_PATH);
    if (!f.is_open()) return;

    try {
        json data = json::parse(f);
        int tileW = data["tilewidth"].get<int>();
        int tileH = data["tileheight"].get<int>();
        int mapW  = data["width"].get<int>();
        int mapH  = data["height"].get<int>();

        // Setup tilesets
        for (auto& ts : data["tilesets"]) {
            TilesetInfo ti;
            ti.firstgid = ts["firstgid"].get<int>();
            if (ts.contains("columns")) ti.columns = ts["columns"].get<int>();
            if (ts.contains("tilewidth"))  ti.tileW = ts["tilewidth"].get<int>();
            if (ts.contains("tileheight")) ti.tileH = ts["tileheight"].get<int>();
            // Map textures
            if (ts.contains("name")) {
                std::string name = ts["name"].get<std::string>();
                if (name == "TilesetFloor"   && m_floorLoaded)  ti.tex = &m_texFloor;
                if (name == "TilesetNature"  && m_natureLoaded) ti.tex = &m_texNature;
            }
            m_tilesets.push_back(ti);
        }

        // Load tile layers
        for (auto& layer : data["layers"]) {
            if (layer["type"].get<std::string>() == "tilelayer") {
                TileLayer tl;
                tl.width  = mapW;
                tl.height = mapH;
                auto& d   = layer["data"];
                tl.data.reserve(d.size());
                for (auto& v : d) tl.data.push_back(v.get<int>());
                m_layers.push_back(std::move(tl));
            }
        }
        m_mapLoaded = true;
    } catch (...) {
        m_mapLoaded = false;
    }
}

// *** Waypoints sesuai peta (Map 1.tmj, 1056x528) ***
// Jalur dari tile layer 2 (visual path tiles)
// Musuh spawn dari kiri-atas -> loop kotak -> exit ke kanan -> base
void Map::buildWaypoints() {
    // Tile path width = 3 tiles = 48px, center = +24px
    // Top band:    tiles row 4-6  (y=64-96)  center y=80
    // Right side:  tiles col 26-28 (x=416-448) center x=432
    // Bottom band: tiles row 22-24 (y=352-384) center y=368
    // Left side:   tiles col 0-2   (x=0-32)   center x=16 (tapi spawn di x=0)
    // Inner-left:  tiles col 2-4   (x=32-64)  center x=48
    // Center exit: tiles row 13-15 (y=208-240) center y=224
    // Exit path goes right from col 26 to col 65

    m_waypoints = {
        {   0.f,  80.f },  // [0] SPAWN – kiri atas
        { 432.f,  80.f },  // [1] Pojok kanan atas
        { 432.f, 368.f },  // [2] Pojok kanan bawah
        {  16.f, 368.f },  // [3] Pojok kiri bawah
        {  16.f, 224.f },  // [4] Tengah kiri (titik berbalik)
        { 432.f, 224.f },  // [5] Tengah (exit loop ke kanan)
        {1048.f, 224.f },  // [6] BASE – kanan tengah
    };
    // *** Ubah koordinat di atas untuk mengubah jalur ***
}

void Map::draw(sf::RenderWindow& window) const {
    // Render tile layers
    if (m_mapLoaded) {
        for (auto& layer : m_layers) {
            drawTileLayer(window, layer);
        }
    } else {
        // Fallback: gambar background sederhana
        sf::RectangleShape bg({ (float)WINDOW_WIDTH, (float)GAME_AREA_HEIGHT });
        bg.setFillColor(sf::Color(45, 65, 40));
        window.draw(bg);

        // Gambar jalur manual
        sf::Color pathColor(0xC8, 0x9B, 0x0A);
        float hw = 24.f;
        for (size_t i = 0; i + 1 < m_waypoints.size(); ++i) {
            auto a = m_waypoints[i], b = m_waypoints[i+1];
            sf::Vector2f diff = b - a;
            float len = std::sqrt(diff.x*diff.x + diff.y*diff.y);
            if (len < 1.f) continue;
            sf::RectangleShape seg;
            seg.setSize({ len, hw*2.f });
            seg.setFillColor(pathColor);
            float ang = std::atan2(diff.y, diff.x) * 180.f / 3.14159265f;
            seg.setRotation(ang);
            sf::Vector2f perp = { -diff.y/len*hw, diff.x/len*hw };
            seg.setPosition(a + perp);
            window.draw(seg);
            sf::CircleShape jnt(hw);
            jnt.setFillColor(pathColor);
            jnt.setOrigin(hw, hw);
            jnt.setPosition(a);
            window.draw(jnt);
        }
    }

    drawBase(window);
    drawSpawnMarker(window);
}

void Map::drawTileLayer(sf::RenderWindow& window, const TileLayer& layer) const {
    const int TS = 16;
    for (int row = 0; row < layer.height; ++row) {
        for (int col = 0; col < layer.width; ++col) {
            int gid = layer.data[row * layer.width + col];
            if (gid == 0) continue;

            // Find tileset for this gid
            const TilesetInfo* ts = nullptr;
            for (int i = (int)m_tilesets.size()-1; i >= 0; --i) {
                if (m_tilesets[i].firstgid <= gid) { ts = &m_tilesets[i]; break; }
            }
            if (!ts || !ts->tex || ts->columns <= 0) continue;

            int lid = gid - ts->firstgid;
            int tcol = lid % ts->columns;
            int trow = lid / ts->columns;

            sf::Sprite sp;
            sp.setTexture(*ts->tex);
            sp.setTextureRect(sf::IntRect(
                tcol * ts->tileW, trow * ts->tileH,
                ts->tileW, ts->tileH));
            sp.setPosition((float)(col * TS), (float)(row * TS));
            window.draw(sp);
        }
    }
}

void Map::drawBase(sf::RenderWindow& window) const {
    if (m_waypoints.empty()) return;
    auto basePos = m_waypoints.back();
    float sz = 72.f;
    sf::RectangleShape base({ sz, sz });
    base.setFillColor(sf::Color(0x00, 0xBF, 0xD8, 220));
    base.setOutlineColor(sf::Color::White);
    base.setOutlineThickness(2.f);
    base.setOrigin(sz*.5f, sz*.5f);
    base.setPosition(basePos);
    window.draw(base);
}

void Map::drawSpawnMarker(sf::RenderWindow& window) const {
    if (m_waypoints.empty()) return;
    sf::CircleShape mark(12.f);
    mark.setFillColor(sf::Color(200, 50, 50, 180));
    mark.setOutlineColor(sf::Color::White);
    mark.setOutlineThickness(1.5f);
    mark.setOrigin(12.f, 12.f);
    mark.setPosition(m_waypoints.front());
    window.draw(mark);
}

bool Map::isDeployable(sf::Vector2f pos) const {
    if (pos.y > GAME_AREA_HEIGHT - 10.f) return false;
    if (pos.x < 5.f || pos.x > WINDOW_WIDTH - 5.f) return false;
    if (nearPath(pos)) return false;
    auto base = m_waypoints.back();
    float dx = pos.x - base.x, dy = pos.y - base.y;
    if (std::sqrt(dx*dx + dy*dy) < 50.f) return false;
    return true;
}

bool Map::nearPath(sf::Vector2f pos, float margin) const {
    for (size_t i = 0; i+1 < m_waypoints.size(); ++i) {
        if (pointSegDist(pos, m_waypoints[i], m_waypoints[i+1]) < 28.f + margin)
            return true;
    }
    return false;
}

float Map::pointSegDist(sf::Vector2f p, sf::Vector2f a, sf::Vector2f b) const {
    sf::Vector2f ab = b-a, ap = p-a;
    float len2 = ab.x*ab.x + ab.y*ab.y;
    if (len2 < 1e-6f) {
        float dx=p.x-a.x, dy=p.y-a.y;
        return std::sqrt(dx*dx+dy*dy);
    }
    float t = std::max(0.f, std::min(1.f, (ap.x*ab.x+ap.y*ab.y)/len2));
    sf::Vector2f cl = {a.x+t*ab.x, a.y+t*ab.y};
    float dx=p.x-cl.x, dy=p.y-cl.y;
    return std::sqrt(dx*dx+dy*dy);
}