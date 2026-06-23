#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include "Projectile.hpp"

enum class TroopState   { Idle, Attacking, Hurt };
// *** DIUBAH: 4 arah (panah keyboard) ***
enum class FacingDir    { Left, Right, Up, Down };

class Enemy;

class Troop {
public:
    Troop(sf::Vector2f pos, FacingDir facing,
          sf::Texture* texIdle,
          sf::Texture* texAttack,
          sf::Texture* texHurt,
          sf::Texture* texFireball);

    void update(float dt,
                const std::vector<Enemy*>& enemies,
                std::vector<Projectile>& outProjectiles);

    void draw(sf::RenderWindow& window) const;
    void drawRangeIndicator(sf::RenderWindow& window) const;
    sf::Vector2f getPosition() const { return m_pos; }

private:
    sf::Vector2f  m_pos;
    FacingDir     m_facing;
    TroopState    m_state         = TroopState::Idle;
    float         m_attackCooldown= 0.f;

    sf::Texture*  m_texIdle    = nullptr;
    sf::Texture*  m_texAttack  = nullptr;
    sf::Texture*  m_texHurt    = nullptr;
    sf::Texture*  m_texFireball= nullptr;
    sf::Sprite    m_sprite;
    int           m_frame      = 0;
    float         m_animTimer  = 0.f;

    bool  inAttackRange(sf::Vector2f enemyPos) const;
    float facingAngleRad() const;
    float angleToTarget(sf::Vector2f target) const;
    void  updateAnim(float dt);
    void  applyFrame();
};