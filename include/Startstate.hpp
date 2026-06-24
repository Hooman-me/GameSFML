#pragma once
#include <SFML/Graphics.hpp>

class StartScreen {
public:
    StartScreen();
    
    // startGame dioper sebagai referensi agar StartScreen bisa mengubah status di Main.cpp
    void handleEvent(const sf::Event& event, bool& startGame);
    void update(float dt, sf::RenderWindow& window);
    void draw(sf::RenderWindow& window);

private:
    sf::Texture m_texBg;
    sf::Sprite  m_sprBg;
    sf::Font    m_font;
    sf::Text    m_text;

    float m_blinkTimer = 0.f;
    bool  m_showText   = true;
    float m_idleTimer  = 0.f;
};