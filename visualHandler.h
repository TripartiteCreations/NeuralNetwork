
#pragma once
#include "NeuralNetworkVisualizer.h"

#include <SDL3/SDL.h>
#include "Neurons.h"

class Neurons;
class Connection;

class visualHandler {
private:
    bool running = true;
    int num_neurons = 10;
    std::vector<Neurons> neurons;
    NeuralNetworkVisualizer visualizer;
    SDL_Window* window;
public:
    
    float time = 0;
    visualHandler() : visualizer(1200, 800), window(visualizer.getWindow()) {
        initialize();
    }
    


    
    void initialize() {
        // Create neurons
        for (int i = 0; i < num_neurons; ++i) {
            neurons.emplace_back();
            visualizer.addNeuron(i);

        }
        addRandomConnection();
    }

    void addRandomConnection() {

        for (int i = 0; i < num_neurons; ++i) {
            for (int j = 0; j < num_neurons; ++j) {
                if (i == j) continue;
                std::random_device rd;


                std::mt19937 gen(rd());


                std::uniform_real_distribution<> distrib(0, 1);
                std::random_device connect;


                std::mt19937 g(connect());


                std::uniform_real_distribution<> d(0, 1);
                float c = d(g);
                if (c > 0.3f) continue;
                float weight = distrib(gen);
                
                neurons[i].connectTo(&neurons[j], weight);
                visualizer.addConnection(i, j, weight);
            }
        }
    }
    void update() {
        while (running && visualizer.isWindowOpen())
        {
            time += 1.f;
            // Handle events
            SDL_Event event;
            while (SDL_PollEvent(&event))
            {
                if (event.type == SDL_EVENT_QUIT) {
                    running = false;
                }
                // Handle mouse clicks on neurons
                else if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
                    if (event.button.button == SDL_BUTTON_LEFT) {
                        // Get mouse position and check for neuron click
                        int neuron_idx = visualizer.checkNeuronClick(event.button.x, event.button.y);

                        if (neuron_idx >= 0 && neuron_idx < static_cast<int>(neurons.size())) {
                            // Send signal to clicked neuron
                            neurons[neuron_idx].sendSignal(0.35f, 1);

                            std::cout << "Clicked Neuron " << neuron_idx
                                << " - Sending signal 0.6f\n";
                        }
                    }
                }
            }

            for (int i = 0; i < neurons.size(); ++i) {

                neurons[i].update(time);

                // Update visualizer with neuron data
                visualizer.updateNeuronActivity(i, neurons[i].getInput());

                // Update connection weights
               
                for (const auto& conn_ptr : neurons[i].connect) {
                    Connection* conn = conn_ptr.get();

                    for (int k = 0; k < num_neurons; ++k) {
                        if (conn->to->neuron_id == k) {
                            visualizer.updateConnectionWeight(i, k, conn->weight);
                        }
                    }
                }
            }

            // Render
            visualizer.render();
        }
    }

};
  