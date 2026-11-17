
#pragma once

#include <SFML/Graphics.hpp>
#include "sorting/SortingEngine.hpp"

#include <vector>
#include <cstddef>

class Visualizer
{
public:
    Visualizer();
    void run();

private:
    void handleEvents();
    void update();
    void render();
    void renderHeader();
    void reset();
    void selectAlgorithm(sorting::Algorithm algorithm);
    

    sf::Font font;
    sf::RenderWindow window;
    sf::Clock animationClock;

    const std::vector<int> original = {
        45, 12, 78, 34, 90,
        23, 67, 15, 56, 82,
        38, 71, 29, 95, 43,
        61, 18, 84, 52, 36
    };

    std::vector<int> values;
    std::vector<sorting::SortEvent> events;

    sorting::SortStats stats;
    sorting::Algorithm selectedAlgorithm = sorting::Algorithm::Bubble;

    std::size_t eventIndex = 0;
    bool playing = false;

    sf::Time eventDelay = sf::milliseconds(80);

    int highlightedA = -1;
    int highlightedB = -1;
};
