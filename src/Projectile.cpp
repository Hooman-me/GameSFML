#include "Projectile.hpp"
#include "Constants.hpp"
#include <cmath>

// ============================================================
//  PROJECTILE.CPP
// ============================================================

Projectile::Projectile(sf::Vector2f origin, float angleRad, float damage)
    : m_pos(origin), m_damage(damage)
{
    m_velocity = {
        std::cos(angleRad) * PROJ_SPEED,
        std::sin(angleRad) * PROJ_SPEED
    };
}

void Projectile::update(float dt) {
    if (m_state == ProjState::Flying) {
        m_pos      += m_velocity * dt;
        m_rotAngle += 360.f * dt * 2.f; // rotasi visual peluru
    } else if (m_state == ProjState::Exploding) {
        m_explodeTimer += dt;
        if (m_explodeTimer >= EXPLODE_DURATION) {
            m_state = ProjState::Done;
        }
    }
}

void Projectile::triggerExplosion() {
    if (m_state == ProjState::Flying) {
        m_state = ProjState::Exploding;
        m_explodeTimer = 0.f;
    }
}

sf::FloatRect Projectile::getBounds() const {
    float r = PROJ_SIZE;
    return { m_pos.x - r, m_pos.y - r, r * 2.f, r * 2.f };
}

void Projectile::draw(sf::RenderWindow& window) const {
    if (m_state == ProjState::Done) return;

    if (m_state == ProjState::Flying) {
        // Peluru: diamond kuning berputar
        sf::RectangleShape bullet({ PROJ_SIZE * 1.4f, PROJ_SIZE * 1.4f });
        bullet.setFillColor(sf::Color(255, 230, 50));      // *** warna peluru ***
        bullet.setOutlineColor(sf::Color(200, 140, 0));
        bullet.setOutlineThickness(1.5f);
        bullet.setOrigin(PROJ_SIZE * 0.7f, PROJ_SIZE * 0.7f);
        bullet.setPosition(m_pos);
        bullet.setRotation(m_rotAngle);
        window.draw(bullet);

    } else if (m_state == ProjState::Exploding) {
        // Ledakan: lingkaran oranye yang membesar lalu memudar
        float t   = m_explodeTimer / EXPLODE_DURATION;     // 0→1
        float rad = PROJ_SIZE * (1.f + t * 3.f);
        sf::Uint8 alpha = (sf::Uint8)(255 * (1.f - t));

        sf::CircleShape exp(rad);
        exp.setFillColor(sf::Color(255, 140, 0, alpha));   // *** warna ledakan ***
        exp.setOutlineColor(sf::Color(255, 60, 0, alpha));
        exp.setOutlineThickness(2.f);
        exp.setOrigin(rad, rad);
        exp.setPosition(m_pos);
        window.draw(exp);
    }
}