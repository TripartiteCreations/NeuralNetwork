#include "Neurons.h"

static unsigned int neuron_counter = 0;

Neurons::Neurons() {
    neuron_id = neuron_counter++;
}



void Neurons::connectTo(Neurons* n2, float weight) {
   
    auto c = std::make_unique<Connection>();
    c->to = n2;
    c->from = this;
    c->weight = weight;
    std::random_device rd;


    std::mt19937 gen(rd());

  
    std::uniform_real_distribution<> distrib(10.0, 100.0);

    c->delay = distrib(gen);
    connect.push_back(std::move(c));
    //cout << connect.size() << endl;

}


void Neurons::sendSignal(float v, float d) {

    auto s = std::make_unique<Signal>();
    s->signal = v;
    s->delay = d;
    s->sent_T = simulation_time;
   
    signals.push_back(std::move(s));

    //cout << signal.size() <<endl;

}

int Neurons::find_connection(Neurons* from, Neurons* to) {
    for (int i = 0; i < connect.size(); ++i) {
        if (connect.at(i)->from == from && connect.at(i)->to == to) {
            return i;
        }
    }
    return -1;
}



void Neurons::update(float t) {
    simulation_time = t;
    update_count++;

    // Track recovery if neuron was dead
    if (was_dead && isAlive()) {
        frames_since_death++;
        recovery_progress = std::min(recovery_progress + 0.01f, 1.0f);
    }

    // Decay existing activity and input
    this->activity_record *= a_r_d;
    input *= decay;

    float s = 0;
    for (int i = 0; i < signals.size(); ++i) {
        float delay = signals.at(i)->delay;
        float st = signals.at(i)->sent_T;

        s = signals.at(i)->signal;
        float CRT = t - st;

        if (CRT > delay) {
            this->activity_record += a_r;
            this->current_RT = t;

            // Apply adaptive normalization to incoming signal
            float adapted_signal = applyAdaptiveNormalization(s);

            input += adapted_signal;
            signals.erase(signals.begin() + i);
            i--;
        }
    }

    // Calculate signal coherence before propagating
    float coherence = calculateSignalCoherence();
    updateHealth(coherence);

    propagate();
}



void Neurons::propagate() {
    int C_S = connect.size();


    for (int i = 0; i < C_S; ++i) {
        Connection* c = connect[i].get();

        STDP(c);
        //c->to->input *= decay;
        c->weight = util.clamp(c->weight, -1, 1);
        c->weight *= weight_decay;
       // std::cout << "c : " << c->delay << std::endl;
        if (input != 0) {
            c->to->sendSignal(input * c->weight, c->delay);
        }


    }
    
}



void Neurons::STDP(Connection* c) {
    Neurons* from = c->from;
    Neurons* to = c->to;
    float f_signal_T = from->current_RT;
    float t_signal_T = to->current_RT;
    float spike_diff = t_signal_T - f_signal_T;

    if (t_signal_T == 0) spike_diff = f_signal_T;

    
    float tau = 20.0f; // Time constant for STDP
    float A_pos = 0.001f; // Positive learning rate
    float A_neg = 0.001f; // Negative learning rate 


    if (spike_diff >= 0) {
        // Post-synaptic neuron fires after pre-synaptic
        changes = A_pos * std::exp(-spike_diff / tau);
    } else {
        // Post-synaptic neuron fires before pre-synaptic
        changes = -A_neg * std::exp(spike_diff / tau);
    }

    if (changes != 0) {
        std::cout << "Neuron " << from->neuron_id << " -> " << to->neuron_id 
                  << " spike_diff: " << spike_diff << " weight_change: " << changes << std::endl;
    }

    c->weight += changes;
}




void Neurons::Hebbian(Connection* c) {
    Neurons* from = c->from;
    Neurons* to = c->to;
    float f_signal_T = from->current_RT;
    float t_signal_T = to->current_RT;
    bool f_f = firing_threshold(from->input, from);
    bool f_t = firing_threshold(to->input, to);
    if (f_f && f_t) {
        bool ts = (f_signal_T == t_signal_T);
        if (ts) c->weight += 0.001f;
    }
   
}


bool Neurons::firing_threshold(float c_i, Neurons* n) {
    if ((c_i < 0 && c_i < -n->F_T) || (c_i > 0 && c_i > n->F_T)) {
        return true;
    }
    return false;
}




// setters and getters

float Neurons::getInput() {
    return this->input;
}

float Neurons::getHealth() const {
    return health;
}

bool Neurons::isAlive() const {
    return health > 0.0f;
}

float Neurons::getSignalSensitivity() const {
    return signal_sensitivity;
}

float Neurons::getDeathCauseType() const {
    return death_cause;
}

