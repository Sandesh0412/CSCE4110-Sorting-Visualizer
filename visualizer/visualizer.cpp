#include "visualizer.hpp"

#include <algorithm>
#include <array>
#include <chrono>
#include <filesystem>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <string>

// Create the window and prepare the visualizer
Visualizer::Visualizer()
    : window(
        sf::VideoMode({1000, 820}),
        "Sorting Algorithm Visualizer"
      ),
      values(original)
{
    window.setFramerateLimit(60);
    window.setKeyRepeatEnabled(false);
    loadFont();

    selectAlgorithm(selectedAlgorithm);
}

// Main application loop
void Visualizer::run()
{
    while (window.isOpen())
    {
        handleEvents();

        if (window.isOpen())
        {
            update();
            render();
        }
    }
}

// Handle keyboard input and closing the window
void Visualizer::handleEvents()
{
    while (const auto event = window.pollEvent())
    {
        if (event->is<sf::Event::Closed>())
        {
            window.close();
        }

        if (const auto* key =
            event->getIf<sf::Event::KeyPressed>())
        {
            if (key->code == sf::Keyboard::Key::Space)
            {
                if (isComplete())
                {
                    reset();
                }

                playing = !playing;
                animationClock.restart();
            }

            if (key->code == sf::Keyboard::Key::R)
            {
                reset();
            }

            if (key->code == sf::Keyboard::Key::Num1)
            {
                selectAlgorithm(sorting::Algorithm::Bubble);
            }

            if (key->code == sf::Keyboard::Key::Num2)
            {
                selectAlgorithm(sorting::Algorithm::Merge);
            }

            if (key->code == sf::Keyboard::Key::Num3)
            {
                selectAlgorithm(sorting::Algorithm::Quick);
            }

            if (key->code == sf::Keyboard::Key::Up)
            {
                int delay = eventDelay.asMilliseconds();

                if (delay > 10)
                {
                    eventDelay = sf::milliseconds(
                        std::max(10, delay - 10)
                    );
                }
            }

            if (key->code == sf::Keyboard::Key::Down)
            {
                int delay = eventDelay.asMilliseconds();

                if (delay < 500)
                {
                    eventDelay = sf::milliseconds(
                        std::min(500, delay + 10)
                    );
                }
            }
        }
    }
}

// Reset the sorting animation
void Visualizer::reset()
{
    values = original;
    eventIndex = 0;
    playing = false;
    highlightedA = -1;
    highlightedB = -1;
    animationClock.restart();
}

// Select an algorithm and prepare its events and benchmarks
void Visualizer::selectAlgorithm(sorting::Algorithm algorithm)
{
    selectedAlgorithm = algorithm;

    reset();
    events.clear();

    std::vector<int> sortedCopy = original;

    sorting::EventSink sink =
        [&](const sorting::SortEvent& event)
    {
        events.push_back(event);
    };

    // Generate events for animation separately from timing
    const auto playbackStats = sorting::sort(
        selectedAlgorithm,
        sortedCopy,
        &sink
    );

    // Warm up the selected algorithm
    auto warmup = original;
    sorting::sort(selectedAlgorithm, warmup, nullptr);

    // Measure 31 runs and use the median
    std::array<double, benchmarkTrials> durations{};

    for (auto& duration : durations)
    {
        auto benchmarkData = original;

        const auto start = std::chrono::steady_clock::now();

        const auto measuredStats = sorting::sort(
            selectedAlgorithm,
            benchmarkData,
            nullptr
        );

        const auto stop = std::chrono::steady_clock::now();

        duration = std::chrono::duration<double, std::milli>(
            stop - start
        ).count();

        if (benchmarkData != sortedCopy ||
            measuredStats.comparisons != playbackStats.comparisons ||
            measuredStats.swaps != playbackStats.swaps ||
            measuredStats.writes != playbackStats.writes)
        {
            throw std::runtime_error(
                "Benchmark and playback results differ"
            );
        }

        // Statistics represent one sort, not all 31 runs
        stats = measuredStats;
    }

    std::sort(durations.begin(), durations.end());
    sortingTimeMs = durations[benchmarkTrials / 2];
}

// Process sorting events
void Visualizer::update()
{
    if (!playing || eventIndex >= events.size())
    {
        return;
    }

    if (animationClock.getElapsedTime() < eventDelay)
    {
        return;
    }

    const sorting::SortEvent& current = events[eventIndex];

    highlightedA = -1;
    highlightedB = -1;

    switch (current.type)
    {
        case sorting::EventType::Compare:
            highlightedA = current.i;
            highlightedB = current.j;
            break;

        case sorting::EventType::Swap:
            std::swap(values[current.i], values[current.j]);
            highlightedA = current.i;
            highlightedB = current.j;
            break;

        case sorting::EventType::Write:
            if (current.j == sorting::kWritePrimary)
            {
                values[current.i] = current.value;
                highlightedA = current.i;
            }
            break;

        case sorting::EventType::Partition:
            highlightedA = current.i;
            highlightedB = current.j;
            break;
    }

    eventIndex++;
    animationClock.restart();

    if (eventIndex >= events.size())
    {
        playing = false;
    }
}

