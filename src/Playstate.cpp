#include "PlayState.hpp"
#include "Constants.hpp"
#include <cmath>
#include <algorithm>
#include <sstream>

// ============================================================
//  PLAYSTATE.CPP
// ============================================================

PlayState::PlayState() {
    // Load texture & font (gagal → pakai placeholder, game tetap jalan)
    m_troopTexLoaded = m_troopTex.loadFromFile(TROOP_SPRITE_PATH);
    m_enemyTexLoaded = m_enemyTex.loadFromFile(ENEMY_SPRITE_PATH);
    m_fontLoaded     = m_font.loadFromFile(FONT_PATH);

    m_money  = MONEY_START;
    m_baseHP = BASE_MAX_HP;

    buildCards();
}

void PlayState::buildCards() {
    // *** Tambah variasi troop di sini nanti ***
    m_cards.push_back({ "Fox\nArcher", TROOP_COST, sf::Color(80, 160, 80) });
    // Placeholder kartu kosong agar visual UI bar terisi 4 slot
    m_cards.push_back({ "", 0, sf::Color(60, 60, 60) });
    m_cards.push_back({ "", 0, sf::Color(60, 60, 60) });
    m_cards.push_back({ "", 0, sf::Color(60, 60, 60) });
}

// ---------------------------------------------------------------
// Event handling
// ---------------------------------------------------------------
void PlayState::handleEvent(const sf::Event& event, sf::RenderWindow& window) {
    if (m_phase != GamePhase::Playing) {
        // Restart jika tekan R
        if (event.type == sf::Event::KeyPressed &&
            event.key.code == sf::Keyboard::R) {
            *this = PlayState();
        }
        return;
    }

    if (event.type == sf::Event::MouseButtonPressed &&
        event.mouseButton.button == sf::Mouse::Left)
    {
        sf::Vector2f mousePos = window.mapPixelToCoords(
            { event.mouseButton.x, event.mouseButton.y });

        // Cek apakah klik di UI bar bawah
        float barTop = (float)GAME_AREA_HEIGHT;
        if (mousePos.y >= barTop) {
            // Klik di area UI bar → cek kartu
            float cardW = 120.f, cardH = 90.f;
            float startX = (float)WINDOW_WIDTH * 0.5f - 240.f;
            float startY = barTop + 10.f;

            for (int i = 0; i < (int)m_cards.size(); ++i) {
                sf::FloatRect cardRect(startX + i * (cardW + 5.f), startY, cardW, cardH);
                if (cardRect.contains(mousePos)) {
                    // Hanya kartu dengan nama valid
                    if (!m_cards[i].name.empty()) {
                        if (m_selectedCard == i) {
                            // Klik kartu yang sama → batal
                            m_selectedCard = -1;
                            m_placing = false;
                        } else {
                            m_selectedCard = i;
                            m_placing = true;
                        }
                    }
                    return;
                }
            }
            return;
        }

        // Klik di area game
        if (m_placing && m_selectedCard >= 0) {
            tryDeployTroop(mousePos);
        }
    }

    // Klik kanan atau Escape → batalkan deploy
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

    // --- Uang otomatis +1/detik ---
    m_moneyTimer += dt;
    if (m_moneyTimer >= MONEY_TICK) {
        m_moneyTimer -= MONEY_TICK;
        m_money++;
    }

    // --- Spawn musuh ---
    m_spawnTimer += dt;
    if (m_spawned < WAVE_ENEMY_COUNT && m_spawnTimer >= ENEMY_SPAWN_INTERVAL) {
        m_spawnTimer -= ENEMY_SPAWN_INTERVAL;
        spawnEnemy();
    }

    // --- Update musuh ---
    sf::Texture* eTex = m_enemyTexLoaded ? &m_enemyTex : nullptr;
    (void)eTex; // sudah di-set saat spawn

    for (auto& e : m_enemies) {
        e.update(dt);
        if (e.hasReached()) {
            // Musuh mencapai base
            m_baseHP--;
            if (m_baseHP <= 0) {
                m_baseHP = 0;
                m_phase  = GamePhase::GameOver;
            }
        }
    }

    // Hapus musuh yang sudah mati atau sudah menembus base
    m_enemies.erase(
        std::remove_if(m_enemies.begin(), m_enemies.end(),
            [](const Enemy& e){ return e.isDead() || e.hasReached(); }),
        m_enemies.end());

    // --- Update troop ---
    // Buat daftar pointer ke musuh hidup
    std::vector<Enemy*> enemyPtrs;
    enemyPtrs.reserve(m_enemies.size());
    for (auto& e : m_enemies) enemyPtrs.push_back(&e);

    for (auto& t : m_troops) {
        t.update(dt, enemyPtrs, m_projectiles);
    }

    // --- Update proyektil ---
    for (auto& p : m_projectiles) {
        p.update(dt);
    }

    // --- Deteksi tabrakan proyektil × musuh ---
    updateCollisions();

    // Hapus proyektil yang selesai
    m_projectiles.erase(
        std::remove_if(m_projectiles.begin(), m_projectiles.end(),
            [](const Projectile& p){ return p.getState() == ProjState::Done; }),
        m_projectiles.end());

    // --- Cek Victory ---
    bool allSpawned  = (m_spawned >= WAVE_ENEMY_COUNT);
    bool allDeadOrGone = m_enemies.empty();
    if (allSpawned && allDeadOrGone && m_phase == GamePhase::Playing) {
        m_phase = GamePhase::Victory;
    }
}

