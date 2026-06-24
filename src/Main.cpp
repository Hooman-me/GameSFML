#include <SFML/Graphics.hpp>
#include "Playstate.hpp"
#include "Startstate.hpp"
#include "Endingstate.hpp"
#include "Constants.hpp"

enum class AppState { Start, Playing, Ending };

int main() {
    sf::RenderWindow window(
        sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT),
        "Tower Defense  CatBlaze vs Slimes",
        sf::Style::Titlebar | sf::Style::Close);
    window.setFramerateLimit(60);

    StartScreen startScreen;
    PlayState game;
    EndingState endScreen;

    AppState currentState = AppState::Start; 
    sf::Clock clock;

    while (window.isOpen()) {
        float dt = clock.restart().asSeconds();
        if (dt > 0.05f) dt = 0.05f;
        
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) window.close();
            
            // --- Distribusi Event Berdasarkan State ---
            if (currentState == AppState::Start) {
                bool startGame = false;
                startScreen.handleEvent(event, startGame);
                if (startGame) currentState = AppState::Playing;
            } 
            else if (currentState == AppState::Playing) {
                game.handleEvent(event, window);
            }
            else if (currentState == AppState::Ending) {
                bool wantsRestart = false;
                // Lempar variabel wantsRestart ke EndingState
                endScreen.handleEvent(event, window, wantsRestart);
                
                // Jika player menekan 'R', reset PlayState dan ganti layar
                if (wantsRestart) {
                    game.reset(); 
                    currentState = AppState::Playing;
                }
            }
        }
        
        // --- Distribusi Update Logic ---
        if (currentState == AppState::Start) {
            startScreen.update(dt, window);
        } 
        else if (currentState == AppState::Playing) {
            game.update(dt);
            
            GamePhase phase = game.getPhase();
            if (phase == GamePhase::Victory || phase == GamePhase::GameOver) {
                endScreen.setVictory(phase == GamePhase::Victory);
                currentState = AppState::Ending;
            }
        }

        // --- Proses Rendering ---
        window.clear();
        
        if (currentState == AppState::Start) {
            startScreen.draw(window);
        } 
        else if (currentState == AppState::Playing) {
            game.draw(window);
        }
        else if (currentState == AppState::Ending) {
            game.draw(window); 
            endScreen.draw(window); 
        }
        
        window.display();
    }
    return 0;
}