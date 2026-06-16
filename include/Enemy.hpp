#pragma once
#include <SFML/Graphics.hpp>
#include <vector>

// ============================================================
//  ENEMY.HPP  –  Musuh Slime dengan animasi multi-state
// ============================================================

enum class EnemyAnim { Idle, Walk, Hurt, Dead };

class Enemy {
public:
    Enemy(const std::vector<sf::Vector2f>& waypoints,
          sf::Texture* texIdle,
          sf::Texture* texWalk,
          sf::Texture* texHurt);

    void update(float dt);
    void draw(sf::RenderWindow& window) const;

    sf::Vector2f  getPosition() const { return m_pos; }
    sf::FloatRect getBounds()   const;
    bool isDead()    const { return m_hp <= 0.f && m_deathTimer <= 0.f; }
    bool hasReached()const { return m_reached; }
    float getHP()    const { return m_hp; }
    float getMaxHP() const { return m_maxHp; }

    // Returns actual damage dealt (after defense)
    float takeDamage(float rawDmg);

private:
    float m_hp, m_maxHp;
    float m_speed;
    float m_defense;
    bool  m_reached = false;

    sf::Vector2f m_pos;
    int          m_waypointIdx = 1;
    const std::vector<sf::Vector2f>* m_waypoints;

    // Animasi
    EnemyAnim    m_anim     = EnemyAnim::Walk;
    sf::Texture* m_texIdle  = nullptr;
    sf::Texture* m_texWalk  = nullptr;
    sf::Texture* m_texHurt  = nullptr;
    sf::Sprite   m_sprite;
    int          m_frame    = 0;
    float        m_animTimer= 0.f;
    bool         m_facingRight = true;

    float        m_hurtTimer  = 0.f; // sekali kena, flash hurt animation
    float        m_deathTimer = 0.f; // setelah mati, finish death anim then remove

    void updateAnim(float dt);
    void applyFrame();
    void drawHPBar(sf::RenderWindow& window) const;
};