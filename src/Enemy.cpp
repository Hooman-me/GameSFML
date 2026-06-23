#include "Enemy.hpp"
#include "Constants.hpp"
#include <cmath>

Enemy::Enemy(const std::vector<sf::Vector2f>* waypoints,
             sf::Texture* texIdle, sf::Texture* texWalk, sf::Texture* texHurt)
    : m_waypoints(waypoints)
    , m_texIdle(texIdle), m_texWalk(texWalk), m_texHurt(texHurt)
{
    m_hp=m_maxHp=ENEMY_HP; m_speed=ENEMY_SPEED; m_defense=ENEMY_DEFENSE;
    m_pos = (*m_waypoints)[0];
    m_sprite.setOrigin(SLIME_FRAME_W*.5f, SLIME_FRAME_H*.5f);
    m_anim=EnemyAnim::Walk;
    applyFrame();
}

float Enemy::takeDamage(float rawDmg) {
    float actual = std::max(1.f, rawDmg - m_defense);
    m_hp = std::max(0.f, m_hp - actual);
    if (m_hp <= 0.f) {
        m_anim=EnemyAnim::Dead; m_frame=0; m_animTimer=0.f;
        m_deathTimer = SLIME_HURT_FRAMES * SLIME_ANIM_TIME;
    } else {
        m_anim=EnemyAnim::Hurt; m_frame=0; m_animTimer=0.f;
        m_hurtTimer = SLIME_HURT_FRAMES * SLIME_ANIM_TIME;
    }
    return actual;
}

void Enemy::update(float dt) {
    if (m_hp <= 0.f) { m_deathTimer -= dt; updateAnim(dt); return; }
    if (m_reached) return;
    if (m_hurtTimer > 0.f) {
        m_hurtTimer -= dt;
        if (m_hurtTimer <= 0.f) { m_anim=EnemyAnim::Walk; m_frame=0; }
    }
    int wSz = (int)m_waypoints->size();
    if (m_waypointIdx >= wSz) { m_reached=true; return; }
    sf::Vector2f target = (*m_waypoints)[m_waypointIdx];
    sf::Vector2f diff   = target - m_pos;
    float dist = std::sqrt(diff.x*diff.x+diff.y*diff.y);
    if (dist < ENEMY_WAYPOINT_DIST) {
        m_pos = target; ++m_waypointIdx;
        if (m_waypointIdx >= wSz) { m_reached=true; return; }
        sf::Vector2f nd = (*m_waypoints)[m_waypointIdx] - m_pos;
        m_facingRight = (nd.x >= 0.f);
    } else {
        m_pos += (diff/dist) * m_speed * dt;
        m_facingRight = (diff.x >= 0.f);
    }
    updateAnim(dt);
}

void Enemy::updateAnim(float dt) {
    int maxF = (m_anim==EnemyAnim::Walk) ? SLIME_WALK_FRAMES : SLIME_HURT_FRAMES;
    m_animTimer += dt;
    if (m_animTimer >= SLIME_ANIM_TIME) {
        m_animTimer -= SLIME_ANIM_TIME;
        if (++m_frame >= maxF) {
            m_frame = 0;
            if (m_anim==EnemyAnim::Hurt) { m_anim=EnemyAnim::Walk; }
        }
    }
    applyFrame();
}

void Enemy::applyFrame() {
    sf::Texture* tex = (m_anim==EnemyAnim::Walk) ? m_texWalk :
                       (m_anim==EnemyAnim::Idle) ? m_texIdle : m_texHurt;
    if (!tex) return;
    int fw=SLIME_FRAME_W, fh=SLIME_FRAME_H;
    m_sprite.setTexture(*tex, true);
    m_sprite.setTextureRect(sf::IntRect(m_frame*fw, 0, fw, fh));
    m_sprite.setOrigin(fw*.5f, fh*.5f);
    m_sprite.setScale(m_facingRight ? 1.f : -1.f, 1.f);
}

sf::FloatRect Enemy::getBounds() const {
    float hw=SLIME_FRAME_W*.45f, hh=SLIME_FRAME_H*.45f;
    return {m_pos.x-hw, m_pos.y-hh, hw*2.f, hh*2.f};
}

void Enemy::draw(sf::RenderWindow& window) const {
    if (m_hp<=0.f && m_deathTimer<=0.f) return;
    sf::Sprite sp=m_sprite;
    sp.setPosition(m_pos);
    sp.setColor(m_anim==EnemyAnim::Hurt ? sf::Color(255,150,150) : sf::Color::White);
    window.draw(sp);
    if (m_hp>0.f) drawHPBar(window);
}

void Enemy::drawHPBar(sf::RenderWindow& window) const {
    float pct=m_hp/m_maxHp;
    float bx=m_pos.x-HPBAR_WIDTH*.5f, by=m_pos.y+HPBAR_OFFSET_Y;
    sf::RectangleShape bg({HPBAR_WIDTH,HPBAR_HEIGHT});
    bg.setFillColor(sf::Color(40,40,40,210)); bg.setPosition(bx,by);
    window.draw(bg);
    sf::RectangleShape bar({HPBAR_WIDTH*pct,HPBAR_HEIGHT});
    bar.setFillColor(sf::Color(0xFF,0x7A,0x00)); bar.setPosition(bx,by);
    window.draw(bar);
}