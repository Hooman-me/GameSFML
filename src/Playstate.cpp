#include "Playstate.hpp"
#include "Constants.hpp"
#include <algorithm>
#include <sstream>
#include <cmath>

PlayState::PlayState() { loadAssets(); }

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
    m_fontLoaded   = m_font.loadFromFile(FONT_REAL_PATH);
}

// ---------------------------------------------------------------
void PlayState::handleEvent(const sf::Event& event, sf::RenderWindow& window) {
    if (m_phase != GamePhase::Playing) {
        if (event.type==sf::Event::KeyPressed && event.key.code==sf::Keyboard::R)
            *this = PlayState();
        return;
    }

    // *** Arrow keys mengubah arah deploy ***
    if (event.type == sf::Event::KeyPressed) {
        switch (event.key.code) {
            case sf::Keyboard::Left:  m_deployDir = FacingDir::Left;  break;
            case sf::Keyboard::Right: m_deployDir = FacingDir::Right; break;
            case sf::Keyboard::Up:    m_deployDir = FacingDir::Up;    break;
            case sf::Keyboard::Down:  m_deployDir = FacingDir::Down;  break;
            case sf::Keyboard::Escape:
                m_selectedCard=-1; m_placing=false;
                break;
            default: break;
        }
    }

    if (event.type==sf::Event::MouseButtonPressed &&
        event.mouseButton.button==sf::Mouse::Left)
    {
        sf::Vector2f mp = window.mapPixelToCoords(
            {event.mouseButton.x, event.mouseButton.y});

        float barTop = (float)GAME_AREA_HEIGHT;
        if (mp.y >= barTop) {
            // Klik UI bar – pilih kartu
            float cardW=115.f, cardH=90.f, startX=10.f, startY=barTop+11.f;
            for (int i=0; i<NUM_CARDS; ++i) {
                if (!CARDS[i].name || !CARDS[i].name[0]) continue;
                sf::FloatRect r(startX+i*(cardW+6.f), startY, cardW, cardH);
                if (r.contains(mp)) {
                    m_selectedCard = (m_selectedCard==i) ? -1 : i;
                    m_placing = (m_selectedCard>=0);
                    return;
                }
            }
            return;
        }
        if (m_placing && m_selectedCard>=0) tryDeployTroop(mp);
    }

    if (event.type==sf::Event::MouseButtonPressed &&
        event.mouseButton.button==sf::Mouse::Right) {
        m_selectedCard=-1; m_placing=false;
    }
}

// ---------------------------------------------------------------
void PlayState::update(float dt) {
    if (m_phase!=GamePhase::Playing) return;
    m_moneyTimer+=dt;
    if (m_moneyTimer>=MONEY_TICK) { m_moneyTimer-=MONEY_TICK; m_money++; }

    m_spawnTimer+=dt;
    if (m_spawned<WAVE_ENEMY_COUNT && m_spawnTimer>=ENEMY_SPAWN_INTERVAL) {
        m_spawnTimer-=ENEMY_SPAWN_INTERVAL; spawnEnemy();
    }

    for (auto& e : m_enemies) {
        e.update(dt);
        if (e.hasReached()) {
            m_baseHP--;
            if (m_baseHP<=0) { m_baseHP=0; m_phase=GamePhase::GameOver; }
        }
    }
    m_enemies.erase(
        std::remove_if(m_enemies.begin(),m_enemies.end(),
            [](const Enemy& e){return e.isDead()||e.hasReached();}),
        m_enemies.end());

    std::vector<Enemy*> ePtrs;
    for (auto& e : m_enemies) ePtrs.push_back(&e);
    for (auto& t : m_troops) t.update(dt, ePtrs, m_projectiles);
    for (auto& p : m_projectiles) p.update(dt);
    updateCollisions();
    m_projectiles.erase(
        std::remove_if(m_projectiles.begin(),m_projectiles.end(),
            [](const Projectile& p){return p.getState()==ProjState::Done;}),
        m_projectiles.end());

    for (auto& d : m_dmgTexts) {
        d.life-=dt; d.pos.y-=28.f*dt;
        d.alpha=std::max(0.f,d.alpha-255.f*dt);
    }
    m_dmgTexts.erase(
        std::remove_if(m_dmgTexts.begin(),m_dmgTexts.end(),
            [](const DmgText& d){return d.life<=0.f;}),
        m_dmgTexts.end());

    if (m_spawned>=WAVE_ENEMY_COUNT && m_enemies.empty() && m_phase==GamePhase::Playing)
        m_phase=GamePhase::Victory;
}

