#include "Troops.hpp"
#include "../include/Enemy.hpp"
#include "../include/Constants.hpp"
#include <cmath>

// ============================================================
//  TROOPS.CPP
// ============================================================

static constexpr float PI = 3.14159265f;

Troop::Troop(sf::Vector2f pos, FacingDir facing, sf::Texture* tex)
    : m_pos(pos), m_facing(facing), m_tex(tex)
{
    m_attackCooldown = 0.f;

    if (m_tex) {
        int row = (facing == FacingDir::Right) ? SPRITE_ROW_RIGHT : SPRITE_ROW_LEFT;
        m_sprite.setTexture(*m_tex);
        m_sprite.setTextureRect(sf::IntRect(0, row * SPRITE_FRAME_H,
                                            SPRITE_FRAME_W, SPRITE_FRAME_H));
        m_sprite.setOrigin(SPRITE_FRAME_W * 0.5f, SPRITE_FRAME_H * 0.5f);
        m_sprite.setScale(1.f, 1.f); // *** ubah skala sprite troop ***
    }
}

// Sudut tengah FOV dalam radian
float Troop::facingAngleRad() const {
    return (m_facing == FacingDir::Right) ? 0.f : PI;
}

// Cek apakah musuh masuk kerucut serang
bool Troop::inAttackRange(sf::Vector2f enemyPos) const {
    sf::Vector2f diff = enemyPos - m_pos;
    float dist = std::sqrt(diff.x * diff.x + diff.y * diff.y);

    // Saringan 1: jarak
    if (dist > TROOP_ATTACK_RANGE) return false;

    // Saringan 2: sudut
    float angleToEnemy = std::atan2(diff.y, diff.x); // -PI..+PI
    float fov = TROOP_FOV_DEG * PI / 180.f;

    // Hitung selisih sudut (dalam rentang -PI..+PI)
    float da = angleToEnemy - facingAngleRad();
    // Normalisasi
    while (da >  PI) da -= 2.f * PI;
    while (da < -PI) da += 2.f * PI;

    return std::abs(da) <= fov * 0.5f;
}

void Troop::update(float dt,
                   const std::vector<Enemy*>& enemies,
                   std::vector<Projectile>& outProjectiles)
{
    // Kurangi cooldown
    if (m_attackCooldown > 0.f) m_attackCooldown -= dt;

    bool attacking = false;

    // Cari musuh pertama yang dalam jangkauan
    if (m_attackCooldown <= 0.f) {
        for (Enemy* e : enemies) {
            if (!e || e->isDead() || e->hasReached()) continue;
            if (inAttackRange(e->getPosition())) {
                // Tembak! Peluru terbang lurus ke arah hadap (bukan homing)
                outProjectiles.emplace_back(m_pos, facingAngleRad(), TROOP_ATK);
                m_attackCooldown = 1.f / TROOP_ATTACK_SPEED;
                m_state  = TroopState::Attacking;
                attacking = true;
                break;
            }
        }
    }

    if (!attacking && m_attackCooldown <= 0.f) {
        m_state = TroopState::Idle;
    }

    updateAnimation(dt, attacking);
}

void Troop::updateAnimation(float dt, bool attacking) {
    if (!m_tex) return;

    int row = (m_facing == FacingDir::Right) ? SPRITE_ROW_RIGHT : SPRITE_ROW_LEFT;
    // Saat attacking bisa pakai baris berbeda jika spritesheet mendukung
    // (untuk sekarang pakai baris yang sama, hanya frame lebih cepat)
    float speed = attacking ? ANIM_FRAME_TIME * 0.5f : ANIM_FRAME_TIME;

    m_animTimer += dt;
    if (m_animTimer >= speed) {
        m_animTimer -= speed;
        m_frameCol = (m_frameCol + 1) % SPRITE_FRAMES;
    }

    m_sprite.setTextureRect(sf::IntRect(
        m_frameCol * SPRITE_FRAME_W,
        row * SPRITE_FRAME_H,
        SPRITE_FRAME_W,
        SPRITE_FRAME_H
    ));
}

void Troop::draw(sf::RenderWindow& window) const {
    if (m_tex) {
        sf::Sprite sp = m_sprite;
        sp.setPosition(m_pos);
        window.draw(sp);
    } else {
        drawPlaceholder(window);
    }
}

void Troop::drawPlaceholder(sf::RenderWindow& window) const {
    // Body: lingkaran hijau (troop)
    sf::CircleShape body(20.f);
    body.setFillColor(sf::Color(50, 180, 80));
    body.setOutlineColor(sf::Color(20, 100, 40));
    body.setOutlineThickness(2.5f);
    body.setOrigin(20.f, 20.f);
    body.setPosition(m_pos);
    window.draw(body);

    // Telinga kecil (karakter hewan)
    sf::CircleShape ear(6.f);
    ear.setFillColor(sf::Color(40, 150, 60));
    ear.setOrigin(6.f, 6.f);
    ear.setPosition(m_pos.x - 10.f, m_pos.y - 22.f);
    window.draw(ear);
    ear.setPosition(m_pos.x + 10.f, m_pos.y - 22.f);
    window.draw(ear);

    // Panah arah hadap
    sf::RectangleShape arrow({ 22.f, 4.f });
    arrow.setFillColor(sf::Color::Yellow);
    arrow.setOrigin(0.f, 2.f);
    arrow.setPosition(m_pos);
    arrow.setRotation((m_facing == FacingDir::Right) ? 0.f : 180.f);
    window.draw(arrow);
}

void Troop::drawRangeIndicator(sf::RenderWindow& window) const {
    // Gambar kerucut FOV sebagai setengah-lingkaran transparan
    float fov   = TROOP_FOV_DEG * PI / 180.f;
    float base  = facingAngleRad();
    int   steps = 30;

    sf::VertexArray fan(sf::TriangleFan, steps + 2);
    fan[0].position = m_pos;
    fan[0].color    = sf::Color(255, 255, 100, 40);

    for (int i = 0; i <= steps; ++i) {
        float a = base - fov * 0.5f + fov * (float)i / (float)steps;
        fan[i + 1].position = {
            m_pos.x + std::cos(a) * TROOP_ATTACK_RANGE,
            m_pos.y + std::sin(a) * TROOP_ATTACK_RANGE
        };
        fan[i + 1].color = sf::Color(255, 255, 100, 15);
    }
    window.draw(fan);

    // Garis batas kerucut
    sf::VertexArray lines(sf::Lines, 4);
    float aL = base - fov * 0.5f, aR = base + fov * 0.5f;
    lines[0].position = m_pos;
    lines[1].position = { m_pos.x + std::cos(aL)*TROOP_ATTACK_RANGE,
                          m_pos.y + std::sin(aL)*TROOP_ATTACK_RANGE };
    lines[2].position = m_pos;
    lines[3].position = { m_pos.x + std::cos(aR)*TROOP_ATTACK_RANGE,
                          m_pos.y + std::sin(aR)*TROOP_ATTACK_RANGE };
    for (int i = 0; i < 4; ++i) lines[i].color = sf::Color(255, 255, 100, 80);
    window.draw(lines);
}