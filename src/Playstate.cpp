#include "Playstate.hpp"
#include "Constants.hpp"
#include <algorithm>
#include <sstream>
#include <cmath>

PlayState::PlayState() {
    loadAssets();
    m_money  = MONEY_START;
    m_baseHP = BASE_MAX_HP;
}

void PlayState::loadAssets() {
    bool ok = true;
    ok &= m_texCatIdle.loadFromFile(CAT_IDLE_PATH);
    ok &= m_texCatAttack.loadFromFile(CAT_ATTACK_PATH);
    ok &= m_texCatHurt.loadFromFile(CAT_HURT_PATH);
    ok &= m_texSlimeIdle.loadFromFile(SLIME_IDLE_PATH);
    ok &= m_texSlimeWalk.loadFromFile(SLIME_WALK_PATH);
    ok &= m_texSlimeHurt.loadFromFile(SLIME_HURT_PATH);
    ok &= m_texFireball.loadFromFile(FIREBALL_PATH);
    m_assetsLoaded = ok;

    m_fontLoaded = m_font.loadFromFile(FONT_REAL_PATH);
}

// ---------------------------------------------------------------
// Events
// ---------------------------------------------------------------
void PlayState::handleEvent(const sf::Event& event, sf::RenderWindow& window) {
    if (m_phase != GamePhase::Playing) {
        if (event.type == sf::Event::KeyPressed &&
            event.key.code == sf::Keyboard::R)
            *this = PlayState();
        return;
    }

    if (event.type == sf::Event::MouseButtonPressed &&
        event.mouseButton.button == sf::Mouse::Left)
    {
        sf::Vector2f mp = window.mapPixelToCoords(
            { event.mouseButton.x, event.mouseButton.y });

        float barTop = (float)GAME_AREA_HEIGHT;

        if (mp.y >= barTop) {
            // Klik di UI bar – cek kartu
            float cardW = 115.f, cardH = 90.f;
            float startX = 10.f, startY = barTop + 11.f;

            for (int i = 0; i < NUM_CARDS; ++i) {
                if (!CARDS[i].name || CARDS[i].name[0] == '\0') continue;
                sf::FloatRect r(startX + i * (cardW + 6.f), startY, cardW, cardH);
                if (r.contains(mp)) {
                    m_selectedCard = (m_selectedCard == i) ? -1 : i;
                    m_placing = (m_selectedCard >= 0);
                    return;
                }
            }
            return;
        }

        // Klik di area game
        if (m_placing && m_selectedCard >= 0)
            tryDeployTroop(mp);
    }

    if ((event.type == sf::Event::MouseButtonPressed &&
         event.mouseButton.button == sf::Mouse::Right) ||
        (event.type == sf::Event::KeyPressed &&
         event.key.code == sf::Keyboard::Escape))
    {
        m_selectedCard = -1;
        m_placing      = false;
    }
}

// ---------------------------------------------------------------
// Update
// ---------------------------------------------------------------
void PlayState::update(float dt) {
    if (m_phase != GamePhase::Playing) return;

    // Auto money
    m_moneyTimer += dt;
    if (m_moneyTimer >= MONEY_TICK) {
        m_moneyTimer -= MONEY_TICK;
        m_money++;
    }

    // Spawn enemies
    m_spawnTimer += dt;
    if (m_spawned < WAVE_ENEMY_COUNT && m_spawnTimer >= ENEMY_SPAWN_INTERVAL) {
        m_spawnTimer -= ENEMY_SPAWN_INTERVAL;
        spawnEnemy();
    }

    // Update enemies
    for (auto& e : m_enemies) {
        e.update(dt);
        if (e.hasReached()) {
            m_baseHP--;
            if (m_baseHP <= 0) {
                m_baseHP = 0;
                m_phase  = GamePhase::GameOver;
            }
        }
    }
    m_enemies.erase(
        std::remove_if(m_enemies.begin(), m_enemies.end(),
            [](const Enemy& e){ return e.isDead() || e.hasReached(); }),
        m_enemies.end());

    // Update troops
    std::vector<Enemy*> ePtrs;
    ePtrs.reserve(m_enemies.size());
    for (auto& e : m_enemies) ePtrs.push_back(&e);
    for (auto& t : m_troops)
        t.update(dt, ePtrs, m_projectiles);

    // Update projectiles
    for (auto& p : m_projectiles) p.update(dt);

    // Collisions
    updateCollisions();

    m_projectiles.erase(
        std::remove_if(m_projectiles.begin(), m_projectiles.end(),
            [](const Projectile& p){ return p.getState() == ProjState::Done; }),
        m_projectiles.end());

    // Update damage texts
    for (auto& dt_ : m_dmgTexts) {
        dt_.life  -= dt;
        dt_.pos.y -= 30.f * dt;
        dt_.alpha  = std::max(0.f, dt_.alpha - 255.f * dt);
    }
    m_dmgTexts.erase(
        std::remove_if(m_dmgTexts.begin(), m_dmgTexts.end(),
            [](const DmgText& d){ return d.life <= 0.f; }),
        m_dmgTexts.end());

    // Victory check
    if (m_spawned >= WAVE_ENEMY_COUNT && m_enemies.empty() &&
        m_phase == GamePhase::Playing)
        m_phase = GamePhase::Victory;
}

