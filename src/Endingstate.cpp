#include "Endingstate.hpp"
#include "Constants.hpp"

EndingState::EndingState() {
    m_fontLoaded = m_font.loadFromFile(FONT_REAL_PATH);
    if (m_fontLoaded) {
        // Teks Utama (Victory/Game Over)
        m_mainText.setFont(m_font);
        m_mainText.setCharacterSize(60);

        // Teks [ESC] Quit (Posisi Kiri)
        m_subTextEsc.setFont(m_font);
        m_subTextEsc.setString("[ESC] Quit");
        m_subTextEsc.setCharacterSize(22);
        m_subTextEsc.setFillColor(sf::Color(255, 150, 150)); // Warna agak kemerahan
        auto bEsc = m_subTextEsc.getLocalBounds();
        m_subTextEsc.setOrigin(bEsc.width * .5f, bEsc.height * .5f);
        m_subTextEsc.setPosition((float)WINDOW_WIDTH * 0.4f, (float)GAME_AREA_HEIGHT * 0.58f);

        // Teks [R] Restart (Posisi Kanan)
        m_subTextR.setFont(m_font);
        m_subTextR.setString("[R] Restart");
        m_subTextR.setCharacterSize(22);
        m_subTextR.setFillColor(sf::Color(150, 255, 150)); // Warna agak kehijauan
        auto bR = m_subTextR.getLocalBounds();
        m_subTextR.setOrigin(bR.width * .5f, bR.height * .5f);
        m_subTextR.setPosition((float)WINDOW_WIDTH * 0.6f, (float)GAME_AREA_HEIGHT * 0.58f);
    }
}

void EndingState::setVictory(bool isVictory) {
    if (m_fontLoaded) {
        m_mainText.setString(isVictory ? "VICTORY!" : "GAME OVER");
        m_mainText.setFillColor(isVictory ? sf::Color(100, 255, 100) : sf::Color(255, 80, 80));
        auto b1 = m_mainText.getLocalBounds();
        m_mainText.setOrigin(b1.width * .5f, b1.height * .5f);
        m_mainText.setPosition((float)WINDOW_WIDTH * .5f, (float)GAME_AREA_HEIGHT * .38f);
    }
}

void EndingState::handleEvent(const sf::Event& event, sf::RenderWindow& window, bool& wantsRestart) {
    if (event.type == sf::Event::KeyPressed) {
        if (event.key.code == sf::Keyboard::Escape) {
            window.close(); // Keluar
        } else if (event.key.code == sf::Keyboard::R) {
            wantsRestart = true; // Beri sinyal ke Main.cpp untuk reset
        }
    }
}

void EndingState::draw(sf::RenderWindow& window) {
    sf::RectangleShape ov({(float)WINDOW_WIDTH, (float)GAME_AREA_HEIGHT});
    ov.setFillColor(sf::Color(0, 0, 0, 160)); // Latar sedikit lebih gelap biar teks pop-out
    window.draw(ov);

    if (m_fontLoaded) {
        window.draw(m_mainText);
        window.draw(m_subTextEsc);
        window.draw(m_subTextR);
    }
}