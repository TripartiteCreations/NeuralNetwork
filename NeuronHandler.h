#pragma once
#include "Neurons.h"
#include <vector>
#include "NeuralNetworkVisualizer.h"
#include <random> 
class NeuronHandler {
public:
	NeuronHandler(int num_neurons, NeuralNetworkVisualizer& visualizer) : num_neurons(num_neurons), visualizer(visualizer) {
		this->visualizer = visualizer;
		for (int i = 0; i < num_neurons; ++i) {
			neurons.emplace_back();
			visualizer.addNeuron(i);	
		}
	}
	void connectNeurons() {
		for (int from = 0; from < neurons.size(); ++from) {

			for (int to = 0; to < neurons.size(); ++to) {
				if (from == to) continue; 
				std::random_device rd;


				std::mt19937 gen(rd());


				std::uniform_real_distribution<> distrib(0, 1);
				float probability = distrib(gen);
				if (probability < 0.2) { // 50% chance to connect
					
					neurons[from].connectTo(&neurons[to], 0.5); // Connect with weight 0.5
					visualizer.addConnection(from, to, 0.5);
				}
			}


		}



	}
	

	std::vector<Neurons> neurons;
	int num_neurons;
	NeuralNetworkVisualizer& visualizer;
};