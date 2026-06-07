#pragma once
#include <SDL3/SDL.h>
#include <vector>
#include <memory>
#include <cmath>
#include <algorithm>
#include <queue>
#include <iostream>


const float PI = 3.14159265359f;

struct Color {
    Uint8 r, g, b, a;
    Color(Uint8 r = 255, Uint8 g = 255, Uint8 b = 255, Uint8 a = 255)
        : r(r), g(g), b(b), a(a) {}
};

// Neuron visualization data
struct NeuronVisual {
    float x, y;                          
    float radius;                       
    std::queue<float> activity_history;  
    std::size_t max_history;            
    float health;                    
    bool alive;                         

    NeuronVisual(float x, float y, float radius, std::size_t history_size = 50)
        : x(x), y(y), radius(radius), max_history(history_size), health(0.5f), alive(true) {}

    void addActivity(float activity) {
        activity_history.push(activity);
        if (activity_history.size() > max_history) {
            activity_history.pop();
        }
    }

    void setHealth(float h) {
        health = std::max(0.0f, std::min(1.0f, h));
    }
};

// Connection visualization data
struct ConnectionVisual {
    std::size_t from_idx;
    std::size_t to_idx;
    float weight;
    float visual_weight;  // For smooth animation

    ConnectionVisual(std::size_t from, std::size_t to, float w = 0.5f)
        : from_idx(from), to_idx(to), weight(w), visual_weight(w) {}
};

class NeuralNetworkVisualizer {
private:
    SDL_Window* window;
    SDL_Renderer* renderer;
    std::vector<NeuronVisual> neurons;
    std::vector<ConnectionVisual> connections;

    int window_width;
    int window_height;
    float center_x;
    float center_y;
    float network_radius;

public:
    NeuralNetworkVisualizer(int width = 1200, int height = 800)
        : window(nullptr), renderer(nullptr), window_width(width), window_height(height),
          center_x(width / 2.0f), center_y(height / 2.0f), network_radius(250.0f) {
        initSDL();
    }

    ~NeuralNetworkVisualizer() {
        if (renderer) SDL_DestroyRenderer(renderer);
        if (window) SDL_DestroyWindow(window);
        SDL_Quit();
    }

    // Initialize SDL3
    void initSDL() {
        if (!SDL_Init(SDL_INIT_VIDEO)) {
            std::cerr << "Failed to initialize SDL: " << SDL_GetError() << std::endl;
            return;
        }

        window = SDL_CreateWindow(
            "Neural Network Visualizer",
            window_width, window_height,
            SDL_WINDOW_OPENGL
        );

        if (!window) {
            std::cerr << "Failed to create window: " << SDL_GetError() << std::endl;
            return;
        }

        renderer = SDL_CreateRenderer(window, nullptr);
        if (!renderer) {
            std::cerr << "Failed to create renderer: " << SDL_GetError() << std::endl;
            return;
        }

        SDL_SetRenderVSync(renderer, 1);  // Enable VSync
    }

    // Add a neuron to visualization
    void addNeuron(std::size_t index) {
        // Arrange neurons in a circle
        float angle = (2.0f * PI * index) / (index + 1);
        float x = center_x + network_radius * std::cos(angle);
        float y = center_y + network_radius * std::sin(angle);

        neurons.emplace_back(x, y, 25.0f, 60);  // 25px radius, 60 samples history
    }

    // Add a connection between neurons
    void addConnection(std::size_t from, std::size_t to, float weight = 0.5f) {
        if (from < neurons.size() && to < neurons.size()) {
            connections.emplace_back(from, to, weight);
        }
    }

    // Update neuron activity (called every frame)
    void updateNeuronActivity(std::size_t index, float activity, float health, bool alive) {
        if (index < neurons.size()) {
            neurons[index].addActivity(activity);
            neurons[index].setHealth(health);
            neurons[index].alive = alive;
        }
    }

