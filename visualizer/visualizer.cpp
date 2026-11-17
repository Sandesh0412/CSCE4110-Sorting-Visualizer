
#include "Visualizer.hpp"

#include <algorithm>
#include <stdexcept>
#include <string>

// Create the window and prepare Bubble Sort events
Visualizer::Visualizer()

    : window(
        sf::VideoMode({1000, 700}),
        "Sorting Algorithm Visualizer"
      ),
      values(original)
{
    window.setFramerateLimit(60);

    selectAlgorithm(selectedAlgorithm);
}

// Main application loop
void Visualizer::run()
{
    while (window.isOpen())
    {
        handleEvents();
        update();
        render();
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

    stats = sorting::sort(
        selectedAlgorithm,
        sortedCopy,
        &sink
    );
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

// Draw the sorting bars
void Visualizer::render()
{
    window.clear(sf::Color(25, 25, 35));

    const float windowWidth = 1000.f;
    const float bottom = 650.f;
    const float maxBarHeight = 500.f;
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

    window.display();
}