void PlayState::spawnEnemy() {
    m_enemies.emplace_back(
        &m_map.getWaypoints(),
        m_assetsLoaded ? &m_texSlimeIdle : nullptr,
        m_assetsLoaded ? &m_texSlimeWalk : nullptr,
        m_assetsLoaded ? &m_texSlimeHurt : nullptr);
    m_spawned++;
}

void PlayState::updateCollisions() {
    for (auto& proj : m_projectiles) {
        if (proj.getState()!=ProjState::Flying) continue;
        sf::FloatRect pr=proj.getBounds();
        for (auto& enemy : m_enemies) {
            if (enemy.isDead()) continue;
            if (pr.intersects(enemy.getBounds())) {
                float dmg = enemy.takeDamage(proj.getDamage());
                proj.triggerExplosion();
                if (m_fontLoaded && dmg>0.f) {
                    m_dmgTexts.push_back({
                        enemy.getPosition()+sf::Vector2f(0.f,-30.f),
                        dmg, 0.9f, 255.f});
                }
                if (enemy.isDead()) m_money+=MONEY_PER_KILL;
                break;
            }
        }
        sf::FloatRect screen(0.f,0.f,(float)WINDOW_WIDTH,(float)GAME_AREA_HEIGHT);
        if (!screen.intersects(pr)) proj.triggerExplosion();
    }
}

void PlayState::tryDeployTroop(sf::Vector2f worldPos) {
    if (m_selectedCard<0||m_selectedCard>=NUM_CARDS) return;
    if (!CARDS[m_selectedCard].name || !CARDS[m_selectedCard].name[0]) return;
    if ((int)m_troops.size()>=MAX_TROOPS) return;
    if (m_money<CARDS[m_selectedCard].cost) return;
    if (!m_map.isDeployable(worldPos)) return;  // Cek layer 3, 4, jalur, base

    m_troops.emplace_back(worldPos, m_deployDir,
        m_assetsLoaded ? &m_texCatIdle   : nullptr,
        m_assetsLoaded ? &m_texCatAttack : nullptr,
        m_assetsLoaded ? &m_texCatHurt   : nullptr,
        m_assetsLoaded ? &m_texFireball  : nullptr);
    m_money -= CARDS[m_selectedCard].cost;
    if ((int)m_troops.size()>=MAX_TROOPS) { m_selectedCard=-1; m_placing=false; }
}

// ---------------------------------------------------------------
void PlayState::draw(sf::RenderWindow& window) {
    window.clear(sf::Color(34,48,34));
    m_map.draw(window);
    for (auto& t : m_troops)  t.drawRangeIndicator(window);
    for (auto& e : m_enemies) e.draw(window);
    for (auto& t : m_troops)  t.draw(window);
    for (auto& p : m_projectiles) p.draw(window);
    drawDmgTexts(window);
    if (m_placing) {
        sf::Vector2i mi=sf::Mouse::getPosition(window);
        drawDeployCursor(window, window.mapPixelToCoords(mi));
    }
    drawHUD(window);
    drawUIBar(window);
    if (m_phase!=GamePhase::Playing) drawGameOver(window);
}

void PlayState::drawHUD(sf::RenderWindow& window) {
    sf::RectangleShape panel({300.f,34.f});
    panel.setFillColor(sf::Color(50,50,50,210));
    panel.setOutlineColor(sf::Color(130,130,130));
    panel.setOutlineThickness(1.5f);
    panel.setPosition((float)WINDOW_WIDTH*.5f-150.f, 5.f);
    window.draw(panel);
    if (m_fontLoaded) {
        std::ostringstream ss;
        ss<<"ENEMY: "<<m_spawned<<"/"<<WAVE_ENEMY_COUNT<<"    HP: "<<m_baseHP;
        sf::Text t; t.setFont(m_font); t.setString(ss.str());
        t.setCharacterSize(17); t.setFillColor(sf::Color::White);
        t.setPosition((float)WINDOW_WIDTH*.5f-135.f, 10.f);
        window.draw(t);
    } else {
        float pct=(float)m_baseHP/BASE_MAX_HP;
        sf::RectangleShape b({280.f*pct,14.f});
        b.setFillColor(sf::Color(50,210,80));
        b.setPosition((float)WINDOW_WIDTH*.5f-140.f,15.f);
        window.draw(b);
    }
}