// Adaptive signal normalization based on what caused the neuron's death
// Adaptive signal normalization (Homeostasis)
float Neurons::applyAdaptiveNormalization(float incoming_signal) {
    // Proactively regulate the signal based on current sensitivity.
    // If sensitivity > 1.0, it amplifies (neuron is understimulated).
    // If sensitivity < 1.0, it dampens (neuron is overstimulated).
    return incoming_signal * signal_sensitivity;
}

float Neurons::calculateSignalCoherence() {
    // Measure coherence based on actual signal reception and activity level
    // Neurons need signals to survive, but too much noise (chaos) kills them

    // Track how many signals this neuron received
    int received_signals = signals.size();

    // Calculate activity level from input magnitude
    float activity_level = std::abs(input);

    // Check if neuron is in "Goldilocks zone" - not too active, not too silent
    float activity_score = 1.0f;

    if (activity_level < ACTIVITY_OPTIMAL_MIN) {
        // Too silent - neuron is not receiving enough signal
        activity_score = activity_level / ACTIVITY_OPTIMAL_MIN;
    } else if (activity_level > ACTIVITY_OPTIMAL_MAX) {
        // Too active - chaotic input, excessive stimulation
        activity_score = 1.0f - ((activity_level - ACTIVITY_OPTIMAL_MAX) / 2.0f);
        activity_score = std::max(activity_score, 0.0f);
    }
    // else: in optimal range, score stays at 1.0f

    // Detection: are signals being received at all?
    float signal_received_score = (received_signals > 0) ? 1.0f : 0.0f;

    // Combine scores: need both signal reception and optimal activity level
    float coherence = (activity_score * 0.7f + signal_received_score * 0.3f);

    return std::min(std::max(coherence, 0.0f), 1.0f);
}


void Neurons::updateHealth(float coherence) {
    signal_consistency = coherence;

    // Calculate current activity level
    float activity_level = std::abs(input);

    // Are we getting any stimulation at all?
    bool receiving_signals = (signals.size() > 0) || (activity_level > 0.0f);

    // --- 1. PROACTIVE REGULATION (HOMEOSTASIS) ---
    // The neuron constantly tries to keep itself in the Goldilocks zone
    if (receiving_signals) {
        if (activity_level > ACTIVITY_OPTIMAL_MAX) {
            // Overstimulated: Regulate by dampening incoming signals
            signal_sensitivity = std::max(signal_sensitivity - SENSITIVITY_RECOVERY_RATE, MIN_SENSITIVITY);
        }
        else if (activity_level < ACTIVITY_OPTIMAL_MIN) {
            // Understimulated: Regulate by amplifying incoming signals
            signal_sensitivity = std::min(signal_sensitivity + SENSITIVITY_RECOVERY_RATE, MAX_SENSITIVITY);
        }
        else {
            // Optimal: Gradually drift sensitivity back to normal (1.0)
            if (signal_sensitivity > 1.01f) signal_sensitivity -= SENSITIVITY_RECOVERY_RATE * 0.5f;
            else if (signal_sensitivity < 0.99f) signal_sensitivity += SENSITIVITY_RECOVERY_RATE * 0.5f;
            else signal_sensitivity = 1.0f;
        }
    }

    // --- 2. SURVIVAL & DEATH MECHANICS ---
    if (!isAlive()) {
        return; // The neuron is dead, stop updating health.
    }

    float extreme_chaos_threshold = ACTIVITY_OPTIMAL_MAX * 3.0f;

    if (!receiving_signals && activity_level == 0.0f) {
   
        health = std::max(health - HEALTH_LOSS_SILENT, 0.0f);
        if (health <= 0.0f) {
            death_cause = -1.0f;
            std::cout << " Neuron " << neuron_id << " DIED from TOTAL SILENCE\n";
        }
    }
    else if (activity_level > extreme_chaos_threshold) {
        // FATAL: Extreme Chaos (so high that dampening couldn't save it)
        health = std::max(health - HEALTH_LOSS_CHAOS, 0.0f);
        chaos_accumulation = std::min(chaos_accumulation + 0.05f, 1.0f);

        if (health <= 0.0f) {
            death_cause = 1.0f;
            std::cout << " Neuron " << neuron_id << " DIED from EXTREME CHAOS\n";
        }
    }
    else {
        // SURVIVAL: Neuron is either in the optimal range or successfully regulating
        health = std::min(health + HEALTH_GAIN_OPTIMAL, 1.0f);
        chaos_accumulation = std::max(chaos_accumulation - 0.02f, 0.0f);
    }

    // Penalty for neurons with no outgoing connections (dead weight)
    if (connect.empty() && update_count > 20) {
        health *= 0.99f;
    }

    // Debug: log health changes periodically
    if (update_count % 100 == 0 && update_count > 0) {
        std::cout << "Neuron " << neuron_id
            << " | Health: " << health
            << " | Activity: " << activity_level
            << " | Sensitivity: " << signal_sensitivity
            << " | Status: " << (isAlive() ? "ALIVE" : "DEAD") << std::endl;
    }
}