
#include <iostream>
#include <SDL3/SDL.h>

#include "NeuralNetworkVisualizer.h"
#include "Neurons.h"



int main()
{
    std::cout << "=== Neural Network with Survival Mechanism (SDL3 Visualizer) ===\n";
    std::cout << "Neurons survive in Goldilocks zone (moderate activity + constant signals)\n";
    std::cout << "They die from: isolation (no signals) OR chaos (too much noise)\n\n";

    // Initialize visualizer
    NeuralNetworkVisualizer visualizer(1200, 800);
    SDL_Window* window = visualizer.getWindow();

    float time = 0;
    std::vector<Neurons> neurons;
    int num_neurons = 5;

    // Create neurons
    for (int i = 0; i < num_neurons; ++i) {
        neurons.emplace_back();
        visualizer.addNeuron(i);
    }




    // Create network topology
    neurons[0].connectTo(&neurons[1], 0.5);
    visualizer.addConnection(0, 1, 0.5);

    neurons[1].connectTo(&neurons[2], 0.5);
    visualizer.addConnection(1, 2, 0.5);

    neurons[1].connectTo(&neurons[4], 0.5);
    visualizer.addConnection(1, 4, 0.5);

    neurons[0].connectTo(&neurons[4], 0.5);
    visualizer.addConnection(0, 4, 0.5);

    neurons[4].connectTo(&neurons[3], 0.5);
    visualizer.addConnection(4, 3, 0.5);
    neurons[2].connectTo(&neurons[3], 0.5);
    visualizer.addConnection(2, 3, 0.5);

  

    int frame_count = 0;
    bool running = true;

    while (running && visualizer.isWindowOpen())
    {
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
                        neurons[neuron_idx].sendSignal(0.6f, 1);

                        std::cout << "Clicked Neuron " << neuron_idx 
                                  << " - Sending signal 0.6f\n";
                    }
                }
            }
        }

        time += 1.f;
        frame_count++;


        // === Send signals based on time ===
        // Optimal zone: moderate, consistent signals (every 10 frames)
        if (frame_count % 10 == 0) {
           // neurons[0].sendSignal(0.3f, 0.5f);  // Moderate signal
        }


        // Add occasional spike (still within acceptable range)
        if (frame_count % 25 == 0) {
          ///neurons[0].sendSignal(1, 2);
        }

        // Optional: uncomment to test chaotic behavior (neurons will die)
        if (frame_count % 3 == 0) {
             //neurons[0].sendSignal(2.0f, 1); // Excessive noise - kills neurons
        }

        // Update all neurons
        for (int i = 0; i < neurons.size(); ++i) {
            if (neurons[i].isAlive()) {
                neurons[i].update(time);

                // Update visualizer with neuron data
                visualizer.updateNeuronActivity(i, neurons[i].getInput(), neurons[i].getHealth(), neurons[i].isAlive());

                // Update connection weights
                auto& connections = neurons[i].connect;
                for (std::size_t j = 0; j < connections.size(); ++j) {
                    Connection* conn = connections[j].get();
                    // Find connection index in visualizer
                    for (int k = 0; k < num_neurons; ++k) {
                        if (conn->to->neuron_id == k) {
                            visualizer.updateConnectionWeight(i, k, conn->weight);
                        }
                    }
                }
            }
        }

        // Render
        visualizer.render();

        // Status update every 100 frames
        if (frame_count % 100 == 0) {
            std::cout << "\n--- Time: " << frame_count << " ---\n";
            for (int i = 0; i < neurons.size(); ++i) {
                if (neurons[i].isAlive()) {
                    std::cout << "Neuron " << i << " ALIVE - Health: " << neurons[i].getHealth()
                              << " Input: " << neurons[i].getInput() << std::endl;
                } else {
                    std::cout << "Neuron " << i << " DEAD\n";
                }
            }
        }
    }

    std::cout << "\n=== Final Status ===\n";
    int alive_count = 0;
    for (int i = 0; i < neurons.size(); ++i) {
        std::cout << "Neuron " << i << " - " << (neurons[i].isAlive() ? "ALIVE" : "DEAD")
                  << " | Final Health: " << neurons[i].getHealth() << std::endl;
        if (neurons[i].isAlive()) alive_count++;
    }
    std::cout << "Neurons survived: " << alive_count << "/" << num_neurons << std::endl;

    return 0;
}
