#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include "Map.hpp"
#include "Enemy.hpp"
#include "Troops.hpp"
#include "Projectile.hpp"
#include "Constants.hpp"

enum class GamePhase { Playing, GameOver, Victory };

// Damage text popup
struct DmgText {
    sf::Vector2f pos;
    float        value;
    float        life   = 1.0f; // seconds
    float        alpha  = 255.f;
};

class PlayState {
public:
    PlayState();

    void handleEvent(const sf::Event& event, sf::RenderWindow& window);
    void update(float dt);
    void draw(sf::RenderWindow& window);

private:
    Map m_map;

    std::vector<Enemy>      m_enemies;
    std::vector<Troop>      m_troops;
    std::vector<Projectile> m_projectiles;
    std::vector<DmgText>    m_dmgTexts;   // damage number popups

    // --- Textures ---
    sf::Texture m_texCatIdle, m_texCatAttack, m_texCatHurt;
    sf::Texture m_texSlimeIdle, m_texSlimeWalk, m_texSlimeHurt;
    sf::Texture m_texFireball;
    bool        m_assetsLoaded = false;

    // --- Font ---
    sf::Font    m_font;
    bool        m_fontLoaded = false;

    // --- Game state ---
    int         m_money    = MONEY_START;
    int         m_baseHP   = BASE_MAX_HP;
    float       m_moneyTimer = 0.f;
    GamePhase   m_phase    = GamePhase::Playing;

    // --- Wave ---
    float       m_spawnTimer = 0.f;
    int         m_spawned    = 0;
    int         m_killed     = 0;

    // --- Deploy ---
    int         m_selectedCard = -1;
    bool        m_placing      = false;

    // --- Methods ---
    void loadAssets();
    void spawnEnemy();
    void updateCollisions();
    void tryDeployTroop(sf::Vector2f worldPos);

    void drawHUD(sf::RenderWindow& window);
    void drawUIBar(sf::RenderWindow& window);
    void drawDeployCursor(sf::RenderWindow& window, sf::Vector2f mousePos);
    void drawGameOver(sf::RenderWindow& window);
    void drawDmgTexts(sf::RenderWindow& window);

    struct TroopCard {
        const char* name;
        int         cost;
    };
    static constexpr TroopCard CARDS[] = {
        { "CatBlaze", TROOP_COST },
        { "", 0 }, { "", 0 }, { "", 0 }
    };
    static constexpr int NUM_CARDS = 4;
};