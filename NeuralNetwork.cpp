
#include <iostream>
#include <SFML/Graphics.hpp>
#include "GraphVisualizer.h"
#include "Neurons.h"
int main()
{
    GraphVisualizer visualizer(800, 600);
    std::cout << "Neural Network with Survival Mechanism\n";
    float time = 0;
    std::vector <Neurons> n;
    int num_Neurons = 5;

    // Create neurons
    for (int i = 0; i < num_Neurons; ++i) { 
        n.push_back(Neurons()); 
        visualizer.addGraph(30); 
    }

    // Create network topology
    n.at(0).connectTo(&n.at(1), 0.5);
    n.at(1).connectTo(&n.at(2), 0.5);
    n.at(2).connectTo(&n.at(3), 0.5);
    n.at(3).connectTo(&n.at(4), 0.5);
    n.at(4).connectTo(&n.at(0), 0.3); // Create feedback loop

    // Initial signals
    n.at(0).sendSignal(5, 10);
    n.at(1).sendSignal(5, 60);

    sf::RenderWindow window(sf::VideoMode({ 800, 600 }), "Neural Network - Survival Mechanism");

    while (window.isOpen())
    {
        // Handle events
        while (const std::optional event = window.pollEvent())
        {
            // Periodic structured input (structured signal helps neurons survive)
            if (static_cast<int>(time) % 15 == 0) {
                n.at(0).sendSignal(5, 1);
            }

            if (event->is<sf::Event::Closed>())
                window.close();
        }

        time += 1.f;

        // Update all neurons
        for (int i = 0; i < n.size(); ++i) {
            if (n.at(i).isAlive()) {
                n.at(i).update(time);
                visualizer.addValue(i, n.at(i).getInput(), n.at(i).getHealth());

                // Log health status periodically
                if (static_cast<int>(time) % 50 == 0) {
                    std::cout << "Neuron " << i << " - Health: " << n.at(i).getHealth() 
                              << " Input: " << n.at(i).getInput() << std::endl;
                }
            }
        }

        window.clear(sf::Color::Black);
        visualizer.draw(window);
        window.display();
    }

    std::cout << "\nFinal Status:\n";
    for (int i = 0; i < n.size(); ++i) {
        std::cout << "Neuron " << i << " - Alive: " << (n.at(i).isAlive() ? "Yes" : "No") 
                  << " Health: " << n.at(i).getHealth() << std::endl;
    }
}


