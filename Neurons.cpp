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

    // Create a Mersenne Twister engine, seeded by the random device
    std::mt19937 gen(rd());

    // Define a uniform integer distribution for the desired range (e.g., 1 to 100)
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

            input += s;
            signals.erase(signals.begin() + i);
            i--; // Adjust index after erase
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

    // More accurate STDP curve with proper exponential decay
    float tau = 20.0f; // Time constant for STDP
    float A_pos = 0.01f; // Positive learning rate
    float A_neg = 0.01f; // Negative learning rate

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

float Neurons::calculateSignalCoherence() {
    // Measure coherence based on activity patterns and signal consistency
    // High coherence = structured signals, Low coherence = chaos

    if (update_count < 10) return 0.5f; // Neutral coherence during initial phase

    // Coherence based on activity record and signal consistency
    float activity_coherence = std::min(activity_record, 1.0f);

    // Measure deviation of input signals (lower deviation = more structured)
    float input_variance = std::abs(input); // Simplified: measure of signal stability
    float variance_coherence = 1.0f / (1.0f + input_variance);

    // Combine metrics
    float total_coherence = (activity_coherence * 0.6f + variance_coherence * 0.4f);

    return std::min(std::max(total_coherence, 0.0f), 1.0f);
}

void Neurons::updateHealth(float coherence) {
    // Update signal consistency tracking
    signal_consistency = coherence;

    if (coherence > COHERENCE_THRESHOLD) {
        // Structured signal - neuron thrives
        health = std::min(health + HEALTH_GAIN_STRUCTURED, 1.0f);
        chaos_accumulation = std::max(chaos_accumulation - 0.01f, 0.0f);
    } else {
        // Chaotic signal - neuron decays
        health = std::max(health - HEALTH_LOSS_CHAOS, 0.0f);
        chaos_accumulation = std::min(chaos_accumulation + 0.02f, 1.0f);
    }

    // Apply evolutionary pressure based on connection strength
    float avg_weight = 0.0f;
    if (!connect.empty()) {
        for (const auto& c : connect) {
            avg_weight += std::abs(c->weight);
        }
        avg_weight /= connect.size();
    }

    // Neurons with weak connections lose health faster
    if (avg_weight < 0.1f && !connect.empty()) {
        health *= 0.98f;
    }
}
