#include "visualizer.hpp"

#include <algorithm>
#include <array>
#include <chrono>
#include <cctype>
#include <cstdlib>
#include <filesystem>
#include <iomanip>
#include <random>
#include <sstream>
#include <stdexcept>
#include <string>

namespace {

bool isPrintableAscii(char32_t unicode) {
    return unicode >= 32 && unicode < 127;
}

}  // namespace

std::vector<int> Visualizer::defaultArray() {
    return {
        45, 12, 78, 34, 90,
        23, 67, 15, 56, 82,
        38, 71, 29, 95, 43,
        61, 18, 84, 52, 36
    };
}

Visualizer::Visualizer()
    : window(
        sf::VideoMode({1000, 820}),
        "Sorting Algorithm Visualizer"
      ),
      original(defaultArray()),
      values(original),
      statusMessage("Ready - press Space")
{
    window.setFramerateLimit(60);
    window.setKeyRepeatEnabled(false);
    loadFont();

    selectAlgorithm(selectedAlgorithm);
}

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

void Visualizer::handleEvents()
{
    while (const auto event = window.pollEvent())
    {
        if (event->is<sf::Event::Closed>())
        {
            window.close();
            return;
        }

        if (inputMode != InputMode::Normal)
        {
            if (const auto* text =
                event->getIf<sf::Event::TextEntered>())
            {
                handleTextEntered(text->unicode);
            }

            if (const auto* key =
                event->getIf<sf::Event::KeyPressed>())
            {
                if (key->code == sf::Keyboard::Key::Enter)
                {
                    commitTextInput();
                }
                else if (key->code == sf::Keyboard::Key::Escape)
                {
                    cancelTextInput();
                }
                else if (key->code == sf::Keyboard::Key::Backspace)
                {
                    if (!inputBuffer.empty())
                    {
                        inputBuffer.pop_back();
                    }
                }
            }
            continue;
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
                statusMessage = playing ? "Playing" : "Paused";
            }

            if (key->code == sf::Keyboard::Key::R)
            {
                reset();
                statusMessage = "Reset";
            }

            if (key->code == sf::Keyboard::Key::N ||
                key->code == sf::Keyboard::Key::Right)
            {
                playing = false;
                stepOnce();
                statusMessage = isComplete()
                    ? "Complete - Space to replay"
                    : "Stepped";
            }

            if (key->code == sf::Keyboard::Key::G)
            {
                beginEnterRandomSize();
            }

            if (key->code == sf::Keyboard::Key::I)
            {
                beginEnterArray();
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

void Visualizer::beginEnterArray()
{
    playing = false;
    inputMode = InputMode::EnterArray;
    inputBuffer.clear();
    statusMessage = "Enter integers separated by spaces/commas";
}

void Visualizer::beginEnterRandomSize()
{
    playing = false;
    inputMode = InputMode::EnterRandomSize;
    inputBuffer = std::to_string(static_cast<int>(kDefaultRandomSize));
    statusMessage =
        "Enter random size (" +
        std::to_string(kMinRandomSize) + "-" +
        std::to_string(kMaxRandomSize) + "), Enter to confirm";
}

void Visualizer::cancelTextInput()
{
    inputMode = InputMode::Normal;
    inputBuffer.clear();
    statusMessage = "Input cancelled";
}

void Visualizer::handleTextEntered(char32_t unicode)
{
    if (unicode == '\b' || unicode == 127)
    {
        if (!inputBuffer.empty())
        {
            inputBuffer.pop_back();
        }
        return;
    }

    if (!isPrintableAscii(unicode))
    {
        return;
    }

    const char ch = static_cast<char>(unicode);

    if (inputMode == InputMode::EnterRandomSize)
    {
        if (std::isdigit(static_cast<unsigned char>(ch)) &&
            inputBuffer.size() < 3)
        {
            inputBuffer.push_back(ch);
        }
        return;
    }

    // EnterArray: digits, spaces, commas, minus
    if (std::isdigit(static_cast<unsigned char>(ch)) ||
        ch == ' ' || ch == ',' || ch == '-')
    {
        if (inputBuffer.size() < 256)
        {
            inputBuffer.push_back(ch);
        }
    }
}

void Visualizer::commitTextInput()
{
    if (inputMode == InputMode::EnterRandomSize)
    {
        try
        {
            if (inputBuffer.empty())
            {
                throw std::invalid_argument("empty");
            }
            const int size = std::stoi(inputBuffer);
            if (size < static_cast<int>(kMinRandomSize) ||
                size > static_cast<int>(kMaxRandomSize))
            {
                throw std::out_of_range("size");
            }
            generateRandomArray(static_cast<std::size_t>(size));
            statusMessage =
                "Random array of size " + std::to_string(size);
        }
        catch (...)
        {
            statusMessage =
                "Invalid size; use " +
                std::to_string(kMinRandomSize) + "-" +
                std::to_string(kMaxRandomSize);
            inputMode = InputMode::Normal;
            inputBuffer.clear();
            return;
        }
    }
    else if (inputMode == InputMode::EnterArray)
    {
        try
        {
            applyUserArray(inputBuffer);
            statusMessage =
                "Custom array loaded (" +
                std::to_string(original.size()) + " elements)";
        }
        catch (const std::exception& ex)
        {
            statusMessage = std::string("Input error: ") + ex.what();
            inputMode = InputMode::Normal;
            inputBuffer.clear();
            return;
        }
    }

    inputMode = InputMode::Normal;
    inputBuffer.clear();
}

void Visualizer::generateRandomArray(std::size_t size)
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dist(1, 99);

    original.clear();
    original.reserve(size);
    for (std::size_t i = 0; i < size; ++i)
    {
        original.push_back(dist(gen));
    }

    preparePlayback();
}

void Visualizer::applyUserArray(const std::string& text)
{
    std::vector<int> parsed;
    std::string token;

    auto flush = [&]() {
        if (token.empty())
        {
            return;
        }
        parsed.push_back(std::stoi(token));
        token.clear();
    };

    for (char ch : text)
    {
        if (ch == ',' || ch == ' ' || ch == '\t')
        {
            flush();
        }
        else
        {
            token.push_back(ch);
        }
    }
    flush();

    if (parsed.empty())
    {
        throw std::invalid_argument("need at least one integer");
    }
    if (parsed.size() > kMaxRandomSize)
    {
        throw std::out_of_range("array too large (max 80)");
    }

    original = std::move(parsed);
    preparePlayback();
}

void Visualizer::preparePlayback()
{
    events.clear();
    values = original;
    eventIndex = 0;
    playing = false;
    highlightedA = -1;
    highlightedB = -1;
    animationClock.restart();

    if (original.empty())
    {
        stats = {};
        sortingTimeMs = 0.0;
        return;
    }

    std::vector<int> sortedCopy = original;

    sorting::EventSink sink =
        [&](const sorting::SortEvent& event)
    {
        events.push_back(event);
    };

    const auto playbackStats = sorting::sort(
        selectedAlgorithm,
        sortedCopy,
        &sink
    );

    auto warmup = original;
    sorting::sort(selectedAlgorithm, warmup, nullptr);

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

        stats = measuredStats;
    }

    std::sort(durations.begin(), durations.end());
    sortingTimeMs = durations[benchmarkTrials / 2];
}

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
    preparePlayback();
    statusMessage = "Algorithm selected";
}