    // Update connection weight visualization
    void updateConnectionWeight(std::size_t from, std::size_t to, float weight) {
        for (auto& conn : connections) {
            if (conn.from_idx == from && conn.to_idx == to) {
                conn.weight = weight;
                // Smooth animation
                conn.visual_weight = conn.visual_weight * 0.85f + weight * 0.15f;
                break;
            }
        }
    }

    //// Check if neuron was clicked and return neuron index, or -1 if no click
    // Pass the mouse coordinates from SDL_EVENT_MOUSE_BUTTON_DOWN
    int checkNeuronClick(int mouse_x, int mouse_y) const {
        for (std::size_t i = 0; i < neurons.size(); ++i) {
            float dx = mouse_x - neurons[i].x;
            float dy = mouse_y - neurons[i].y;
            float dist = std::sqrt(dx * dx + dy * dy);

            // Click radius is neuron radius + 10px for easier clicking...
            if (dist <= neurons[i].radius + 10.0f) {
                return static_cast<int>(i);
            }
        }
        return -1;
    }

    // Draw the entire network
    void render() {
        // Clear background
        SDL_SetRenderDrawColor(renderer, 10, 10, 15, 255);
        SDL_RenderClear(renderer);

        // Draw grid background
        drawGrid();

        // Draw connections first (so they appear behind neurons)
        drawConnections();

        // Draw neurons
        drawNeurons();

        // Present frame
        SDL_RenderPresent(renderer);
    }

    bool isWindowOpen() {
        return window != nullptr;
    }

    SDL_Window* getWindow() {
        return window;
    }

private:
    // Draw grid background
    void drawGrid() {
        SDL_SetRenderDrawColor(renderer, 30, 30, 40, 255);
        int grid_size = 50;

        for (int x = 0; x < window_width; x += grid_size) {
            SDL_RenderLine(renderer, x, 0, x, window_height);
        }
        for (int y = 0; y < window_height; y += grid_size) {
            SDL_RenderLine(renderer, 0, y, window_width, y);
        }
    }

    // Draw all connections
    void drawConnections() {
        for (const auto& conn : connections) {
            if (conn.from_idx >= neurons.size() || conn.to_idx >= neurons.size()) continue;

            const NeuronVisual& from = neurons[conn.from_idx];
            const NeuronVisual& to = neurons[conn.to_idx];

            // Color based on weight (positive = green, negative = red)
            float abs_weight = std::abs(conn.visual_weight);
            if (conn.visual_weight > 0) {
                drawLineGradient(
                    from.x, from.y, to.x, to.y,
                    Color(0, 200 * abs_weight, 100, 150),
                    Color(100, 255, 150, 200)
                );
            } else {
                drawLineGradient(
                    from.x, from.y, to.x, to.y,
                    Color(200 * abs_weight, 0, 100, 150),
                    Color(255, 100, 150, 200)
                );
            }

            // Draw line thickness based on weight magnitude
            float thickness = 1.0f + std::abs(conn.visual_weight) * 3.0f;
            drawThickLine(from.x, from.y, to.x, to.y, thickness,
                         conn.visual_weight > 0 ? Color(100, 255, 150) : Color(255, 100, 150));
        }
    }

    // Draw all neurons
    void drawNeurons() {
        for (std::size_t i = 0; i < neurons.size(); ++i) {
            const NeuronVisual& neuron = neurons[i];

            // Draw activity graph above neuron
            drawActivityGraph(neuron, i);

            // Draw neuron circle
            drawNeuron(neuron);
        }
    }