void PlayState::spawnEnemy() {
    sf::Texture* tex = m_enemyTexLoaded ? &m_enemyTex : nullptr;
    m_enemies.emplace_back(m_map.getWaypoints(), tex);
    m_spawned++;
    m_enemyCount = m_spawned;
}

void PlayState::updateCollisions() {
    for (auto& proj : m_projectiles) {
        if (proj.getState() != ProjState::Flying) continue;

        sf::FloatRect pRect = proj.getBounds();
        for (auto& enemy : m_enemies) {
            if (enemy.isDead()) continue;
            if (pRect.intersects(enemy.getBounds())) {
                float dmg = enemy.takeDamage(proj.getDamage());
                proj.triggerExplosion();
                // Jika musuh mati, beri reward uang
                if (enemy.isDead()) {
                    m_money += MONEY_PER_KILL;
                    m_killed++;
                }
                (void)dmg;
                break; // satu peluru hanya kena satu musuh
            }
        }

        // Hapus peluru yang keluar dari layar
        sf::FloatRect screen(0.f, 0.f, (float)WINDOW_WIDTH, (float)GAME_AREA_HEIGHT);
        if (!screen.intersects(pRect)) {
            proj.triggerExplosion(); // langsung explode & hilang
        }
    }
}

void PlayState::tryDeployTroop(sf::Vector2f worldPos) {
    if (m_selectedCard < 0 || m_selectedCard >= (int)m_cards.size()) return;
    auto& card = m_cards[m_selectedCard];
    if (card.name.empty()) return;

    // Cek batas jumlah troop
    if ((int)m_troops.size() >= MAX_TROOPS) return;

    // Cek uang cukup
    if (m_money < card.cost) return;

    // Cek lokasi valid
    if (!m_map.isDeployable(worldPos)) return;

    // Tentukan arah hadap: jika klik di kiri setengah layar → hadap kanan, sebaliknya kiri
    // (logika sederhana; bisa diganti berdasarkan posisi relatif ke jalur)
    FacingDir dir = (worldPos.x < WINDOW_WIDTH * 0.5f)
                    ? FacingDir::Right : FacingDir::Left;

    sf::Texture* tex = m_troopTexLoaded ? &m_troopTex : nullptr;
    m_troops.emplace_back(worldPos, dir, tex);
    m_money -= card.cost;

    // Jika sudah max troop, batalkan mode deploy
    if ((int)m_troops.size() >= MAX_TROOPS) {
        m_selectedCard = -1;
        m_placing      = false;
    }
}

// ---------------------------------------------------------------
// Draw
// ---------------------------------------------------------------
void PlayState::draw(sf::RenderWindow& window) {
    // Background hijau gelap
    window.clear(sf::Color(34, 48, 34));

    // Map (jalur + base)
    m_map.draw(window);

    // Range indicator troop yang sedang di-hover / dipilih
    for (auto& t : m_troops) {
        t.drawRangeIndicator(window);
    }

    // Musuh
    for (auto& e : m_enemies) e.draw(window);

    // Troop
    for (auto& t : m_troops)  t.draw(window);

    // Proyektil
    for (auto& p : m_projectiles) p.draw(window);

    // Deploy cursor preview
    if (m_placing) {
        sf::Vector2i mp = sf::Mouse::getPosition(window);
        sf::Vector2f mouseWorld = window.mapPixelToCoords(mp);
        drawDeployCursor(window, mouseWorld);
    }

    // HUD & UI
    drawHUD(window);
    drawUIBar(window);

    if (m_phase == GamePhase::GameOver || m_phase == GamePhase::Victory) {
        drawGameOver(window);
    }
}

