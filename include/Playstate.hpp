#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include "Map.hpp"
#include "Enemy.hpp"
#include "Troops.hpp"
#include "Projectile.hpp"
#include "Constants.hpp"

enum class GamePhase { Playing, GameOver, Victory, Paused };

struct DmgText {
    sf::Vector2f pos;
    float value, life=1.0f, alpha=255.f;
};

class PlayState {
public:
GamePhase getPhase() const { return m_phase; }
    PlayState();
    void handleEvent(const sf::Event& event, sf::RenderWindow& window);
    void update(float dt);
    void draw(sf::RenderWindow& window);
    void reset();

private:
    Map m_map;
    std::vector<Enemy>      m_enemies;
    std::vector<Troop>      m_troops;
    std::vector<Projectile> m_projectiles;
    std::vector<DmgText>    m_dmgTexts;

    sf::Texture m_texCatIdle, m_texCatAttack, m_texCatHurt;
    sf::Texture m_texSlimeIdle, m_texSlimeWalk, m_texSlimeHurt;
    sf::Texture m_texFireball;
    sf::Texture m_texMedIcon;
    bool        m_assetsLoaded = false;

    sf::Font    m_font;
    bool        m_fontLoaded = false;

    int         m_money    = MONEY_START;
    int         m_baseHP   = BASE_MAX_HP;
    float       m_moneyTimer = 0.f;
    GamePhase   m_phase    = GamePhase::Playing;

    float       m_spawnTimer = 0.f;
    int         m_spawned    = 0;
    
    // *** BARU: Transition timer untuk 30 detik delay antara top dan bottom spawn ***
    float       m_transitionTimer = 0.f;

    // Deploy state
    int         m_selectedCard = -1;
    bool        m_placing      = false;
    // *** BARU: arah deploy dikontrol keyboard arrow ***
    FacingDir   m_deployDir    = FacingDir::Left;  // default kiri

    void loadAssets();
    void spawnEnemy(bool fromTopPath);  // *** DIUBAH: bool parameter untuk path selection ***
    void updateCollisions();
    void tryDeployTroop(sf::Vector2f worldPos);

    void drawHUD(sf::RenderWindow& window);
    void drawUIBar(sf::RenderWindow& window);
    void drawDeployCursor(sf::RenderWindow& window, sf::Vector2f mousePos);
    void drawPauseScreen(sf::RenderWindow& window);
    void drawDmgTexts(sf::RenderWindow& window);

    struct TroopCard {
        const char* name;
        int cost;
    };
    static constexpr TroopCard CARDS[] = {
        { "CatBlaze", TROOP_COST },
        { "", 0 }, { "", 0 }, { "", 0 }
    };
    static constexpr int NUM_CARDS = 4;
};