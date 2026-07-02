#pragma once
#include "Neurons.h"
#include <vector>
#include "NeuralNetworkVisualizer.h"
#include <random> 
#include "utility.h"
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
				
				float probability = util.random_float(0, 1);
				float weight = util.random_float(0.1, 1.0); // Random weight between 0.1 and 1.0
				if (probability < 0.2) { // 50% chance to connect
					std::cout << "Connecting Neuron " << from << " to Neuron " << to << " with weight: " << weight << std::endl;
					neurons[from].connectTo(&neurons[to], weight); // Connect with random weight
					visualizer.addConnection(from, to, weight);
				}
			}


		}



	}
	
	utility util;
	std::vector<Neurons> neurons;
	int num_neurons;
	NeuralNetworkVisualizer& visualizer;
};