// ---------------------------------------------------------------
// HUD (atas)
// ---------------------------------------------------------------
void PlayState::drawHUD(sf::RenderWindow& window) {
    // Panel HUD atas tengah (seperti gambar referensi)
    sf::RectangleShape panel({ 280.f, 36.f });
    panel.setFillColor(sf::Color(60, 60, 60, 210));
    panel.setOutlineColor(sf::Color(120, 120, 120));
    panel.setOutlineThickness(1.5f);
    panel.setPosition((float)WINDOW_WIDTH * 0.5f - 140.f, 5.f);
    window.draw(panel);

    std::ostringstream ss;
    ss << "ENEMY: " << m_spawned << "/" << WAVE_ENEMY_COUNT
       << "    HP: " << m_baseHP;

    if (m_fontLoaded) {
        sf::Text txt;
        txt.setFont(m_font);
        txt.setString(ss.str());
        txt.setCharacterSize(18);
        txt.setFillColor(sf::Color::White);
        txt.setPosition((float)WINDOW_WIDTH * 0.5f - 130.f, 10.f);
        window.draw(txt);
    } else {
        // Fallback: gambar teks sebagai bar kotak berwarna
        // (saat font tidak tersedia, kita gambar indikator HP manual)
        float hpPct = (float)m_baseHP / BASE_MAX_HP;
        sf::RectangleShape hpBar({ 260.f * hpPct, 14.f });
        hpBar.setFillColor(sf::Color(60, 200, 60));
        hpBar.setPosition((float)WINDOW_WIDTH * 0.5f - 130.f, 16.f);
        window.draw(hpBar);
    }
}

// ---------------------------------------------------------------
// UI Bar bawah (kartu troop + MONEY)
// ---------------------------------------------------------------
void PlayState::drawUIBar(sf::RenderWindow& window) {
    float barTop = (float)GAME_AREA_HEIGHT;

    // Background bar
    sf::RectangleShape bg({ (float)WINDOW_WIDTH, (float)UI_BAR_HEIGHT });
    bg.setFillColor(sf::Color(80, 80, 80, 230));
    bg.setPosition(0.f, barTop);
    window.draw(bg);

    // Separator line
    sf::RectangleShape line({ (float)WINDOW_WIDTH, 2.f });
    line.setFillColor(sf::Color(120, 120, 120));
    line.setPosition(0.f, barTop);
    window.draw(line);

    // --- Kartu troop ---
    float cardW = 120.f, cardH = 90.f;
    float startX = (float)WINDOW_WIDTH * 0.5f - 240.f;
    float startY = barTop + 10.f;

    for (int i = 0; i < (int)m_cards.size(); ++i) {
        auto& card = m_cards[i];
        float cx = startX + i * (cardW + 5.f);
        bool  selected  = (m_selectedCard == i);
        bool  canAfford = (m_money >= card.cost) && !card.name.empty();
        bool  maxReached= ((int)m_troops.size() >= MAX_TROOPS);

        // Kartu background
        sf::RectangleShape crd({ cardW, cardH });
        sf::Color bgColor = card.color;
        if (!canAfford || maxReached) bgColor = sf::Color(50, 50, 50); // greyed out
        if (selected) bgColor = sf::Color(200, 200, 60);
        crd.setFillColor(bgColor);
        crd.setOutlineColor(selected ? sf::Color::Yellow : sf::Color(150, 150, 150));
        crd.setOutlineThickness(selected ? 3.f : 1.5f);
        crd.setPosition(cx, startY);
        window.draw(crd);

        if (!card.name.empty()) {
            // Icon placeholder (lingkaran kecil troop)
            sf::CircleShape icon(22.f);
            icon.setFillColor(sf::Color(80, 200, 100));
            icon.setOrigin(22.f, 22.f);
            icon.setPosition(cx + cardW * 0.5f, startY + 32.f);
            window.draw(icon);

            // Cost badge bawah
            sf::RectangleShape badge({ 40.f, 18.f });
            badge.setFillColor(sf::Color(30, 30, 30, 200));
            badge.setPosition(cx + cardW * 0.5f - 20.f, startY + cardH - 22.f);
            window.draw(badge);

            if (m_fontLoaded) {
                sf::Text costTxt;
                costTxt.setFont(m_font);
                costTxt.setString(std::to_string(card.cost));
                costTxt.setCharacterSize(14);
                costTxt.setFillColor(canAfford && !maxReached
                                     ? sf::Color(255, 220, 50)
                                     : sf::Color(150, 150, 150));
                costTxt.setPosition(cx + cardW * 0.5f - 8.f, startY + cardH - 22.f);
                window.draw(costTxt);

                sf::Text nameTxt;
                nameTxt.setFont(m_font);
                nameTxt.setString(card.name);
                nameTxt.setCharacterSize(11);
                nameTxt.setFillColor(sf::Color::White);
                nameTxt.setPosition(cx + 8.f, startY + 3.f);
                window.draw(nameTxt);
            }
        }
    }

    // --- MONEY panel (kanan) ---
    sf::RectangleShape moneyPanel({ 120.f, 90.f });
    moneyPanel.setFillColor(sf::Color(60, 60, 60));
    moneyPanel.setOutlineColor(sf::Color(150, 150, 150));
    moneyPanel.setOutlineThickness(1.5f);
    moneyPanel.setPosition((float)WINDOW_WIDTH - 130.f, barTop + 10.f);
    window.draw(moneyPanel);

    if (m_fontLoaded) {
        sf::Text moneyLabel;
        moneyLabel.setFont(m_font);
        moneyLabel.setString("MONEY");
        moneyLabel.setCharacterSize(16);
        moneyLabel.setFillColor(sf::Color(200, 200, 200));
        moneyLabel.setPosition((float)WINDOW_WIDTH - 115.f, barTop + 22.f);
        window.draw(moneyLabel);

        sf::Text moneyVal;
        moneyVal.setFont(m_font);
        moneyVal.setString(std::to_string(m_money));
        moneyVal.setCharacterSize(28);
        moneyVal.setFillColor(sf::Color(255, 230, 50));
        moneyVal.setPosition((float)WINDOW_WIDTH - 105.f, barTop + 46.f);
        window.draw(moneyVal);
    } else {
        // Fallback tanpa font: gambar balok kuning sebagai "uang"
        float mw = std::min((float)m_money * 3.f, 100.f);
        sf::RectangleShape mBar({ mw, 14.f });
        mBar.setFillColor(sf::Color(255, 220, 50));
        mBar.setPosition((float)WINDOW_WIDTH - 120.f, barTop + 55.f);
        window.draw(mBar);
    }

    // --- Troop count indicator ---
    if (m_fontLoaded) {
        sf::Text troopCount;
        troopCount.setFont(m_font);
        troopCount.setString("Troops: " + std::to_string(m_troops.size())
                             + "/" + std::to_string(MAX_TROOPS));
        troopCount.setCharacterSize(13);
        troopCount.setFillColor(sf::Color(180, 255, 180));
        troopCount.setPosition(10.f, barTop + 10.f);
        window.draw(troopCount);
    }
}

