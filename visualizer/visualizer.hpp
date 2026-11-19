#pragma once

#include <SFML/Graphics.hpp>
#include "sorting/SortingEngine.hpp"

#include <vector>
#include <cstddef>
#include <string>

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
    void renderPerformance();
    void renderControls();

    void drawText(
        const std::string& text,
        float x,
        float y,
        unsigned int size,
        sf::Color color
    );

    void loadFont();
    bool isComplete() const;
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

    double sortingTimeMs = 0.0;
    static constexpr int benchmarkTrials = 31;

    sorting::Algorithm selectedAlgorithm =
        sorting::Algorithm::Bubble;

    std::size_t eventIndex = 0;
    bool playing = false;

    sf::Time eventDelay = sf::milliseconds(80);

    int highlightedA = -1;
    int highlightedB = -1;
};