void PlayState::drawUIBar(sf::RenderWindow& window) {
    float barTop=(float)GAME_AREA_HEIGHT;
    sf::RectangleShape bg({(float)WINDOW_WIDTH,(float)UI_BAR_HEIGHT});
    bg.setFillColor(sf::Color(75,75,75,235)); bg.setPosition(0.f,barTop);
    window.draw(bg);
    sf::RectangleShape sep({(float)WINDOW_WIDTH,2.f});
    sep.setFillColor(sf::Color(110,110,110)); sep.setPosition(0.f,barTop);
    window.draw(sep);

    float cardW=115.f, cardH=90.f, startX=10.f, startY=barTop+11.f;
    for (int i=0; i<NUM_CARDS; ++i) {
        float cx=startX+i*(cardW+6.f);
        bool empty=(!CARDS[i].name||!CARDS[i].name[0]);
        bool sel=(m_selectedCard==i);
        bool canBuy=(!empty && m_money>=CARDS[i].cost &&
                     (int)m_troops.size()<MAX_TROOPS);
        sf::Color bgC = empty     ? sf::Color(55,55,55) :
                        !canBuy   ? sf::Color(50,50,50) :
                        sel       ? sf::Color(210,190,50) :
                                    sf::Color(70,130,70);
        sf::RectangleShape crd({cardW,cardH});
        crd.setFillColor(bgC);
        crd.setOutlineColor(sel ? sf::Color::Yellow : sf::Color(140,140,140));
        crd.setOutlineThickness(sel ? 3.f : 1.5f);
        crd.setPosition(cx,startY);
        window.draw(crd);

        if (!empty) {
            // *** Gambar CatBlaze menghadap KIRI di kartu ***
            if (m_assetsLoaded) {
                sf::Sprite icon(m_texCatIdle);
                icon.setTextureRect(sf::IntRect(0,0,CAT_FRAME_W,CAT_FRAME_H));
                float sc=std::min((cardW-10.f)/CAT_FRAME_W,(cardH-24.f)/CAT_FRAME_H);
                icon.setScale(-sc, sc);  // negative X = flip kiri
                icon.setOrigin(0.f, CAT_FRAME_H*.5f);
                // Offset karena origin digeser saat flip
                icon.setPosition(cx+cardW*.5f+CAT_FRAME_W*sc*.5f, startY+cardH*.5f-10.f);
                if (!canBuy) icon.setColor(sf::Color(120,120,120));
                window.draw(icon);
            }
            // Cost badge
            sf::RectangleShape badge({44.f,18.f});
            badge.setFillColor(sf::Color(25,25,25,210));
            badge.setPosition(cx+cardW*.5f-22.f, startY+cardH-20.f);
            window.draw(badge);
            if (m_fontLoaded) {
                sf::Text ct; ct.setFont(m_font);
                ct.setString(std::to_string(CARDS[i].cost));
                ct.setCharacterSize(13);
                ct.setFillColor(canBuy?sf::Color(255,220,50):sf::Color(130,130,130));
                ct.setPosition(cx+cardW*.5f-10.f, startY+cardH-21.f);
                window.draw(ct);
                sf::Text nt; nt.setFont(m_font);
                nt.setString(CARDS[i].name);
                nt.setCharacterSize(11); nt.setFillColor(sf::Color::White);
                nt.setPosition(cx+5.f, startY+3.f);
                window.draw(nt);
            }
        }
    }

    // Direction indicator saat placing
    if (m_placing && m_fontLoaded) {
        std::string dirStr = "Arah: ";
        switch (m_deployDir) {
            case FacingDir::Left:  dirStr+="← Kiri";  break;
            case FacingDir::Right: dirStr+="→ Kanan"; break;
            case FacingDir::Up:    dirStr+="↑ Atas";  break;
            case FacingDir::Down:  dirStr+="↓ Bawah"; break;
        }
        dirStr += "  (Arrow Keys)";
        sf::Text dt; dt.setFont(m_font);
        dt.setString(dirStr); dt.setCharacterSize(12);
        dt.setFillColor(sf::Color(255,255,100));
        dt.setPosition(startX, barTop+95.f);
        window.draw(dt);
    }

    // MONEY panel
    sf::RectangleShape mp2({118.f,90.f});
    mp2.setFillColor(sf::Color(55,55,55));
    mp2.setOutlineColor(sf::Color(140,140,140));
    mp2.setOutlineThickness(1.5f);
    mp2.setPosition((float)WINDOW_WIDTH-128.f, barTop+11.f);
    window.draw(mp2);
    if (m_fontLoaded) {
        sf::Text ml; ml.setFont(m_font); ml.setString("MONEY");
        ml.setCharacterSize(15); ml.setFillColor(sf::Color(190,190,190));
        ml.setPosition((float)WINDOW_WIDTH-112.f, barTop+20.f);
        window.draw(ml);
        sf::Text mv; mv.setFont(m_font);
        mv.setString(std::to_string(m_money));
        mv.setCharacterSize(28); mv.setFillColor(sf::Color(255,220,50));
        mv.setPosition((float)WINDOW_WIDTH-100.f, barTop+44.f);
        window.draw(mv);
    }
    // Troop count
    if (m_fontLoaded) {
        sf::Text tc; tc.setFont(m_font);
        tc.setString("Troops: "+std::to_string(m_troops.size())+"/"+
                     std::to_string(MAX_TROOPS));
        tc.setCharacterSize(12); tc.setFillColor(sf::Color(180,255,180));
        tc.setPosition(10.f, barTop+95.f);
        window.draw(tc);
    }
}