// ---------------------------------------------------------------
// Cursor deploy preview
// ---------------------------------------------------------------
void PlayState::drawDeployCursor(sf::RenderWindow& window, sf::Vector2f mousePos) {
    bool valid = m_map.isDeployable(mousePos) &&
                 (int)m_troops.size() < MAX_TROOPS &&
                 m_selectedCard >= 0 &&
                 m_money >= m_cards[m_selectedCard].cost;

    sf::Color col = valid ? sf::Color(100, 255, 100, 160)
                          : sf::Color(255, 80, 80, 160);

    sf::CircleShape preview(22.f);
    preview.setFillColor(col);
    preview.setOutlineColor(sf::Color::White);
    preview.setOutlineThickness(2.f);
    preview.setOrigin(22.f, 22.f);
    preview.setPosition(mousePos);
    window.draw(preview);
}

// ---------------------------------------------------------------
// Game Over / Victory overlay
// ---------------------------------------------------------------
void PlayState::drawGameOver(sf::RenderWindow& window) {
    // Semi-transparent overlay
    sf::RectangleShape overlay({ (float)WINDOW_WIDTH, (float)GAME_AREA_HEIGHT });
    overlay.setFillColor(sf::Color(0, 0, 0, 160));
    window.draw(overlay);

    if (!m_fontLoaded) return;

    bool won = (m_phase == GamePhase::Victory);
    std::string title = won ? "VICTORY!" : "GAME OVER";
    sf::Color   titleCol = won ? sf::Color(100, 255, 100) : sf::Color(255, 80, 80);

    sf::Text t1;
    t1.setFont(m_font);
    t1.setString(title);
    t1.setCharacterSize(64);
    t1.setFillColor(titleCol);
    sf::FloatRect tb = t1.getLocalBounds();
    t1.setOrigin(tb.width * 0.5f, tb.height * 0.5f);
    t1.setPosition((float)WINDOW_WIDTH * 0.5f, (float)GAME_AREA_HEIGHT * 0.4f);
    window.draw(t1);

    sf::Text t2;
    t2.setFont(m_font);
    t2.setString("Press R to restart");
    t2.setCharacterSize(24);
    t2.setFillColor(sf::Color::White);
    sf::FloatRect tb2 = t2.getLocalBounds();
    t2.setOrigin(tb2.width * 0.5f, tb2.height * 0.5f);
    t2.setPosition((float)WINDOW_WIDTH * 0.5f, (float)GAME_AREA_HEIGHT * 0.6f);
    window.draw(t2);
}

sf::Text PlayState::makeText(const std::string& str, unsigned size,
                              sf::Color col, sf::Vector2f pos) {
    sf::Text t;
    if (m_fontLoaded) t.setFont(m_font);
    t.setString(str);
    t.setCharacterSize(size);
    t.setFillColor(col);
    t.setPosition(pos);
    return t;
}