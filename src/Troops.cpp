#include "Troops.hpp"
#include "Enemy.hpp"
#include "Constants.hpp"
#include <cmath>

static constexpr float PI = 3.14159265f;

Troop::Troop(sf::Vector2f pos, FacingDir facing,
             sf::Texture* texIdle,
             sf::Texture* texAttack,
             sf::Texture* texHurt,
             sf::Texture* texFireball)
    : m_pos(pos), m_facing(facing)
    , m_texIdle(texIdle), m_texAttack(texAttack)
    , m_texHurt(texHurt), m_texFireball(texFireball)
{
    m_sprite.setOrigin(CAT_FRAME_W * .5f, CAT_FRAME_H * .5f);
    // Mirror sprite if facing left
    if (facing == FacingDir::Left)
        m_sprite.setScale(-1.f, 1.f);
    applyFrame();
}

float Troop::facingAngleRad() const {
    return (m_facing == FacingDir::Right) ? 0.f : PI;
}

float Troop::angleToTarget(sf::Vector2f target) const {
    sf::Vector2f diff = target - m_pos;
    return std::atan2(diff.y, diff.x);
}

bool Troop::inAttackRange(sf::Vector2f enemyPos) const {
    sf::Vector2f diff = enemyPos - m_pos;
    float dist = std::sqrt(diff.x*diff.x + diff.y*diff.y);
    if (dist > TROOP_ATTACK_RANGE) return false;

    float angleToEnemy = std::atan2(diff.y, diff.x);
    float fov = TROOP_FOV_DEG * PI / 180.f;
    float da  = angleToEnemy - facingAngleRad();
    while (da >  PI) da -= 2.f*PI;
    while (da < -PI) da += 2.f*PI;
    return std::abs(da) <= fov * .5f;
}

void Troop::update(float dt,
                   const std::vector<Enemy*>& enemies,
                   std::vector<Projectile>& outProjectiles)
{
    if (m_attackCooldown > 0.f) m_attackCooldown -= dt;

    bool fired = false;
    if (m_attackCooldown <= 0.f) {
        Enemy* target = nullptr;
        float  closestDist = 1e9f;

        // Find closest enemy in range (prioritize frontmost = highest waypoint index)
        for (Enemy* e : enemies) {
            if (!e || e->isDead() || e->hasReached()) continue;
            if (!inAttackRange(e->getPosition())) continue;
            sf::Vector2f d = e->getPosition() - m_pos;
            float dist = std::sqrt(d.x*d.x + d.y*d.y);
            if (dist < closestDist) {
                closestDist = dist;
                target = e;
            }
        }

        if (target) {
            // FIX: Tembak menuju posisi MUSUH yang sebenarnya (bukan hanya ke kiri/kanan)
            float angle = angleToTarget(target->getPosition());
            outProjectiles.emplace_back(m_pos, angle, TROOP_ATK, m_texFireball);
            m_attackCooldown = 1.f / TROOP_ATTACK_SPEED;
            m_state = TroopState::Attacking;
            m_frame = 0;
            m_animTimer = 0.f;
            fired = true;
        }
    }

    if (!fired && m_attackCooldown <= 0.f && m_state == TroopState::Attacking) {
        m_state = TroopState::Idle;
        m_frame = 0;
        m_animTimer = 0.f;
    }

    updateAnim(dt);
}

void Troop::updateAnim(float dt) {
    int   maxFrames = CAT_IDLE_FRAMES;
    float frameTime = CAT_ANIM_TIME;

    switch (m_state) {
        case TroopState::Idle:
            maxFrames = CAT_IDLE_FRAMES;
            frameTime = CAT_ANIM_TIME;
            break;
        case TroopState::Attacking:
            maxFrames = CAT_ATTACK_FRAMES;
            frameTime = CAT_ATTACK_ANIM_TIME;
            break;
        case TroopState::Hurt:
            maxFrames = CAT_HURT_FRAMES;
            frameTime = CAT_ANIM_TIME;
            break;
    }

    m_animTimer += dt;
    if (m_animTimer >= frameTime) {
        m_animTimer -= frameTime;
        m_frame++;
        if (m_frame >= maxFrames) {
            m_frame = 0;
            // After attack anim completes, back to idle
            if (m_state == TroopState::Attacking && m_attackCooldown <= 0.f)
                m_state = TroopState::Idle;
        }
    }
    applyFrame();
}

void Troop::applyFrame() {
    sf::Texture* tex = nullptr;
    int fw = CAT_FRAME_W, fh = CAT_FRAME_H;

    switch (m_state) {
        case TroopState::Idle:      tex = m_texIdle;   break;
        case TroopState::Attacking: tex = m_texAttack; break;
        case TroopState::Hurt:      tex = m_texHurt;   break;
    }
    if (!tex) { tex = m_texIdle; }
    if (!tex) return;

    m_sprite.setTexture(*tex, true);
    m_sprite.setTextureRect(sf::IntRect(m_frame * fw, 0, fw, fh));
    m_sprite.setOrigin(fw * .5f, fh * .5f);
    m_sprite.setScale((m_facing == FacingDir::Right) ? 1.f : -1.f, 1.f);
}

void Troop::draw(sf::RenderWindow& window) const {
    sf::Sprite sp = m_sprite;
    sp.setPosition(m_pos);
    window.draw(sp);
}

void Troop::drawRangeIndicator(sf::RenderWindow& window) const {
    float fov  = TROOP_FOV_DEG * PI / 180.f;
    float base = facingAngleRad();
    int   steps = 32;

    sf::VertexArray fan(sf::TriangleFan, steps + 2);
    fan[0].position = m_pos;
    fan[0].color    = sf::Color(255, 200, 50, 35);
    for (int i = 0; i <= steps; ++i) {
        float a = base - fov*.5f + fov*(float)i/(float)steps;
        fan[i+1].position = {
            m_pos.x + std::cos(a) * TROOP_ATTACK_RANGE,
            m_pos.y + std::sin(a) * TROOP_ATTACK_RANGE
        };
        fan[i+1].color = sf::Color(255, 200, 50, 10);
    }
    window.draw(fan);

    sf::VertexArray lines(sf::Lines, 4);
    float aL = base - fov*.5f, aR = base + fov*.5f;
    lines[0].position = m_pos;
    lines[1].position = { m_pos.x + std::cos(aL)*TROOP_ATTACK_RANGE,
                          m_pos.y + std::sin(aL)*TROOP_ATTACK_RANGE };
    lines[2].position = m_pos;
    lines[3].position = { m_pos.x + std::cos(aR)*TROOP_ATTACK_RANGE,
                          m_pos.y + std::sin(aR)*TROOP_ATTACK_RANGE };
    for (int i = 0; i < 4; ++i) lines[i].color = sf::Color(255, 200, 50, 70);
    window.draw(lines);
}