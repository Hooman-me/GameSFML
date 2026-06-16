#include <SFML/Graphics.hpp>
#include "Playstate.hpp"
#include "Constants.hpp"

int main() {
    sf::RenderWindow window(
        sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT),
        "Tower Defense – CatBlaze vs Slimes",
        sf::Style::Titlebar | sf::Style::Close);
    window.setFramerateLimit(60);

    PlayState game;
    sf::Clock clock;

    while (window.isOpen()) {
        float dt = clock.restart().asSeconds();
        if (dt > 0.05f) dt = 0.05f; // clamp untuk mencegah lag spike

        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
            game.handleEvent(event, window);
        }

        game.update(dt);
        window.clear();
        game.draw(window);
        window.display();
    }
    return 0;
}