    // Draw a single neuron as a circle
    void drawNeuron(const NeuronVisual& neuron) {
        // Determine color based on health
        Color circle_color;
        if (!neuron.alive) {
            circle_color = Color(100, 100, 100, 200);  // Gray for dead
        } else if (neuron.health > 0.7f) {
            circle_color = Color(0, 255, 0, 255);      // Green for healthy
        } else if (neuron.health > 0.4f) {
            circle_color = Color(255, 200, 0, 255);    // Yellow for medium
        } else {
            circle_color = Color(255, 100, 0, 255);    // Orange for weak
        }

        // Draw filled circle
        drawFilledCircle(neuron.x, neuron.y, neuron.radius, circle_color);

        // Draw border
        drawCircle(neuron.x, neuron.y, neuron.radius, Color(255, 255, 255, 255), 2.0f);

        // Draw health indicator ring
        float health_angle = 2.0f * PI * neuron.health;
        drawArc(neuron.x, neuron.y, neuron.radius + 5.0f, 0, health_angle,
               Color(0, 200, 255, 200), 3.0f);
    }

    // Draw activity waveform graph above neuron
    void drawActivityGraph(const NeuronVisual& neuron, std::size_t neuron_idx) {
        if (neuron.activity_history.empty()) return;

        float graph_width = 80.0f;
        float graph_height = 40.0f;
        float graph_x = neuron.x - graph_width / 2.0f;
        float graph_y = neuron.y - neuron.radius - 50.0f;
        float padding = 3.0f;  // Padding inside the graph container

        // Draw graph background
        SDL_FRect bg_rect = {graph_x, graph_y, graph_width, graph_height};
        SDL_SetRenderDrawColor(renderer, 20, 20, 30, 200);
        SDL_RenderFillRect(renderer, &bg_rect);

        // Draw graph border
        drawRect(graph_x, graph_y, graph_width, graph_height, Color(100, 100, 150, 200), 1.0f);

        // Convert queue to vector for easier access
        std::queue<float> temp_queue = neuron.activity_history;
        std::vector<float> history;
        while (!temp_queue.empty()) {
            history.push_back(temp_queue.front());
            temp_queue.pop();
        }

        // Draw waveform with clipping to container
        if (history.size() > 1) {
            float inner_width = graph_width - 2 * padding;
            float inner_height = graph_height - 2 * padding;
            float inner_x = graph_x + padding;
            float inner_y = graph_y + padding;
            float x_step = inner_width / (history.size() - 1);
            float max_activity = 1.0f;  // Normalized to 0-1

            for (std::size_t i = 0; i < history.size() - 1; ++i) {
                // Calculate points
                float x1 = inner_x + i * x_step;
                float y1 = inner_y + inner_height - (history[i] / max_activity) * inner_height;
                float x2 = inner_x + (i + 1) * x_step;
                float y2 = inner_y + inner_height - (history[i + 1] / max_activity) * inner_height;

                // Clamp points to container bounds
                y1 = std::max(inner_y, std::min(y1, inner_y + inner_height));
                y2 = std::max(inner_y, std::min(y2, inner_y + inner_height));
                x1 = std::max(inner_x, std::min(x1, inner_x + inner_width));
                x2 = std::max(inner_x, std::min(x2, inner_x + inner_width));

                // Color based on activity level
                Color wave_color;
                if (history[i] > 0.6f) {
                    wave_color = Color(0, 255, 150);  // Green for high activity
                } else if (history[i] > 0.3f) {
                    wave_color = Color(100, 200, 255);  // Cyan for medium
                } else {
                    wave_color = Color(255, 100, 200);  // Magenta for low
                }

                drawThickLine(x1, y1, x2, y2, 1.5f, wave_color);
            }
        }

        // Draw baseline (centered line)
        float baseline_y = graph_y + graph_height / 2.0f;
        drawThickLine(graph_x + padding, baseline_y,
                     graph_x + graph_width - padding, baseline_y,
                     0.5f, Color(100, 100, 100, 100));
    }

    // === Drawing Primitives ===

    // Draw filled circle using Midpoint Circle Algorithm
    void drawFilledCircle(float cx, float cy, float radius, Color color) {
        SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);

