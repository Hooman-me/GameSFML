#pragma once
#include <SFML/Graphics.hpp>

// ============================================================
//  PROJECTILE.HPP  –  Peluru yang ditembakkan troop
// ============================================================

enum class ProjState { Flying, Exploding, Done };

class Projectile {
public:
    // angle dalam RADIAN, arah terbang lurus
    Projectile(sf::Vector2f origin, float angleRad, float damage);

    void update(float dt);
    void draw(sf::RenderWindow& window) const;

    sf::FloatRect getBounds() const;
    ProjState     getState()  const { return m_state; }
    float         getDamage() const { return m_damage; }

    // Dipanggil PlayState saat mendeteksi tabrakan
    void triggerExplosion();

private:
    sf::Vector2f m_pos;
    sf::Vector2f m_velocity;
    float        m_damage;
    ProjState    m_state  = ProjState::Flying;
    float        m_explodeTimer = 0.f;

    // Visual
    float        m_rotAngle = 0.f; // rotasi animasi "berputar" peluru
};