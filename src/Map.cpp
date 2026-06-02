#include "Map.hpp"
#include "Constants.hpp"
#include <cmath>

// ============================================================
//  MAP.CPP
// ============================================================

Map::Map() {
    buildWaypoints();
}

// *** UBAH KOORDINAT INI UNTUK MENGUBAH BENTUK JALUR ***
// Sesuai dengan gambar UI: jalur membentuk loop kotak di kiri,
// lalu keluar ke kanan menuju base.
// Koordinat dalam pixel, (0,0) = sudut kiri-atas area game.
void Map::buildWaypoints() {
    m_waypoints = {
        {  20.f, 115.f },   // [0] SPAWN – sisi kiri atas
        { 430.f, 115.f },   // [1] belok kanan atas
        { 430.f, 480.f },   // [2] belok kanan bawah
        {  20.f, 480.f },   // [3] belok kiri bawah
        {  20.f, 295.f },   // [4] tengah kiri (bertemu ujung loop)
        { 430.f, 295.f },   // [5] keluar dari loop ke kanan
        { 950.f, 295.f },   // [6] menuju base
    };
}

// --- Draw seluruh peta ---
void Map::draw(sf::RenderWindow& window) const {
    drawPath(window);
    drawBase(window);
    drawSpawnMarker(window);
}

// Gambar jalur sebagai rangkaian kotak tebal berwarna emas
void Map::drawPath(sf::RenderWindow& window) const {
    sf::Color pathColor(0xC8, 0x9B, 0x0A); // *** warna jalur emas ***

    const float hw = m_pathWidth * 0.5f;

    for (size_t i = 0; i + 1 < m_waypoints.size(); ++i) {
        sf::Vector2f a = m_waypoints[i];
        sf::Vector2f b = m_waypoints[i + 1];

        sf::Vector2f diff = b - a;
        float len = std::sqrt(diff.x * diff.x + diff.y * diff.y);
        if (len < 1.f) continue;

        sf::RectangleShape seg;
        seg.setSize({ len, m_pathWidth });
        seg.setFillColor(pathColor);

        // Hitung sudut
        float angle = std::atan2(diff.y, diff.x) * 180.f / 3.14159265f;
        seg.setRotation(angle);

        // Origin di tengah-kiri segmen
        sf::Vector2f perp = { -diff.y / len * hw, diff.x / len * hw };
        seg.setPosition(a + perp);

        window.draw(seg);

        // Gambar lingkaran di setiap sambungan waypoint agar sudutnya rapi
        sf::CircleShape joint(hw);
        joint.setFillColor(pathColor);
        joint.setOrigin(hw, hw);
        joint.setPosition(a);
        window.draw(joint);
    }
    // Lingkaran di waypoint terakhir
    sf::CircleShape joint(hw);
    joint.setFillColor(pathColor);
    joint.setOrigin(hw, hw);
    joint.setPosition(m_waypoints.back());
    window.draw(joint);
}

// Gambar base (kotak biru) di posisi waypoint terakhir
void Map::drawBase(sf::RenderWindow& window) const {
    if (m_waypoints.empty()) return;
    sf::Vector2f basePos = m_waypoints.back();
    float sz = 80.f; // *** ukuran kotak base ***

    sf::RectangleShape base({ sz, sz });
    base.setFillColor(sf::Color(0x00, 0xBF, 0xD8, 220)); // biru cyan
    base.setOutlineColor(sf::Color::White);
    base.setOutlineThickness(2.f);
    base.setOrigin(sz * 0.5f, sz * 0.5f);
    base.setPosition(basePos);
    window.draw(base);
}

// Marker kecil merah di spawn
void Map::drawSpawnMarker(sf::RenderWindow& window) const {
    if (m_waypoints.empty()) return;
    sf::CircleShape mark(14.f);
    mark.setFillColor(sf::Color(200, 50, 50, 180));
    mark.setOutlineColor(sf::Color::White);
    mark.setOutlineThickness(1.5f);
    mark.setOrigin(14.f, 14.f);
    mark.setPosition(m_waypoints.front());
    window.draw(mark);
}

// Cek apakah posisi layak untuk deploy troop
bool Map::isDeployable(sf::Vector2f pos) const {
    // Harus di dalam area game (bukan UI bar)
    if (pos.y > GAME_AREA_HEIGHT - 10.f) return false;
    if (pos.x < 5.f || pos.x > WINDOW_WIDTH - 5.f) return false;

    // Tidak boleh di atas/dekat jalur
    if (nearPath(pos)) return false;

    // Tidak boleh di atas base
    sf::Vector2f basePos = m_waypoints.back();
    float dx = pos.x - basePos.x;
    float dy = pos.y - basePos.y;
    if (std::sqrt(dx * dx + dy * dy) < 55.f) return false;

    return true;
}

bool Map::nearPath(sf::Vector2f pos, float margin) const {
    for (size_t i = 0; i + 1 < m_waypoints.size(); ++i) {
        float d = pointToSegmentDist(pos, m_waypoints[i], m_waypoints[i + 1]);
        if (d < m_pathWidth * 0.5f + margin) return true;
    }
    return false;
}

float Map::pointToSegmentDist(sf::Vector2f p,
                               sf::Vector2f a,
                               sf::Vector2f b) const {
    sf::Vector2f ab = b - a;
    sf::Vector2f ap = p - a;
    float len2 = ab.x * ab.x + ab.y * ab.y;
    if (len2 < 1e-6f) {
        float dx = p.x - a.x, dy = p.y - a.y;
        return std::sqrt(dx * dx + dy * dy);
    }
    float t = std::max(0.f, std::min(1.f, (ap.x * ab.x + ap.y * ab.y) / len2));
    sf::Vector2f closest = { a.x + t * ab.x, a.y + t * ab.y };
    float dx = p.x - closest.x, dy = p.y - closest.y;
    return std::sqrt(dx * dx + dy * dy);
}