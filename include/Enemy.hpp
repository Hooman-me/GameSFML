#pragma once
#include <SFML/Graphics.hpp>
#include <vector>

// ============================================================
//  ENEMY.HPP  –  Data & logika satu musuh
// ============================================================

class Enemy {
public:
    Enemy(const std::vector<sf::Vector2f>& waypoints,
          sf::Texture* tex);          // boleh nullptr → pakai placeholder

    void update(float dt);
    void draw(sf::RenderWindow& window) const;

    // --- Getter ---
    sf::Vector2f getPosition() const { return m_pos; }
    sf::FloatRect getBounds()  const;
    bool  isDead()    const { return m_hp <= 0.f; }
    bool  hasReached()const { return m_reached; }  // sudah melewati base?
    float getHP()     const { return m_hp; }
    float getMaxHP()  const { return m_maxHp; }

    // Dipanggil saat kena peluru
    // Mengembalikan damage yang benar-benar diterima (setelah defense)
    float takeDamage(float rawDmg);

private:
    // --- State ---
    float m_hp, m_maxHp;
    float m_speed;
    float m_defense;
    bool  m_reached = false;

    // --- Posisi & navigasi ---
    sf::Vector2f             m_pos;
    int                      m_waypointIdx = 1; // menuju waypoint ke-1 dulu
    const std::vector<sf::Vector2f>& m_waypoints;

    // --- Animasi ---
    sf::Texture* m_tex      = nullptr;
    sf::Sprite   m_sprite;
    int          m_frameCol = 0;
    float        m_animTimer= 0.f;
    int          m_spriteRow= 2; // default: hadap kanan

    void updateAnimation(float dt, sf::Vector2f dir);
    void drawHPBar(sf::RenderWindow& window) const;
    void drawPlaceholder(sf::RenderWindow& window) const;
};