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

    // Get adaptation state for visualization
    float getSignalSensitivity() const;
    float getDeathCauseType() const;  // Returns: 0 = normal, 1 = overstimulation, -1 = understimulation

    float changes;

    // Unique identifier for each neuron
    unsigned int neuron_id;

private:
    void STDP(Connection* c);
    void Hebbian(Connection* c);
    
    float calculateSignalCoherence();
    void applyEvolutionaryPressure(float coherence);
    float applyAdaptiveNormalization(float incoming_signal);  // Adapt signal based on death cause

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
    float health = 0.5f; // Health ranges from 0 to 1, starts at 0.5f
    float signal_consistency = 0.0f; // Tracks consistency of incoming signals
    float chaos_accumulation = 0.0f; // Accumulates chaos (noise)
    unsigned int update_count = 0; // Number of updates performed
    float last_signal_magnitude = 0.0f; // Track received signal magnitude
    float signal_reception_quality = 0.5f; // Quality of received signals

    // Adaptive behavior tracking
    float death_cause = 0.0f;  // 0 = normal, 1.0f = overstimulation, -1.0f = understimulation
    float signal_sensitivity = 1.0f;  // How sensitive neuron is to incoming signals (1.0 = normal)
    bool was_dead = false;  // Track if neuron has recovered from death
    float recovery_progress = 0.0f;  // Progress towards full recovery (0-1)
    int frames_since_death = 0;  // Frames since neuron died

    // Constants for survival - Goldilocks zone for activity
    static constexpr float HEALTH_GAIN_OPTIMAL = 0.015f; // Health gain in optimal signal range
    static constexpr float HEALTH_LOSS_CHAOS = 0.003f; // Health loss in chaotic signals
    static constexpr float HEALTH_LOSS_SILENT = 0.0002f; // Health loss when no signals received
    static constexpr float ACTIVITY_OPTIMAL_MIN = 0.2f; // Minimum optimal activity
    static constexpr float ACTIVITY_OPTIMAL_MAX = 0.8f; // Maximum optimal activity
    static constexpr float CHAOS_THRESHOLD = 0.6f; // Threshold for chaos detection (high variance)

    // Adaptive behavior constants
    static constexpr float MAX_SENSITIVITY = 2.0f;  // Max signal amplification for understimulated neurons
    static constexpr float MIN_SENSITIVITY = 0.3f;  // Min signal dampening for overstimulated neurons
    static constexpr float SENSITIVITY_RECOVERY_RATE = 0.02f;  // Speed of recovery to normal sensitivity
};