        int r = static_cast<int>(radius);
        for (int y = -r; y <= r; ++y) {
            int x = static_cast<int>(std::sqrt(r * r - y * y));
            SDL_RenderLine(renderer, cx - x, cy + y, cx + x, cy + y);
        }
    }

    // Draw circle outline
    void drawCircle(float cx, float cy, float radius, Color color, float thickness = 1.0f) {
        SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);

        int r = static_cast<int>(radius);
        int x = 0;
        int y = r;
        int d = 3 - 2 * r;

        while (x <= y) {
            drawThickLine(cx + x, cy - y, cx + x, cy + y, thickness, color);
            drawThickLine(cx - x, cy - y, cx - x, cy + y, thickness, color);
            drawThickLine(cx + y, cy - x, cx + y, cy + x, thickness, color);
            drawThickLine(cx - y, cy - x, cx - y, cy + x, thickness, color);

            if (d < 0) {
                d = d + 4 * x + 6;
            } else {
                d = d + 4 * (x - y) + 10;
                y--;
            }
            x++;
        }
    }

    // Draw arc (for health indicator)
    void drawArc(float cx, float cy, float radius, float start_angle, float end_angle,
                Color color, float thickness = 2.0f) {
        int points = static_cast<int>((end_angle - start_angle) * radius / PI);
        points = std::max(points, 10);

        for (int i = 0; i < points; ++i) {
            float angle1 = start_angle + (end_angle - start_angle) * i / points;
            float angle2 = start_angle + (end_angle - start_angle) * (i + 1) / points;

            float x1 = cx + radius * std::cos(angle1);
            float y1 = cy + radius * std::sin(angle1);
            float x2 = cx + radius * std::cos(angle2);
            float y2 = cy + radius * std::sin(angle2);

            drawThickLine(x1, y1, x2, y2, thickness, color);
        }
    }

    // Draw rectangle
    void drawRect(float x, float y, float w, float h, Color color, float thickness = 1.0f) {
        drawThickLine(x, y, x + w, y, thickness, color);
        drawThickLine(x + w, y, x + w, y + h, thickness, color);
        drawThickLine(x + w, y + h, x, y + h, thickness, color);
        drawThickLine(x, y + h, x, y, thickness, color);
    }

    // Draw line with custom thickness
    void drawThickLine(float x1, float y1, float x2, float y2, float thickness, Color color) {
        SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);

        if (thickness <= 1.0f) {
            SDL_RenderLine(renderer, x1, y1, x2, y2);
            return;
        }

        float dx = x2 - x1;
        float dy = y2 - y1;
        float dist = std::sqrt(dx * dx + dy * dy);

        if (dist < 0.001f) return;

        float px = -dy / dist * thickness / 2.0f;
        float py = dx / dist * thickness / 2.0f;

        // Draw as filled polygon (using line approximation for simplicity)
        for (float offset = -thickness / 2.0f; offset <= thickness / 2.0f; offset += 1.0f) {
            float off_x = -dy / dist * offset;
            float off_y = dx / dist * offset;
            SDL_RenderLine(renderer, x1 + off_x, y1 + off_y, x2 + off_x, y2 + off_y);
        }
    }

    // Draw line with gradient color
    void drawLineGradient(float x1, float y1, float x2, float y2, Color c1, Color c2) {
        int steps = 20;
        for (int i = 0; i < steps; ++i) {
            float t = static_cast<float>(i) / steps;
            float x_start = x1 + (x2 - x1) * t;
            float y_start = y1 + (y2 - y1) * t;
            float x_end = x1 + (x2 - x1) * (t + 1.0f / steps);
            float y_end = y1 + (y2 - y1) * (t + 1.0f / steps);

            Uint8 r = c1.r + (c2.r - c1.r) * t;
            Uint8 g = c1.g + (c2.g - c1.g) * t;
            Uint8 b = c1.b + (c2.b - c1.b) * t;

            drawThickLine(x_start, y_start, x_end, y_end, 1.0f, Color(r, g, b, 200));
        }
    }
};
