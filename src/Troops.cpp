#include "Troops.hpp"
#include "Enemy.hpp"
#include "Constants.hpp"
#include <cmath>

static constexpr float PI = 3.14159265f;

Troop::Troop(sf::Vector2f pos, FacingDir facing,
             sf::Texture* texIdle, sf::Texture* texAttack,
             sf::Texture* texHurt, sf::Texture* texFireball)
    : m_pos(pos), m_facing(facing)
    , m_texIdle(texIdle), m_texAttack(texAttack)
    , m_texHurt(texHurt), m_texFireball(texFireball)
{
    m_sprite.setOrigin(CAT_FRAME_W*.5f, CAT_FRAME_H*.5f);
    applyFrame();
}

float Troop::facingAngleRad() const {
    switch (m_facing) {
        case FacingDir::Right: return 0.f;
        case FacingDir::Left:  return PI;
        case FacingDir::Up:    return -PI*.5f;
        case FacingDir::Down:  return  PI*.5f;
    }
    return 0.f;
}

float Troop::angleToTarget(sf::Vector2f target) const {
    sf::Vector2f d = target - m_pos;
    return std::atan2(d.y, d.x);
}

bool Troop::inAttackRange(sf::Vector2f enemyPos) const {
    sf::Vector2f d = enemyPos - m_pos;
    float dist = std::sqrt(d.x*d.x+d.y*d.y);
    if (dist > TROOP_ATTACK_RANGE) return false;
    float ate = std::atan2(d.y, d.x);
    float fov = TROOP_FOV_DEG * PI / 180.f;
    float da  = ate - facingAngleRad();
    while (da >  PI) da -= 2.f*PI;
    while (da < -PI) da += 2.f*PI;
    return std::abs(da) <= fov*.5f;
}

void Troop::update(float dt,
                   const std::vector<Enemy*>& enemies,
                   std::vector<Projectile>& outProjectiles)
{
    if (m_attackCooldown > 0.f) m_attackCooldown -= dt;
    bool fired = false;
    if (m_attackCooldown <= 0.f) {
        Enemy* target = nullptr;
        float closest = 1e9f;
        for (Enemy* e : enemies) {
            if (!e || e->getHP() <= 0.f || e->hasReached()) continue;
            if (!inAttackRange(e->getPosition())) continue;
            sf::Vector2f d = e->getPosition() - m_pos;
            float dist = std::sqrt(d.x*d.x+d.y*d.y);
            if (dist < closest) { closest=dist; target=e; }
        }
        if (target) {
            float angle = angleToTarget(target->getPosition());
            outProjectiles.emplace_back(m_pos, angle, TROOP_ATK, m_texFireball);
            m_attackCooldown = 1.f / TROOP_ATTACK_SPEED;
            m_state=TroopState::Attacking; m_frame=0; m_animTimer=0.f;
            fired=true;
        }
    }
    if (!fired && m_attackCooldown<=0.f && m_state==TroopState::Attacking) {
        m_state=TroopState::Idle; m_frame=0; m_animTimer=0.f;
    }
    updateAnim(dt);
}

void Troop::updateAnim(float dt) {
    int   maxF = (m_state==TroopState::Attacking) ? CAT_ATTACK_FRAMES : CAT_IDLE_FRAMES;
    float ft   = (m_state==TroopState::Attacking) ? CAT_ATTACK_ANIM_TIME : CAT_ANIM_TIME;
    m_animTimer += dt;
    if (m_animTimer >= ft) {
        m_animTimer -= ft;
        if (++m_frame >= maxF) {
            m_frame=0;
            if (m_state==TroopState::Attacking && m_attackCooldown<=0.f)
                m_state=TroopState::Idle;
        }
    }
    applyFrame();
}

void Troop::applyFrame() {
    sf::Texture* tex = (m_state==TroopState::Attacking) ? m_texAttack : m_texIdle;
    if (!tex) return;
    m_sprite.setTexture(*tex, true);
    m_sprite.setTextureRect(sf::IntRect(m_frame*CAT_FRAME_W, 0, CAT_FRAME_W, CAT_FRAME_H));
    m_sprite.setOrigin(CAT_FRAME_W*.5f, CAT_FRAME_H*.5f);
    
    // *** FIXED: Gunakan rotation untuk Up/Down, bukan vertical flip ***
    // Vertical flip (sy = -1) menyebabkan sprite terbalik dengan origin yang salah
    float sx = 1.f, sy = 1.f;
    float rotation = 0.f;
    
    switch (m_facing) {
        case FacingDir::Right:
            sx = 1.f; sy = 1.f; rotation = 0.f;
            break;
        case FacingDir::Left:
            sx = -1.f; sy = 1.f; rotation = 0.f;
            break;
        case FacingDir::Up:
            // Untuk Up, gunakan scale normal + rotation 180
            // Ini membuat sprite menghadap atas tanpa flip vertikal yang aneh
            sx = 1.f; sy = 1.f; rotation = 180.f;
            break;
        case FacingDir::Down:
            sx = 1.f; sy = 1.f; rotation = 0.f;
            break;
    }
    
    m_sprite.setScale(sx, sy);
    m_sprite.setRotation(rotation);
}

void Troop::draw(sf::RenderWindow& window) const {
    sf::Sprite sp=m_sprite;
    sp.setPosition(m_pos);
    window.draw(sp);
}

void Troop::drawRangeIndicator(sf::RenderWindow& window) const {
    float fov=TROOP_FOV_DEG*PI/180.f, base=facingAngleRad();
    sf::VertexArray fan(sf::TriangleFan, 34);
    fan[0].position=m_pos; fan[0].color=sf::Color(255,200,50,35);
    for (int i=0; i<=32; ++i) {
        float a=base-fov*.5f+fov*(float)i/32.f;
        fan[i+1].position={m_pos.x+std::cos(a)*TROOP_ATTACK_RANGE,
                           m_pos.y+std::sin(a)*TROOP_ATTACK_RANGE};
        fan[i+1].color=sf::Color(255,200,50,10);
    }
    window.draw(fan);
    sf::VertexArray lines(sf::Lines,4);
    float aL=base-fov*.5f, aR=base+fov*.5f;
    lines[0].position=m_pos;
    lines[1].position={m_pos.x+std::cos(aL)*TROOP_ATTACK_RANGE,
                       m_pos.y+std::sin(aL)*TROOP_ATTACK_RANGE};
    lines[2].position=m_pos;
    lines[3].position={m_pos.x+std::cos(aR)*TROOP_ATTACK_RANGE,
                       m_pos.y+std::sin(aR)*TROOP_ATTACK_RANGE};
    for (int i=0;i<4;++i) lines[i].color=sf::Color(255,200,50,70);
    window.draw(lines);
}