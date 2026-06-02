#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include "Projectile.hpp"

// ============================================================
//  TROOPS.HPP  –  Data & logika satu troop yang di-deploy
// ============================================================

enum class TroopState   { Idle, Attacking };
enum class FacingDir    { Left, Right };   // 2 arah deploy utama

class Enemy; // forward declaration

class Troop {
public:
    Troop(sf::Vector2f pos, FacingDir facing, sf::Texture* tex);

    // Dipanggil tiap frame; mengisi outProjectiles jika perlu tembak
    void update(float dt,
                const std::vector<class Enemy*>& enemies,
                std::vector<Projectile>& outProjectiles);

    void draw(sf::RenderWindow& window) const;

    sf::Vector2f getPosition() const { return m_pos; }

    // Gambar indikator radius serang (debug / preview)
    void drawRangeIndicator(sf::RenderWindow& window) const;

private:
    sf::Vector2f  m_pos;
    FacingDir     m_facing;
    TroopState    m_state   = TroopState::Idle;

    float m_attackCooldown  = 0.f;   // waktu tunggu sampai bisa tembak lagi

    // Sprite / animasi
    sf::Texture*  m_tex     = nullptr;
    sf::Sprite    m_sprite;
    int           m_frameCol= 0;
    float         m_animTimer=0.f;

    // --- Sensor / FOV ---
    // Menentukan apakah musuh masuk ke dalam kerucut serang
    bool inAttackRange(sf::Vector2f enemyPos) const;
    float facingAngleRad() const;  // sudut tengah FOV dalam radian

    // Animasi
    void updateAnimation(float dt, bool attacking);
    void drawHPBar(sf::RenderWindow& window) const; // troops tidak ada HP, tapi bisa diperluas
    void drawPlaceholder(sf::RenderWindow& window) const;
};