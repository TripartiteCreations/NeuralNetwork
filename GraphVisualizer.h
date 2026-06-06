#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <memory>
#include <cmath>
#include <algorithm>

class GraphVisualizer {
private:
    struct Graph {
        std::vector<float> data;
        std::size_t maxSamples;
        sf::VertexArray line;
        sf::RectangleShape container;
        sf::Vector2f position;
        sf::Vector2f size;
        float padding = 10.f;
        float changes = 0;

        Graph(std::size_t sampleCount,
            const sf::Vector2f& pos,
            const sf::Vector2f& sz)
            : data(sampleCount, 0.f),
            maxSamples(sampleCount),
            line(sf::PrimitiveType::LineStrip, sampleCount),
            position(pos),
            size(sz)
        {
            container.setPosition(pos);
            container.setSize(sz);
            container.setFillColor(sf::Color(255, 255, 255));
            container.setOutlineColor(sf::Color(1, 80, 80));
            container.setOutlineThickness(2.f);
            updateVertices();
        }

        void addValue(float value) {
            // push new value, drop oldest
            data.erase(data.begin());
            data.push_back(value);
            updateVertices();
        }

        void updateVertices() {
            if (data.empty()) return;

            float maxVal = *std::max_element(data.begin(), data.end());
            float minVal = *std::min_element(data.begin(), data.end());
            float range = std::max(0.0001f, maxVal - minVal);

            const float innerWidth = size.x - 2 * padding;
            const float innerHeight = size.y - 2 * padding;

            for (std::size_t i = 0; i < data.size(); ++i) {
                float normalizedX = (data.size() > 1)
                    ? static_cast<float>(i) / (data.size() - 1)
                    : 0.f;
                float normalizedY = (data[i] - minVal) / range;

                float x = position.x + padding + normalizedX * innerWidth;
                float y = position.y + size.y - padding - normalizedY * innerHeight;

                line[i].position = { x, y };
                line[i].color = sf::Color::White;
                if (changes > 0) {


                    line[i].color = sf::Color::Cyan;
                }
                else if (changes < 0) {
                    line[i].color = sf::Color::Red;
                }
            }
        }

        void draw(sf::RenderWindow& window) const {
            window.draw(container);
            window.draw(line);
        }
    };

    std::vector<std::unique_ptr<Graph>> graphs;
    sf::Vector2u windowSize;

public:
    GraphVisualizer(unsigned int width = 800, unsigned int height = 600)
        : windowSize(width, height) {
    }

    // create a new empty graph that will hold up to `sampleCount` samples
    void addGraph(std::size_t sampleCount = 100) {
        graphs.push_back(std::make_unique<Graph>(sampleCount, sf::Vector2f(0, 0), sf::Vector2f(0, 0)));
        updateLayout();
    }

    void updateLayout() {
        if (graphs.empty()) return;

        int count = static_cast<int>(graphs.size());
        int cols = static_cast<int>(std::ceil(std::sqrt(count)));
        int rows = static_cast<int>(std::ceil(static_cast<float>(count) / cols));

        sf::Vector2f cellSize(
            static_cast<float>(windowSize.x) / cols,
            static_cast<float>(windowSize.y) / rows
        );

        for (int i = 0; i < count; ++i) {
            int row = i / cols;
            int col = i % cols;

            sf::Vector2f pos(col * cellSize.x, row * cellSize.y);
            graphs[i]->position = pos;
            graphs[i]->size = cellSize;
            graphs[i]->updateVertices();
        }
    }

    // push a new value to an existing graph
    void addValue(std::size_t index, float value, float changes) {
        if (index < graphs.size()) {
            graphs[index]->addValue(value);
            graphs[index]->changes = changes;
        }
    }

    void draw(sf::RenderWindow& window) {
        for (auto& g : graphs)
            g->draw(window);
    }
};
