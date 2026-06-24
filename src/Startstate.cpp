#include "Startstate.hpp"
#include "Constants.hpp"

StartScreen::StartScreen() {
    // Load gambar background dari folder assets/StartScreen/
    if (m_texBg.loadFromFile("assets/StartScreen/ARKFLOPS.png")) {
        m_sprBg.setTexture(m_texBg);
    }

    // Load font menggunakan path konstan kamu
    if (m_font.loadFromFile(FONT_REAL_PATH)) {
        m_text.setFont(m_font);
        m_text.setString("PRESS ENTER TO START");
        m_text.setCharacterSize(28);
        m_text.setFillColor(sf::Color::White);
        
        // Tengahkan titik acuan tulisan
        sf::FloatRect bounds = m_text.getLocalBounds();
        m_text.setOrigin(bounds.width / 2.f, bounds.height / 2.f);
        
        // Letakkan tulisan agak ke bawah (sekitar 80% dari tinggi layar)
        m_text.setPosition(WINDOW_WIDTH / 2.f, WINDOW_HEIGHT * 0.8f);
    }
}

void StartScreen::handleEvent(const sf::Event& event, bool& startGame) {
    if (event.type == sf::Event::KeyPressed) {
        if (event.key.code == sf::Keyboard::Enter) {
            startGame = true; // Ubah state untuk masuk ke game
        }
    }
}

void StartScreen::update(float dt, sf::RenderWindow& window) {
    // 1. Logika AFK 30 detik
    m_idleTimer += dt;
    if (m_idleTimer >= 30.f) {
        window.close(); // Tutup game otomatis
    }

    // 2. Logika teks kedip-kedip (toggle tiap 0.6 detik)
    m_blinkTimer += dt;
    if (m_blinkTimer >= 0.6f) {
        m_blinkTimer = 0.f;
        m_showText = !m_showText;
    }
}

void StartScreen::draw(sf::RenderWindow& window) {
    window.draw(m_sprBg);
    // Gambar tulisan hanya jika m_showText sedang true
    if (m_showText) {
        window.draw(m_text);
    }
}