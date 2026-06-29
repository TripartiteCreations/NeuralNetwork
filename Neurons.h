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
    
 

    // Unique identifier for each neuron
    unsigned int neuron_id;

private:
    void STDP(Connection* c);
	void applySTDP();
	void SpikeHandler(float t);
    void RewardHandler(float reward, float coherence);
    float calculateSignalCoherence();
    void createConnection();

    float applyAdaptiveNormalization(float incoming_signal);  // Adapt signal based on death cause

    float input = 0;
	float neuron_availability_timer = 0.6; // timer for neuron availability decay
	float neuron_availability = 0.2; // neuron availability for firing
	float n_availability_decay = 0.01f; // neuron availability decay rate

    float base_amptitude = 0.8;
	float reward = 0;
    float r_r = 0.01f; //reward rate
    float reward_decay = 0.9;
	float signal_decay = 0.99999f; // incoming signal decay
    
    float excitability = 1; 
    float ex_decay = 0.01f;

	
    
    float simulation_time = 0;
    float current_RT = 0; // current receiving time of the current neuron
    float decay = 0.5f;
    float weight_decay = 0.99999; // w decay

    float time_decay = 0.1;
    float F_T = 0.6f; //firing threshold

    float activity_record = 0;
    float a_r_d = 0.9995; //activity record decay
    float a_r = 0.01; // activity record increment per activation

    

    float signal_consistency = 0.0f; // Tracks consistency of incoming signals
    
    unsigned int update_count = 0; // Number of updates performed
    
	float target_activity = 0.6f; // Target activity level for optimal neuron function
	float learning_rate = 0.01f; // Learning rate for STDP adjustments

    float chaos_accumulation = 0.0f; // Accumulates chaos (noise)
	float chaos_scale = 0.001f; // Scale for chaos accumulation

	float trace_firing = 0.0f; // Tracks recent firing events for STDP
	float trace_decay = 20; // Decay rate for firing trace
    static constexpr float ACTIVITY_OPTIMAL_MIN = 0.2f; // Minimum optimal activity
    static constexpr float ACTIVITY_OPTIMAL_MAX = 0.8f; // Maximum optimal activity
    static constexpr float CHAOS_THRESHOLD = 0.6f; // Threshold for chaos detection (high variance)

    // Adaptive behavior constants
    static constexpr float MAX_SENSITIVITY = 2.0f;  // Max signal amplification for understimulated neurons
    static constexpr float MIN_SENSITIVITY = 0.3f;  // Min signal dampening for overstimulated neurons
    static constexpr float SENSITIVITY_RECOVERY_RATE = 0.02f;  // Speed of recovery to normal sensitivity
};

