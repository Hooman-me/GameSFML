#include "../include/Enemy.hpp"
#include "../include/Constants.hpp"
#include <cmath>
#include <algorithm> // Ditambahkan untuk std::max

Enemy::Enemy(const std::vector<sf::Vector2f>& waypoints,
             sf::Texture* texIdle,
             sf::Texture* texWalk,
             sf::Texture* texHurt)
    : m_waypoints(&waypoints) // FIX: Mengambil alamat memori dari waypoints
    , m_texIdle(texIdle), m_texWalk(texWalk), m_texHurt(texHurt)
{
    m_hp      = ENEMY_HP;
    m_maxHp   = ENEMY_HP;
    m_speed   = ENEMY_SPEED;
    m_defense = ENEMY_DEFENSE;
    m_pos     = waypoints[0];

    // GiantSlime default scale (62x52 per frame)
    m_sprite.setOrigin(SLIME_FRAME_W * .5f, SLIME_FRAME_H * .5f);
    m_anim = EnemyAnim::Walk;
    applyFrame();
}

float Enemy::takeDamage(float rawDmg) {
    // Flat reduction (Arknights-style), minimum 1
    float actual = std::max(1.f, rawDmg - m_defense);
    m_hp -= actual;
    if (m_hp < 0.f) m_hp = 0.f;

    if (m_hp <= 0.f) {
        m_anim       = EnemyAnim::Dead;
        m_frame      = 0;
        m_animTimer  = 0.f;
        m_deathTimer = SLIME_IDLE_FRAMES * SLIME_ANIM_TIME; // show death anim
    } else {
        m_anim      = EnemyAnim::Hurt;
        m_frame     = 0;
        m_animTimer = 0.f;
        m_hurtTimer = SLIME_HURT_FRAMES * SLIME_ANIM_TIME;
    }
    return actual;
}

void Enemy::update(float dt) {
    if (m_hp <= 0.f) {
        m_deathTimer -= dt;
        updateAnim(dt);
        return;
    }
    if (m_reached) return;

    // Hurt timer
    if (m_hurtTimer > 0.f) {
        m_hurtTimer -= dt;
        if (m_hurtTimer <= 0.f) {
            m_anim  = EnemyAnim::Walk;
            m_frame = 0;
        }
    }

    // Move toward next waypoint
    if (m_waypointIdx >= (int)m_waypoints->size()) {
        m_reached = true;
        return;
    }
    
    // FIX: Dereference pointer untuk mengakses index
    sf::Vector2f target = (*m_waypoints)[m_waypointIdx];
    sf::Vector2f diff   = target - m_pos;
    float dist = std::sqrt(diff.x*diff.x + diff.y*diff.y);

    if (dist < ENEMY_WAYPOINT_DIST) {
        m_pos = target;
        m_waypointIdx++;
        
        // FIX: Gunakan panah -> untuk method size()
        if (m_waypointIdx >= (int)m_waypoints->size()) {
            m_reached = true;
            return;
        }
        
        // Update facing after waypoint change
        // FIX: Dereference pointer untuk mengakses index
        sf::Vector2f nd = (*m_waypoints)[m_waypointIdx] - m_pos;
        m_facingRight = (nd.x >= 0.f);
    } else {
        sf::Vector2f dir = { diff.x/dist, diff.y/dist };
        m_pos += dir * m_speed * dt;
        m_facingRight = (diff.x >= 0.f);
    }

    updateAnim(dt);
}

void Enemy::updateAnim(float dt) {
    int maxFrames = SLIME_WALK_FRAMES;
    float frameTime = SLIME_ANIM_TIME;

    switch (m_anim) {
        case EnemyAnim::Idle: maxFrames = SLIME_IDLE_FRAMES; break;
        case EnemyAnim::Walk: maxFrames = SLIME_WALK_FRAMES; break;
        case EnemyAnim::Hurt: maxFrames = SLIME_HURT_FRAMES; break;
        case EnemyAnim::Dead: maxFrames = SLIME_HURT_FRAMES; break; // reuse hurt anim
    }

    m_animTimer += dt;
    if (m_animTimer >= frameTime) {
        m_animTimer -= frameTime;
        m_frame++;
        if (m_frame >= maxFrames) {
            if (m_anim == EnemyAnim::Hurt) {
                // loop back to walk after hurt
                m_anim  = EnemyAnim::Walk;
                m_frame = 0;
            } else {
                m_frame = 0; // loop all others
            }
        }
    }
    applyFrame();
}

void Enemy::applyFrame() {
    sf::Texture* tex = nullptr;
    int frameW = SLIME_FRAME_W;
    int frameH = SLIME_FRAME_H;

    switch (m_anim) {
        case EnemyAnim::Idle: tex = m_texIdle; break;
        case EnemyAnim::Walk: tex = m_texWalk; break;
        case EnemyAnim::Hurt:
        case EnemyAnim::Dead: tex = m_texHurt; break;
    }
    if (!tex) return;

    m_sprite.setTexture(*tex, true);
    m_sprite.setTextureRect(sf::IntRect(m_frame * frameW, 0, frameW, frameH));
    m_sprite.setOrigin(frameW * .5f, frameH * .5f);

    // Mirror based on facing direction
    float scaleX = m_facingRight ? 1.f : -1.f;
    m_sprite.setScale(scaleX, 1.f);
}

sf::FloatRect Enemy::getBounds() const {
    float hw = SLIME_FRAME_W * .45f;
    float hh = SLIME_FRAME_H * .45f;
    return { m_pos.x - hw, m_pos.y - hh, hw*2.f, hh*2.f };
}

void Enemy::draw(sf::RenderWindow& window) const {
    if (m_hp <= 0.f && m_deathTimer <= 0.f) return;

    sf::Sprite sp = m_sprite;
    sp.setPosition(m_pos);

    // Flash red when hurt
    if (m_anim == EnemyAnim::Hurt)
        sp.setColor(sf::Color(255, 150, 150));
    else
        sp.setColor(sf::Color::White);

    window.draw(sp);
    if (m_hp > 0.f) drawHPBar(window);
}

void Enemy::drawHPBar(sf::RenderWindow& window) const {
    float pct = m_hp / m_maxHp;
    float bx  = m_pos.x - HPBAR_WIDTH * .5f;
    float by  = m_pos.y + HPBAR_OFFSET_Y;

    sf::RectangleShape bg({ HPBAR_WIDTH, HPBAR_HEIGHT });
    bg.setFillColor(sf::Color(40, 40, 40, 210));
    bg.setPosition(bx, by);
    window.draw(bg);

    sf::RectangleShape bar({ HPBAR_WIDTH * pct, HPBAR_HEIGHT });
    bar.setFillColor(sf::Color(0xFF, 0x7A, 0x00)); // oranye
    bar.setPosition(bx, by);
    window.draw(bar);
}