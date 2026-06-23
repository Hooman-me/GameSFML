#include "Projectile.hpp"
#include "Constants.hpp"
#include <cmath>

Projectile::Projectile(sf::Vector2f origin, float angleRad, float damage,
                       sf::Texture* texFireball)
    : m_pos(origin), m_angleRad(angleRad), m_damage(damage), m_texFireball(texFireball)
{
    m_velocity = {std::cos(angleRad)*PROJ_SPEED, std::sin(angleRad)*PROJ_SPEED};
    if (m_texFireball) {
        m_sprite.setTexture(*m_texFireball);
        m_sprite.setTextureRect(sf::IntRect(0,0,FIREBALL_FRAME_W,FIREBALL_FRAME_H));
        m_sprite.setOrigin(FIREBALL_FRAME_W*.5f, FIREBALL_FRAME_H*.5f);
        m_sprite.setScale(FIREBALL_SCALE, FIREBALL_SCALE);
        m_sprite.setRotation(angleRad * 180.f / 3.14159265f);
    }
}

void Projectile::update(float dt) {
    if (m_state==ProjState::Flying) {
        m_pos += m_velocity*dt;
        if (m_texFireball) {
            m_animTimer += dt;
            if (m_animTimer >= FIREBALL_ANIM_TIME) {
                m_animTimer -= FIREBALL_ANIM_TIME;
                m_frame = (m_frame+1) % FIREBALL_FRAMES;
                m_sprite.setTextureRect(sf::IntRect(m_frame*FIREBALL_FRAME_W,0,
                    FIREBALL_FRAME_W,FIREBALL_FRAME_H));
            }
        }
    } else if (m_state==ProjState::Exploding) {
        m_explodeTimer += dt;
        m_explodeRadius = (m_explodeTimer/EXPLODE_DURATION)*40.f;
        if (m_explodeTimer>=EXPLODE_DURATION) m_state=ProjState::Done;
    }
}

void Projectile::triggerExplosion() {
    if (m_state==ProjState::Flying) {
        m_state=ProjState::Exploding; m_explodeTimer=0.f; m_explodeRadius=0.f;
    }
}

sf::FloatRect Projectile::getBounds() const {
    float r=FIREBALL_FRAME_W*FIREBALL_SCALE*.5f;
    return {m_pos.x-r, m_pos.y-r, r*2.f, r*2.f};
}

void Projectile::draw(sf::RenderWindow& window) const {
    if (m_state==ProjState::Done) return;
    if (m_state==ProjState::Flying) {
        if (m_texFireball) {
            sf::Sprite sp=m_sprite; sp.setPosition(m_pos); window.draw(sp);
        } else {
            sf::CircleShape ball(8.f);
            ball.setFillColor(sf::Color(255,160,20));
            ball.setOrigin(8.f,8.f); ball.setPosition(m_pos);
            window.draw(ball);
        }
    } else {
        float t=m_explodeTimer/EXPLODE_DURATION;
        sf::Uint8 a=(sf::Uint8)(255*(1.f-t));
        float r=m_explodeRadius;
        sf::CircleShape outer(r);
        outer.setFillColor(sf::Color(255,120,0,(sf::Uint8)(a*.5f)));
        outer.setOutlineColor(sf::Color(255,60,0,a));
        outer.setOutlineThickness(3.f);
        outer.setOrigin(r,r); outer.setPosition(m_pos);
        window.draw(outer);
        float ri=r*.5f;
        sf::CircleShape inner(ri);
        inner.setFillColor(sf::Color(255,240,100,a));
        inner.setOrigin(ri,ri); inner.setPosition(m_pos);
        window.draw(inner);
    }
}