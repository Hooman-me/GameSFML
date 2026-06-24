#pragma once
#include <SFML/Graphics.hpp>

class EndingState {
public:
    EndingState();
    
    void setVictory(bool isVictory); 
    // --- DIUBAH: Tambahan parameter wantsRestart ---
    void handleEvent(const sf::Event& event, sf::RenderWindow& window, bool& wantsRestart);
    void draw(sf::RenderWindow& window);

private:
    sf::Font m_font;
    bool     m_fontLoaded = false;
    sf::Text m_mainText;
    
    // --- DIUBAH: Dua teks terpisah ---
    sf::Text m_subTextEsc;
    sf::Text m_subTextR;
};