void PlayState::drawDeployCursor(sf::RenderWindow& window, sf::Vector2f mp) {
    bool valid = m_map.isDeployable(mp) &&
                 (int)m_troops.size()<MAX_TROOPS &&
                 m_selectedCard>=0 &&
                 m_money>=CARDS[m_selectedCard].cost;
    sf::Color col = valid ? sf::Color(100,255,100,160) : sf::Color(255,80,80,160);

    if (m_assetsLoaded) {
        sf::Sprite prev(m_texCatIdle);
        prev.setTextureRect(sf::IntRect(0,0,CAT_FRAME_W,CAT_FRAME_H));
        prev.setOrigin(CAT_FRAME_W*.5f, CAT_FRAME_H*.5f);
        // Mirror sesuai arah deploy yang dipilih
        float sx = (m_deployDir==FacingDir::Left||m_deployDir==FacingDir::Up) ? -1.f : 1.f;
        prev.setScale(sx, 1.f);
        prev.setColor(col);
        prev.setPosition(mp);
        window.draw(prev);
    } else {
        sf::CircleShape c(20.f); c.setFillColor(col);
        c.setOrigin(20.f,20.f); c.setPosition(mp);
        window.draw(c);
    }
    // Tunjukkan panah arah
    if (!valid) {
        sf::RectangleShape redX({4.f,28.f});
        redX.setFillColor(sf::Color(255,0,0,200));
        redX.setOrigin(2.f,14.f); redX.setPosition(mp); redX.setRotation(45.f);
        window.draw(redX);
        redX.setRotation(-45.f); window.draw(redX);
    }
}

void PlayState::drawDmgTexts(sf::RenderWindow& window) {
    if (!m_fontLoaded) return;
    for (auto& d : m_dmgTexts) {
        sf::Text t; t.setFont(m_font);
        std::ostringstream ss; ss<<(int)d.value;
        t.setString(ss.str()); t.setCharacterSize(16);
        sf::Uint8 a=(sf::Uint8)d.alpha;
        t.setFillColor(sf::Color(255,230,50,a));
        t.setOutlineColor(sf::Color(0,0,0,a));
        t.setOutlineThickness(1.5f);
        t.setPosition(d.pos);
        window.draw(t);
    }
}

void PlayState::drawGameOver(sf::RenderWindow& window) {
    sf::RectangleShape ov({(float)WINDOW_WIDTH,(float)GAME_AREA_HEIGHT});
    ov.setFillColor(sf::Color(0,0,0,155)); window.draw(ov);
    if (!m_fontLoaded) return;
    bool won=(m_phase==GamePhase::Victory);
    sf::Text t1; t1.setFont(m_font);
    t1.setString(won?"VICTORY!":"GAME OVER");
    t1.setCharacterSize(60);
    t1.setFillColor(won?sf::Color(100,255,100):sf::Color(255,80,80));
    auto b1=t1.getLocalBounds();
    t1.setOrigin(b1.width*.5f,b1.height*.5f);
    t1.setPosition((float)WINDOW_WIDTH*.5f,(float)GAME_AREA_HEIGHT*.38f);
    window.draw(t1);
    sf::Text t2; t2.setFont(m_font);
    t2.setString("Press R to restart");
    t2.setCharacterSize(22); t2.setFillColor(sf::Color::White);
    auto b2=t2.getLocalBounds();
    t2.setOrigin(b2.width*.5f,b2.height*.5f);
    t2.setPosition((float)WINDOW_WIDTH*.5f,(float)GAME_AREA_HEIGHT*.58f);
    window.draw(t2);
}