void PlayState::spawnEnemy() {
    m_enemies.emplace_back(
        m_map.getWaypoints(),
        m_assetsLoaded ? &m_texSlimeIdle : nullptr,
        m_assetsLoaded ? &m_texSlimeWalk : nullptr,
        m_assetsLoaded ? &m_texSlimeHurt : nullptr);
    m_spawned++;
}

void PlayState::updateCollisions() {
    for (auto& proj : m_projectiles) {
        if (proj.getState() != ProjState::Flying) continue;
        sf::FloatRect pRect = proj.getBounds();

        for (auto& enemy : m_enemies) {
            if (enemy.isDead()) continue;
            if (pRect.intersects(enemy.getBounds())) {
                // takeDamage mengembalikan damage aktual setelah defense
                float dmg = enemy.takeDamage(proj.getDamage());
                proj.triggerExplosion();

                // Spawn damage text popup
                if (m_fontLoaded && dmg > 0.f) {
                    DmgText dt_;
                    dt_.pos   = enemy.getPosition() + sf::Vector2f(0.f, -30.f);
                    dt_.value = dmg;
                    dt_.life  = 0.9f;
                    dt_.alpha = 255.f;
                    m_dmgTexts.push_back(dt_);
                }

                if (enemy.isDead()) m_money += MONEY_PER_KILL;
                break;
            }
        }

        // Remove out-of-screen projectiles
        sf::FloatRect screen(0.f, 0.f, (float)WINDOW_WIDTH, (float)GAME_AREA_HEIGHT);
        if (!screen.intersects(pRect))
            proj.triggerExplosion();
    }
}

void PlayState::tryDeployTroop(sf::Vector2f worldPos) {
    if (m_selectedCard < 0 || m_selectedCard >= NUM_CARDS) return;
    if (!CARDS[m_selectedCard].name || CARDS[m_selectedCard].name[0] == '\0') return;
    if ((int)m_troops.size() >= MAX_TROOPS)  return;
    if (m_money < CARDS[m_selectedCard].cost) return;
    if (!m_map.isDeployable(worldPos)) return;

    // Arah hadap: kiri setengah layar = hadap kanan, kanan = hadap kiri
    FacingDir dir = (worldPos.x < WINDOW_WIDTH * .5f)
                    ? FacingDir::Right : FacingDir::Left;

    m_troops.emplace_back(worldPos, dir,
        m_assetsLoaded ? &m_texCatIdle   : nullptr,
        m_assetsLoaded ? &m_texCatAttack : nullptr,
        m_assetsLoaded ? &m_texCatHurt   : nullptr,
        m_assetsLoaded ? &m_texFireball  : nullptr);

    m_money -= CARDS[m_selectedCard].cost;

    if ((int)m_troops.size() >= MAX_TROOPS) {
        m_selectedCard = -1;
        m_placing      = false;
    }
}

// ---------------------------------------------------------------
// Draw
// ---------------------------------------------------------------
void PlayState::draw(sf::RenderWindow& window) {
    window.clear(sf::Color(34, 48, 34));
    m_map.draw(window);

    // Range indicators
    for (auto& t : m_troops) t.drawRangeIndicator(window);

    for (auto& e : m_enemies)     e.draw(window);
    for (auto& t : m_troops)      t.draw(window);
    for (auto& p : m_projectiles) p.draw(window);

    drawDmgTexts(window);

    if (m_placing) {
        sf::Vector2i mi = sf::Mouse::getPosition(window);
        drawDeployCursor(window, window.mapPixelToCoords(mi));
    }

    drawHUD(window);
    drawUIBar(window);

    if (m_phase != GamePhase::Playing)
        drawGameOver(window);
}

