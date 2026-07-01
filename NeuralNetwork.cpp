
#include <iostream>
#include <SDL3/SDL.h>

#include "NeuralNetworkVisualizer.h"
#include "Neurons.h"
#include "GeneticAlgorithm.h"

#include "NeuronHandler.h"

int main()
{
    std::cout << "=== Spiking Neural Network ===\n";
    std::cout << "Neurons survive in Goldilocks zone (moderate activity + constant signals)\n";
    std::cout << "They die from: isolation (no signals) OR chaos (too much noise)\n\n";

    // Initialize visualizer
    NeuralNetworkVisualizer visualizer(1200, 800);
    SDL_Window* window = visualizer.getWindow();

    float time = 0;
    int num_neurons = 4;
    NeuronHandler neuronHandler(num_neurons, visualizer);


    neuronHandler.connectNeurons();

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

                    if (neuron_idx >= 0 && neuron_idx < static_cast<int>(neuronHandler.neurons.size())) {
                        // Send signal to clicked neuron
                        neuronHandler.neurons[neuron_idx].sendSignal(1, 1);

                        std::cout << "Clicked Neuron " << neuron_idx
                            << " - Sending signal 0.6f\n";
                    }
                }
            }
        }

        time += 1.f;
        frame_count++;


        // test stdp pos fire before pre
        if (frame_count % 200 == 0) {
            neuronHandler.neurons[1].sendSignal(1.0f, 0.5f);  // Moderate signal
        }
        if (frame_count % 120 == 0) {
            neuronHandler.neurons[0].sendSignal(1.0f, 0.5f);  // Moderate signal
        }


        // Add occasional spike (still within acceptable range)
        if (frame_count % 25 == 0) {
            //neurons[0].sendSignal(1, 2);
        }

        // to test chaotic behavior (neurons will die)
        if (frame_count % 3 == 0) {
            //neurons[0].sendSignal(2.0f, 1); // Excessive noise - kills neurons
        }

        // Update all neurons
        for (int i = 0; i < neuronHandler.neurons.size(); ++i) {

            neuronHandler.neurons[i].update(time);

            // Update visualizer with neuron data
            visualizer.updateNeuronActivity(i, neuronHandler.neurons[i].getInput());

            // Update connection weights
            auto& connections = neuronHandler.neurons[i].connect;
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

        // Render
        visualizer.render();


    }



    return 0;
}
