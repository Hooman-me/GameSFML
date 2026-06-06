#include "../include/Enemy.hpp"
#include "../include/Constants.hpp"
#include <cmath>

// ============================================================
//  ENEMY.CPP
// ============================================================

Enemy::Enemy(const std::vector<sf::Vector2f>& waypoints, sf::Texture* tex)
    : m_waypoints(waypoints), m_tex(tex)
{
    m_hp      = ENEMY_HP;
    m_maxHp   = ENEMY_HP;
    m_speed   = ENEMY_SPEED;
    m_defense = ENEMY_DEFENSE;

    m_pos = waypoints[0];

    if (m_tex) {
        m_sprite.setTexture(*m_tex);
        m_sprite.setTextureRect(sf::IntRect(0, SPRITE_ROW_RIGHT * SPRITE_FRAME_H,
                                            SPRITE_FRAME_W, SPRITE_FRAME_H));
        m_sprite.setOrigin(SPRITE_FRAME_W * 0.5f, SPRITE_FRAME_H * 0.5f);
        m_sprite.setScale(1.f, 1.f); // *** ubah skala sprite musuh ***
    }
}

void Enemy::update(float dt) {
    if (isDead() || m_reached) return;

    // Menuju waypoint berikutnya
    if (m_waypointIdx >= (int)m_waypoints.size()) {
        m_reached = true;
        return;
    }

    sf::Vector2f target = m_waypoints[m_waypointIdx];
    sf::Vector2f diff   = target - m_pos;
    float dist = std::sqrt(diff.x * diff.x + diff.y * diff.y);

    sf::Vector2f dir = { 0.f, 0.f };
    if (dist > 0.01f) dir = { diff.x / dist, diff.y / dist };

    if (dist < ENEMY_WAYPOINT_DIST) {
        m_pos = target;
        m_waypointIdx++;
        if (m_waypointIdx >= (int)m_waypoints.size()) {
            m_reached = true;
            return;
        }
    } else {
        m_pos += dir * m_speed * dt;
    }

    updateAnimation(dt, dir);
}

void Enemy::updateAnimation(float dt, sf::Vector2f dir) {
    if (!m_tex) return;

    // Tentukan baris sprite berdasarkan arah gerak dominan
    if (std::abs(dir.x) >= std::abs(dir.y)) {
        m_spriteRow = (dir.x >= 0.f) ? SPRITE_ROW_RIGHT : SPRITE_ROW_LEFT;
    } else {
        m_spriteRow = (dir.y >= 0.f) ? SPRITE_ROW_DOWN : SPRITE_ROW_UP;
    }

    m_animTimer += dt;
    if (m_animTimer >= ANIM_FRAME_TIME) {
        m_animTimer -= ANIM_FRAME_TIME;
        m_frameCol = (m_frameCol + 1) % SPRITE_FRAMES;
    }

    m_sprite.setTextureRect(sf::IntRect(
        m_frameCol * SPRITE_FRAME_W,
        m_spriteRow * SPRITE_FRAME_H,
        SPRITE_FRAME_W,
        SPRITE_FRAME_H
    ));
}

float Enemy::takeDamage(float rawDmg) {
    // Arknights flat-reduction dengan minimal 1
    float actual = std::max(1.f, rawDmg - m_defense);
    m_hp -= actual;
    if (m_hp < 0.f) m_hp = 0.f;
    return actual;
}

sf::FloatRect Enemy::getBounds() const {
    float hw = SPRITE_FRAME_W * 0.5f;
    float hh = SPRITE_FRAME_H * 0.5f;
    return { m_pos.x - hw, m_pos.y - hh, (float)SPRITE_FRAME_W, (float)SPRITE_FRAME_H };
}

void Enemy::draw(sf::RenderWindow& window) const {
    if (isDead()) return;

    if (m_tex) {
        sf::Sprite sp = m_sprite;
        sp.setPosition(m_pos);
        window.draw(sp);
    } else {
        drawPlaceholder(window);
    }
    drawHPBar(window);
}

void Enemy::drawPlaceholder(sf::RenderWindow& window) const {
    // Placeholder: lingkaran merah
    sf::CircleShape body(20.f);
    body.setFillColor(sf::Color(200, 60, 60));
    body.setOutlineColor(sf::Color(120, 20, 20));
    body.setOutlineThickness(2.f);
    body.setOrigin(20.f, 20.f);
    body.setPosition(m_pos);
    window.draw(body);

    // Mata putih kecil
    sf::CircleShape eye(4.f);
    eye.setFillColor(sf::Color::White);
    eye.setOrigin(4.f, 4.f);
    eye.setPosition(m_pos.x + 7.f, m_pos.y - 5.f);
    window.draw(eye);
    eye.setPosition(m_pos.x - 7.f, m_pos.y - 5.f);
    window.draw(eye);
}

void Enemy::drawHPBar(sf::RenderWindow& window) const {
    float pct = m_hp / m_maxHp;
    float bx  = m_pos.x - HPBAR_WIDTH * 0.5f;
    float by  = m_pos.y + HPBAR_OFFSET_Y;

    // Latar belakang gelap
    sf::RectangleShape bg({ HPBAR_WIDTH, HPBAR_HEIGHT });
    bg.setFillColor(sf::Color(50, 50, 50, 200));
    bg.setPosition(bx, by);
    window.draw(bg);

    // Bar oren (musuh)
    sf::RectangleShape bar({ HPBAR_WIDTH * pct, HPBAR_HEIGHT });
    bar.setFillColor(sf::Color(0xFF, 0x7A, 0x00)); // *** warna oren HP bar musuh ***
    bar.setPosition(bx, by);
    window.draw(bar);
}