// ---------------------------------------------------------------
// HUD
// ---------------------------------------------------------------
void PlayState::drawHUD(sf::RenderWindow& window) {
    // Panel atas tengah
    sf::RectangleShape panel({ 300.f, 34.f });
    panel.setFillColor(sf::Color(50, 50, 50, 210));
    panel.setOutlineColor(sf::Color(130,130,130));
    panel.setOutlineThickness(1.5f);
    panel.setPosition((float)WINDOW_WIDTH*.5f - 150.f, 5.f);
    window.draw(panel);

    if (m_fontLoaded) {
        std::ostringstream ss;
        ss << "ENEMY: " << m_spawned << "/" << WAVE_ENEMY_COUNT
           << "    HP: " << m_baseHP;
        sf::Text txt;
        txt.setFont(m_font);
        txt.setString(ss.str());
        txt.setCharacterSize(17);
        txt.setFillColor(sf::Color::White);
        txt.setPosition((float)WINDOW_WIDTH*.5f - 135.f, 10.f);
        window.draw(txt);
    } else {
        // HP bar visual fallback
        float pct = (float)m_baseHP / BASE_MAX_HP;
        sf::RectangleShape hb({ 280.f * pct, 14.f });
        hb.setFillColor(sf::Color(50, 210, 80));
        hb.setPosition((float)WINDOW_WIDTH*.5f - 140.f, 15.f);
        window.draw(hb);
    }
}

// ---------------------------------------------------------------
// UI Bar bawah
// ---------------------------------------------------------------
void PlayState::drawUIBar(sf::RenderWindow& window) {
    float barTop = (float)GAME_AREA_HEIGHT;

    // Background
    sf::RectangleShape bg({ (float)WINDOW_WIDTH, (float)UI_BAR_HEIGHT });
    bg.setFillColor(sf::Color(75, 75, 75, 235));
    bg.setPosition(0.f, barTop);
    window.draw(bg);

    sf::RectangleShape sep({ (float)WINDOW_WIDTH, 2.f });
    sep.setFillColor(sf::Color(110, 110, 110));
    sep.setPosition(0.f, barTop);
    window.draw(sep);

    // Kartu troop
    float cardW = 115.f, cardH = 90.f;
    float startX = 10.f, startY = barTop + 11.f;

    for (int i = 0; i < NUM_CARDS; ++i) {
        float cx = startX + i * (cardW + 6.f);
        bool empty    = (!CARDS[i].name || CARDS[i].name[0] == '\0');
        bool selected = (m_selectedCard == i);
        bool canBuy   = (!empty && m_money >= CARDS[i].cost &&
                        (int)m_troops.size() < MAX_TROOPS);

        sf::RectangleShape crd({ cardW, cardH });
        sf::Color bg2 = empty ? sf::Color(55,55,55) :
                        !canBuy ? sf::Color(50,50,50) :
                        selected ? sf::Color(210,190,50) :
                        sf::Color(70,130,70);
        crd.setFillColor(bg2);
        crd.setOutlineColor(selected ? sf::Color::Yellow : sf::Color(140,140,140));
        crd.setOutlineThickness(selected ? 3.f : 1.5f);
        crd.setPosition(cx, startY);
        window.draw(crd);

        if (!empty) {
            // CatBlaze idle frame as card icon
            if (m_assetsLoaded) {
                sf::Sprite icon(m_texCatIdle);
                icon.setTextureRect(sf::IntRect(0, 0, CAT_FRAME_W, CAT_FRAME_H));
                icon.setOrigin(CAT_FRAME_W*.5f, CAT_FRAME_H*.5f);
                // Scale to fit card nicely
                float sc = std::min((cardW-10.f)/CAT_FRAME_W, (cardH-24.f)/CAT_FRAME_H);
                icon.setScale(sc, sc);
                if (!canBuy) icon.setColor(sf::Color(120,120,120));
                icon.setPosition(cx + cardW*.5f, startY + cardH*.5f - 8.f);
                window.draw(icon);
            }

            // Cost badge
            sf::RectangleShape badge({ 44.f, 18.f });
            badge.setFillColor(sf::Color(25,25,25,210));
            badge.setPosition(cx + cardW*.5f - 22.f, startY + cardH - 20.f);
            window.draw(badge);

            if (m_fontLoaded) {
                sf::Text costTxt;
                costTxt.setFont(m_font);
                costTxt.setString(std::to_string(CARDS[i].cost));
                costTxt.setCharacterSize(13);
                costTxt.setFillColor(canBuy ? sf::Color(255,220,50) : sf::Color(130,130,130));
                costTxt.setPosition(cx + cardW*.5f - 10.f, startY + cardH - 21.f);
                window.draw(costTxt);

                sf::Text nameTxt;
                nameTxt.setFont(m_font);
                nameTxt.setString(CARDS[i].name);
                nameTxt.setCharacterSize(11);
                nameTxt.setFillColor(sf::Color::White);
                nameTxt.setPosition(cx + 5.f, startY + 3.f);
                window.draw(nameTxt);
            }
        }
    }

    // MONEY panel (kanan)
    sf::RectangleShape moneyPanel({ 118.f, 90.f });
    moneyPanel.setFillColor(sf::Color(55, 55, 55));
    moneyPanel.setOutlineColor(sf::Color(140,140,140));
    moneyPanel.setOutlineThickness(1.5f);
    moneyPanel.setPosition((float)WINDOW_WIDTH - 128.f, barTop + 11.f);
    window.draw(moneyPanel);

    if (m_fontLoaded) {
        sf::Text mLabel;
        mLabel.setFont(m_font);
        mLabel.setString("MONEY");
        mLabel.setCharacterSize(15);
        mLabel.setFillColor(sf::Color(190,190,190));
        mLabel.setPosition((float)WINDOW_WIDTH - 112.f, barTop + 20.f);
        window.draw(mLabel);

        sf::Text mVal;
        mVal.setFont(m_font);
        mVal.setString(std::to_string(m_money));
        mVal.setCharacterSize(28);
        mVal.setFillColor(sf::Color(255,220,50));
        mVal.setPosition((float)WINDOW_WIDTH - 100.f, barTop + 44.f);
        window.draw(mVal);
    }

    // Troop count (kiri bawah)
    if (m_fontLoaded) {
        sf::Text tc;
        tc.setFont(m_font);
        tc.setString("Troops: " + std::to_string(m_troops.size()) + "/" +
                     std::to_string(MAX_TROOPS));
        tc.setCharacterSize(12);
        tc.setFillColor(sf::Color(180,255,180));
        tc.setPosition(10.f, barTop + 94.f);
        window.draw(tc);
    }
}