// Draw the sorting bars and interface
void Visualizer::render()
{
    window.clear(sf::Color(25, 25, 35));

    const float windowWidth = 1000.f;
    const float bottom = 510.f;
    const float maxBarHeight = 390.f;
    const float spacing = 4.f;

    const int maxValue = *std::max_element(
        original.begin(),
        original.end()
    );

    const float barWidth =
        (windowWidth - spacing * values.size()) / values.size();

    for (std::size_t i = 0; i < values.size(); i++)
    {
        float height =
            (static_cast<float>(values[i]) / maxValue)
            * maxBarHeight;

        sf::RectangleShape bar({barWidth, height});

        bar.setPosition({
            i * (barWidth + spacing),
            bottom - height
        });

        if (static_cast<int>(i) == highlightedA ||
            static_cast<int>(i) == highlightedB)
        {
            bar.setFillColor(sf::Color(255, 200, 60));
        }
        else
        {
            bar.setFillColor(sf::Color(70, 150, 255));
        }

        window.draw(bar);
    }

    renderHeader();
    renderPerformance();
    renderControls();

    window.display();
}

// Check whether the animation is complete
bool Visualizer::isComplete() const
{
    return !events.empty() && eventIndex >= events.size();
}

// Load the Inter font
void Visualizer::loadFont()
{
    auto directory = std::filesystem::current_path();

    for (int depth = 0; depth < 6; ++depth)
    {
        const auto candidate =
            directory / "assets" / "fonts" / "Inter-Regular.ttf";

        if (std::filesystem::exists(candidate) &&
            font.openFromFile(candidate))
        {
            return;
        }

        const auto parent = directory.parent_path();

        if (parent == directory)
        {
            break;
        }

        directory = parent;
    }

    throw std::runtime_error(
        "Cannot load assets/fonts/Inter-Regular.ttf. "
        "Run from the repository root."
    );
}

// Helper function for drawing text
void Visualizer::drawText(
    const std::string& text,
    float x,
    float y,
    unsigned int size,
    sf::Color color)
{
    sf::Text label(font, text, size);
    label.setPosition({x, y});
    label.setFillColor(color);
    window.draw(label);
}

// Draw the header
void Visualizer::renderHeader()
{
    const sf::Color ink(235, 239, 250);
    const sf::Color muted(165, 176, 199);

    drawText(
        "Sorting Algorithm Visualizer",
        24.f, 18.f, 27, ink
    );

    drawText(
        "Same starting array for all three algorithms",
        24.f, 57.f, 16, muted
    );

    std::string status = "Ready - press Space";

    if (isComplete())
        status = "Complete - Space to replay";
    else if (playing)
        status = "Playing";
    else if (eventIndex > 0)
        status = "Paused";

    drawText(
        status,
        650.f, 25.f, 18,
        sf::Color(255, 200, 60)
    );

    drawText(
        "Playback delay: " +
        std::to_string(eventDelay.asMilliseconds()) + " ms",
        650.f, 57.f, 16, muted
    );
}

// Draw sorting statistics and performance results
void Visualizer::renderPerformance()
{
    sf::RectangleShape panel({952.f, 166.f});
    panel.setPosition({24.f, 540.f});
    panel.setFillColor(sf::Color(35, 41, 57));
    window.draw(panel);

    const sf::Color ink(235, 239, 250);
    const sf::Color muted(165, 176, 199);

    std::string name;

    switch (selectedAlgorithm)
    {
        case sorting::Algorithm::Bubble:
            name = "Bubble Sort";
            break;

        case sorting::Algorithm::Merge:
            name = "Merge Sort";
            break;

        case sorting::Algorithm::Quick:
            name = "Quick Sort";
            break;
    }

    drawText(
        "Algorithm: " + name,
        44.f, 553.f, 22, ink
    );

    drawText(
        "Array size: " + std::to_string(original.size()),
        660.f, 553.f, 22, ink
    );

    if (isComplete())
    {
        std::ostringstream time;

        time << std::fixed
             << std::setprecision(6)
             << sortingTimeMs;

        drawText(
            "Sorting time: " + time.str() + " ms",
            44.f, 591.f, 23,
            sf::Color(97, 218, 172)
        );

        drawText(
            "Comparisons: " +
            std::to_string(stats.comparisons),
            44.f, 630.f, 20, ink
        );

        drawText(
            "Swaps: " + std::to_string(stats.swaps),
            385.f, 630.f, 20, ink
        );

        drawText(
            "Writes: " + std::to_string(stats.writes),
            660.f, 630.f, 20, ink
        );
    }
    else
    {
        drawText(
            "Sorting time: -- (shown when playback completes)",
            44.f, 591.f, 21, muted
        );

        drawText(
            "Comparisons: --",
            44.f, 630.f, 20, muted
        );

        drawText(
            "Swaps: --",
            385.f, 630.f, 20, muted
        );

        drawText(
            "Writes: --",
            660.f, 630.f, 20, muted
        );
    }

    drawText(
        "31-run median | Events disabled | Excludes animation | Counts per sort",
        44.f, 673.f, 14, muted
    );
}

// Draw keyboard instructions
void Visualizer::renderControls()
{
    const sf::Color ink(235, 239, 250);
    const sf::Color muted(165, 176, 199);

    drawText(
        "KEYBOARD CONTROLS",
        24.f, 723.f, 15,
        sf::Color(255, 200, 60)
    );

    drawText(
        "Space = Play / Pause     R = Reset     Up = Faster     Down = Slower",
        24.f, 749.f, 18, ink
    );

    drawText(
        "1 = Bubble Sort     2 = Merge Sort     3 = Quick Sort",
        24.f, 779.f, 17, muted
    );
}
