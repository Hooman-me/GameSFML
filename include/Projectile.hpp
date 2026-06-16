#pragma once
#include <SFML/Graphics.hpp>

// ============================================================
//  PROJECTILE.HPP  –  Fireball dengan sprite & animasi
// ============================================================

enum class ProjState { Flying, Exploding, Done };

class Projectile {
public:
    Projectile(sf::Vector2f origin, float angleRad, float damage,
               sf::Texture* texFireball);

    void update(float dt);
    void draw(sf::RenderWindow& window) const;

    sf::FloatRect getBounds() const;
    ProjState     getState()  const { return m_state; }
    float         getDamage() const { return m_damage; }

    void triggerExplosion();

private:
    sf::Vector2f  m_pos;
    sf::Vector2f  m_velocity;
    float         m_angleRad;
    float         m_damage;
    ProjState     m_state       = ProjState::Flying;
    float         m_explodeTimer= 0.f;

    sf::Texture*  m_texFireball = nullptr;
    sf::Sprite    m_sprite;
    int           m_frame       = 0;
    float         m_animTimer   = 0.f;

    // Explode: expand circle
    float         m_explodeRadius = 0.f;
};