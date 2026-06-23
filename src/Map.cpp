#include "Map.hpp"
#include "Constants.hpp"
#include <fstream>
#include <cmath>
#include <algorithm>
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

        struct RawTS { int fgid; std::string name; int cols; };
        std::vector<RawTS> rawTS;
        for (auto& ts : data["tilesets"]) {
            RawTS r;
            r.fgid = ts["firstgid"].get<int>();
            r.name = ts.contains("name") ? ts["name"].get<std::string>() : "";
            r.cols = ts.contains("columns") ? ts["columns"].get<int>() : 0;
            rawTS.push_back(r);
        }
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
            m_tilesets.push_back(ti);
        }

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

// ============================================================
// DUAL WAYPOINTS
// Path TOP (kiri atas): spawn kiri atas -> lurus kanan ->
//   belok bawah -> sampai tengah Y -> lurus kanan ke base
// Path BOTTOM (kiri bawah): spawn kiri bawah -> lurus kanan ->
//   belok atas -> sampai tengah Y -> lurus kanan ke base
//
// Koordinat berdasarkan layout Map 1.2:
//   - Tengah Y (middle band) = 232
//   - Kolom belok (right turn) = 440
//   - Spawn top Y = 88, spawn bottom Y = 376
// ============================================================
void Map::buildWaypoints() {
    // --- PATH ATAS ---
    // Kiri atas -> lurus kanan -> belok bawah -> tengah -> base
    m_waypointsTop = {
        {  -30.f,  88.f },   // [0] SPAWN off-screen kiri atas
        {  440.f,  88.f },   // [1] lurus ke kanan hingga kolom belok
        {  440.f, 232.f },   // [2] belok bawah sampai tengah (middle band)
        {  848.f, 232.f },   // [3] lurus ke kanan menuju base
        { BASE_CENTER_X, BASE_CENTER_Y }, // [4] BASE
    };

    // --- PATH BAWAH ---
    // Kiri bawah -> lurus kanan -> belok atas -> tengah -> base
    m_waypointsBottom = {
        {  -30.f, 376.f },   // [0] SPAWN off-screen kiri bawah
        {  440.f, 376.f },   // [1] lurus ke kanan hingga kolom belok
        {  440.f, 232.f },   // [2] belok atas sampai tengah (middle band)
        {  848.f, 232.f },   // [3] lurus ke kanan menuju base
        { BASE_CENTER_X, BASE_CENTER_Y }, // [4] BASE
    };

    // Gabungkan semua waypoints untuk keperluan cek nearPath (deploy check)
    m_waypointsAll = m_waypointsTop;
    for (auto& wp : m_waypointsBottom)
        m_waypointsAll.push_back(wp);
}

void Map::draw(sf::RenderWindow& window) const {
    if (m_mapLoaded) {
        for (auto& layer : m_layers)
            drawTileLayer(window, layer);
    } else {
        sf::RectangleShape bg({(float)WINDOW_WIDTH,(float)GAME_AREA_HEIGHT});
        bg.setFillColor(sf::Color(55,80,40));
        window.draw(bg);
    }
    // TIDAK memanggil drawSpawnMarker -- marker sekarang invisible
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

void Map::drawBase(sf::RenderWindow& window) const {
    // Base sudah digambar via tile layer (house tileset)
    (void)window;
}

// ---------------------------------------------------------------
bool Map::isDeployable(sf::Vector2f pos) const {
    if (pos.x < 8.f || pos.x > WINDOW_WIDTH-8.f) return false;
    if (pos.y < 8.f || pos.y > GAME_AREA_HEIGHT-8.f) return false;
    if (nearPath(pos))      return false;
    if (onEnvLayer(pos))    return false;
    if (onObjectLayer(pos)) return false;
    if (onBaseBounds(pos))  return false;
    return true;
}

bool Map::nearPath(sf::Vector2f pos) const {
    // Cek terhadap SEMUA waypoint (top + bottom) agar area jalan tetap non-deployable
    for (size_t i=0; i+1<m_waypointsAll.size(); ++i) {
        // Skip segment yang menghubungkan tail top ke head bottom
        // (indeks m_waypointsTop.size()-1 ke m_waypointsTop.size() adalah sambungan buatan)
        // Kita cukup cek semua segment karena nearPath hanya untuk block deploy
        if (pointSegDist(pos, m_waypointsAll[i], m_waypointsAll[i+1]) < 26.f)
            return true;
    }
    // Cek juga tiap path secara mandiri untuk segment dalam setiap path
    for (size_t i=0; i+1<m_waypointsTop.size(); ++i)
        if (pointSegDist(pos, m_waypointsTop[i], m_waypointsTop[i+1]) < 26.f)
            return true;
    for (size_t i=0; i+1<m_waypointsBottom.size(); ++i)
        if (pointSegDist(pos, m_waypointsBottom[i], m_waypointsBottom[i+1]) < 26.f)
            return true;
    return false;
}

bool Map::onEnvLayer(sf::Vector2f pos) const {
    const int TS = 16;
    for (auto& layer : m_layers) {
        if (layer.name != "Tile Layer 3") continue;
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