void Visualizer::applyEvent(const sorting::SortEvent& current)
{
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
}

void Visualizer::stepOnce()
{
    if (eventIndex >= events.size())
    {
        return;
    }

    applyEvent(events[eventIndex]);
    eventIndex++;
    animationClock.restart();

    if (eventIndex >= events.size())
    {
        playing = false;
    }
}

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

    stepOnce();

    if (eventIndex >= events.size())
    {
        statusMessage = "Complete - Space to replay";
    }
}

void Visualizer::render()
{
    window.clear(sf::Color(25, 25, 35));

    const float windowWidth = 1000.f;
    const float bottom = 510.f;
    const float maxBarHeight = 390.f;
    const float spacing = 4.f;

    if (!values.empty())
    {
        int maxValue = 1;
        for (int v : values)
        {
            maxValue = std::max(maxValue, std::abs(v));
        }

        const float barWidth =
            (windowWidth - spacing * static_cast<float>(values.size()))
            / static_cast<float>(values.size());

        for (std::size_t i = 0; i < values.size(); i++)
        {
            float height =
                (static_cast<float>(values[i]) / static_cast<float>(maxValue))
                * maxBarHeight;

            sf::RectangleShape bar({barWidth, height});

            bar.setPosition({
                static_cast<float>(i) * (barWidth + spacing),
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
    }

    renderHeader();
    renderPerformance();
    renderControls();
    if (inputMode != InputMode::Normal)
    {
        renderInputOverlay();
    }

    window.display();
}

bool Visualizer::isComplete() const
{
    return !events.empty() && eventIndex >= events.size();
}

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

void Visualizer::renderHeader()
{
    const sf::Color ink(235, 239, 250);
    const sf::Color muted(165, 176, 199);

    drawText(
        "Sorting Algorithm Visualizer",
        24.f, 18.f, 27, ink
    );

    drawText(
        "G = random   I = custom array   N/Right = step",
        24.f, 57.f, 16, muted
    );

    std::string status = statusMessage;
    if (inputMode == InputMode::Normal)
    {
        if (isComplete())
            status = "Complete - Space to replay";
        else if (playing)
            status = "Playing";
        else if (eventIndex > 0 && statusMessage == "Ready - press Space")
            status = "Paused";
    }

    drawText(
        status,
        520.f, 25.f, 16,
        sf::Color(255, 200, 60)
    );

    drawText(
        "Playback delay: " +
        std::to_string(eventDelay.asMilliseconds()) + " ms",
        650.f, 57.f, 16, muted
    );
}

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
        "Space = Play/Pause   N/Right = Step   R = Reset   Up/Down = Speed",
        24.f, 749.f, 17, ink
    );

    drawText(
        "1/2/3 = Bubble/Merge/Quick   G = Random   I = Enter array   Esc = Cancel input",
        24.f, 779.f, 15, muted
    );
}

void Visualizer::renderInputOverlay()
{
    sf::RectangleShape dim({1000.f, 820.f});
    dim.setFillColor(sf::Color(0, 0, 0, 160));
    window.draw(dim);

    sf::RectangleShape box({860.f, 140.f});
    box.setPosition({70.f, 320.f});
    box.setFillColor(sf::Color(35, 41, 57));
    window.draw(box);

    const sf::Color ink(235, 239, 250);
    const sf::Color muted(165, 176, 199);

    const char* title = inputMode == InputMode::EnterRandomSize
        ? "Random array size"
        : "Custom integer array";

    drawText(title, 90.f, 335.f, 22, ink);
    drawText(
        inputMode == InputMode::EnterRandomSize
            ? "Type a size, then Enter. Esc cancels."
            : "Type integers (spaces or commas). Enter confirms. Esc cancels.",
        90.f, 370.f, 16, muted
    );

    const std::string shown =
        inputBuffer.empty() ? "_" : (inputBuffer + "_");
    drawText(shown, 90.f, 410.f, 22, sf::Color(255, 200, 60));
}
