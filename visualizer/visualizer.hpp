#pragma once

#include <SFML/Graphics.hpp>
#include "sorting/SortingEngine.hpp"

#include <cstddef>
#include <string>
#include <vector>

class Visualizer
{
public:
    Visualizer();
    void run();

private:
    enum class InputMode {
        Normal,
        EnterArray,
        EnterRandomSize
    };

    void handleEvents();
    void update();
    void render();
    void renderHeader();
    void renderPerformance();
    void renderControls();
    void renderInputOverlay();

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
    void preparePlayback();
    void applyEvent(const sorting::SortEvent& current);
    void stepOnce();

    void generateRandomArray(std::size_t size);
    void applyUserArray(const std::string& text);
    void beginEnterArray();
    void beginEnterRandomSize();
    void commitTextInput();
    void cancelTextInput();
    void handleTextEntered(char32_t unicode);

    static std::vector<int> defaultArray();

    sf::Font font;
    sf::RenderWindow window;
    sf::Clock animationClock;

    std::vector<int> original;
    std::vector<int> values;
    std::vector<sorting::SortEvent> events;

    sorting::SortStats stats;

    double sortingTimeMs = 0.0;
    static constexpr int benchmarkTrials = 31;
    static constexpr std::size_t kMinRandomSize = 5;
    static constexpr std::size_t kMaxRandomSize = 80;
    static constexpr std::size_t kDefaultRandomSize = 20;

    sorting::Algorithm selectedAlgorithm =
        sorting::Algorithm::Bubble;

    std::size_t eventIndex = 0;
    bool playing = false;

    sf::Time eventDelay = sf::milliseconds(80);

    int highlightedA = -1;
    int highlightedB = -1;

    InputMode inputMode = InputMode::Normal;
    std::string inputBuffer;
    std::string statusMessage;
};
