#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <memory>
#include "Map.hpp"
#include "Enemy.hpp"
#include "Troops.hpp"
#include "Projectile.hpp"

// ============================================================
//  PLAYSTATE.HPP  –  Otak utama game: loop, HUD, input
// ============================================================

enum class GamePhase { Playing, GameOver, Victory };

class PlayState {
public:
    PlayState();

    void handleEvent(const sf::Event& event, sf::RenderWindow& window);
    void update(float dt);
    void draw(sf::RenderWindow& window);

private:
    // --- Subsistem ---
    Map m_map;

    // --- Entities ---
    std::vector<Enemy>      m_enemies;
    std::vector<Troop>      m_troops;
    std::vector<Projectile> m_projectiles;

    // --- Textures (nullptr = pakai placeholder) ---
    sf::Texture m_troopTex;
    sf::Texture m_enemyTex;
    bool        m_troopTexLoaded = false;
    bool        m_enemyTexLoaded = false;

    // --- Font ---
    sf::Font    m_font;
    bool        m_fontLoaded = false;

    // --- HUD data ---
    int   m_money    = MONEY_START;
    int   m_baseHP   = BASE_MAX_HP;
    int   m_enemyCount = 0;        // musuh yang sudah muncul
    float m_moneyTimer = 0.f;
    GamePhase m_phase = GamePhase::Playing;

    // --- Wave / spawn ---
    float m_spawnTimer   = 0.f;
    int   m_spawned      = 0;
    int   m_killed       = 0;

    // --- Deploy cursor state ---
    int   m_selectedCard = -1;     // indeks kartu yang dipilih (-1 = tidak ada)
    bool  m_placing      = false;  // mode "sedang menaruh troop"

    // --- Metode privat ---
    void spawnEnemy();
    void updateCollisions();
    void tryDeployTroop(sf::Vector2f worldPos);

    // UI drawing
    void drawHUD(sf::RenderWindow& window);
    void drawUIBar(sf::RenderWindow& window);
    void drawDeployCursor(sf::RenderWindow& window, sf::Vector2f mousePos);
    void drawGameOver(sf::RenderWindow& window);

    // Helper
    sf::Text makeText(const std::string& str, unsigned size,
                      sf::Color col, sf::Vector2f pos);

    // Kartu troop di UI bar
    struct TroopCard {
        std::string name;
        int         cost;
        sf::Color   color;  // placeholder warna kartu
    };
    std::vector<TroopCard> m_cards;
    void buildCards();
};