// ---------------------------------------------------------------
// Deploy cursor
// ---------------------------------------------------------------
void PlayState::drawDeployCursor(sf::RenderWindow& window, sf::Vector2f mp) {
    bool valid = m_map.isDeployable(mp) &&
                 (int)m_troops.size() < MAX_TROOPS &&
                 m_selectedCard >= 0 &&
                 m_money >= CARDS[m_selectedCard].cost;

    sf::Color col = valid ? sf::Color(100,255,100,160) : sf::Color(255,80,80,160);

    if (m_assetsLoaded) {
        sf::Sprite preview(m_texCatIdle);
        preview.setTextureRect(sf::IntRect(0,0,CAT_FRAME_W,CAT_FRAME_H));
        preview.setOrigin(CAT_FRAME_W*.5f, CAT_FRAME_H*.5f);
        preview.setColor(col);
        preview.setPosition(mp);
        window.draw(preview);
    } else {
        sf::CircleShape c(20.f);
        c.setFillColor(col);
        c.setOrigin(20.f,20.f);
        c.setPosition(mp);
        window.draw(c);
    }
}

// ---------------------------------------------------------------
// Damage texts
// ---------------------------------------------------------------
void PlayState::drawDmgTexts(sf::RenderWindow& window) {
    if (!m_fontLoaded) return;
    for (auto& dt_ : m_dmgTexts) {
        sf::Text t;
        t.setFont(m_font);
        std::ostringstream ss;
        ss << (int)dt_.value;
        t.setString(ss.str());
        t.setCharacterSize(16);
        sf::Uint8 a = (sf::Uint8)dt_.alpha;
        t.setFillColor(sf::Color(255, 230, 50, a));
        t.setOutlineColor(sf::Color(0,0,0,a));
        t.setOutlineThickness(1.5f);
        t.setPosition(dt_.pos);
        window.draw(t);
    }
}

// ---------------------------------------------------------------
// Game Over / Victory overlay
// ---------------------------------------------------------------
void PlayState::drawGameOver(sf::RenderWindow& window) {
    sf::RectangleShape overlay({ (float)WINDOW_WIDTH, (float)GAME_AREA_HEIGHT });
    overlay.setFillColor(sf::Color(0,0,0,155));
    window.draw(overlay);
    if (!m_fontLoaded) return;

    bool  won   = (m_phase == GamePhase::Victory);
    sf::Color tc = won ? sf::Color(100,255,100) : sf::Color(255,80,80);

    sf::Text t1;
    t1.setFont(m_font);
    t1.setString(won ? "VICTORY!" : "GAME OVER");
    t1.setCharacterSize(60);
    t1.setFillColor(tc);
    auto b1 = t1.getLocalBounds();
    t1.setOrigin(b1.width*.5f, b1.height*.5f);
    t1.setPosition((float)WINDOW_WIDTH*.5f, (float)GAME_AREA_HEIGHT*.38f);
    window.draw(t1);

    sf::Text t2;
    t2.setFont(m_font);
    t2.setString("Press R to restart");
    t2.setCharacterSize(22);
    t2.setFillColor(sf::Color::White);
    auto b2 = t2.getLocalBounds();
    t2.setOrigin(b2.width*.5f, b2.height*.5f);
    t2.setPosition((float)WINDOW_WIDTH*.5f, (float)GAME_AREA_HEIGHT*.58f);
    window.draw(t2);
}