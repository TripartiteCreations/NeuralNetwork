#pragma once
#include <iostream>
#include <vector>

#include <functional>

#include <cmath>
#include <random> 
#include <algorithm>
#include <ctime>
#include "Connection.h"
#include "utility.h"
#include "Signal.h"







class Neurons {

    utility util;


public:
    Neurons();

    std::vector<std::unique_ptr<Signal>> signals;

    std::vector < std::unique_ptr<Connection>> connect;

    bool firing_threshold(float current_input, Neurons* n);
    void update(float time);
    void connectTo(Neurons* n2, float weight);
    void sendSignal(float v, float delay);
    void propagate();
    int find_connection(Neurons* from, Neurons* to);


    float getInput();
    float getHealth() const;
    bool isAlive() const;
    void updateHealth(float coherence);

    float changes;

    // Unique identifier for each neuron
    unsigned int neuron_id;


private:
    void STDP(Connection* c);
    void Hebbian(Connection* c);
    float calculateSignalCoherence();
    void applyEvolutionaryPressure(float coherence);

    float input = 0;

    float simulation_time = 0;
    float current_RT = 0; // current receiving time of the current neuron
    float decay = 0.5f;
    float weight_decay = 0.99999; // w decay

    float time_decay = 0.1;
    float F_T = 0.3f; //firing threshold


    float a_r_d = 0.9; //activity record decay
    float a_r = 0.1; // activity record increment per activation
    float activity_record = 0;

    // Survival mechanism
    float health = 1.0f; // Health ranges from 0 to 1
    float signal_consistency = 0.0f; // Tracks consistency of incoming signals
    float chaos_accumulation = 0.0f; // Accumulates chaos (noise)
    unsigned int update_count = 0; // Number of updates performed

    // Constants for survival
    static constexpr float HEALTH_GAIN_STRUCTURED = 0.02f; // Health gain in structured signals
    static constexpr float HEALTH_LOSS_CHAOS = 0.01f; // Health loss in chaotic signals
    static constexpr float COHERENCE_THRESHOLD = 0.4f; // Threshold for signal coherence
};

