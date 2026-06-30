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
    s->sender = this;
    //if (!firing_threshold(s->signal, this)) return;
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
    this->excitability += ex_decay * (1.0f-excitability);
    input *= decay;

    float s = 0;
    
    bool fire = false;
    float active_signal_sum = 0.0f;
    std::vector<int> signals_to_remove;

    for (int i = 0; i < signals.size(); ++i) {
        float delay = signals.at(i)->delay;
        float st = signals.at(i)->sent_T;
        float CRT = t - st;

        if (CRT > delay) {
           
            active_signal_sum += signals.at(i)->signal;
            signals_to_remove.push_back(i);
        }
    }

    
    if (!signals_to_remove.empty()) {
        std::cout << "Active Signal Sum: " << active_signal_sum << std::endl;
        fire = firing_threshold(active_signal_sum, this);
    }

   
    if (fire) {
        this->current_RT = t;

        // Apply adaptive normalization 
        float adapted_signal = applyAdaptiveNormalization(active_signal_sum);

        input += 0.5f;

       
        for (int i = signals_to_remove.size() - 1; i >= 0; --i) {
            signals.erase(signals.begin() + signals_to_remove[i]);
        }
    }
 
    

    // Calculate signal coherence before propagating
    float coherence = calculateSignalCoherence();
   
    RewardHandler(reward, coherence);
    propagate();
}



void Neurons::propagate() {
    int C_S = connect.size();


    for (int i = 0; i < C_S; ++i) {
        Connection* c = connect[i].get();

        
        //c->to->input *= decay;
        c->weight = util.clamp(c->weight, -1, 1);
        c->weight *= weight_decay;
       // std::cout << "c : " << c->delay << std::endl;
        if (input != 0) {
            c->to->sendSignal(input * c->weight, c->delay);
        }
        STDP(c);

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

    if (f_signal_T == 0 && t_signal_T == 0) {
        return;
    }
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



// Adaptive signal normalization
float Neurons::applyAdaptiveNormalization(float incoming_signal) {
 
    return incoming_signal * excitability;
}

void Neurons::RewardHandler(float reward, float coherence) {

    excitability += (r_r * reward) * coherence;

}

float Neurons::calculateSignalCoherence() {


    // Track how many signals this neuron received
    int received_signals = signals.size();

    // Calculate activity level from input magnitude
    activity_record += std::abs(input);

    
    float activity_score = 1.0f;

    if (activity_record < ACTIVITY_OPTIMAL_MIN) {
        // Too silent - neuron is not receiving enough signal
        activity_score = activity_record / ACTIVITY_OPTIMAL_MIN;
    } else if (activity_record > ACTIVITY_OPTIMAL_MAX) {
        // Too active - chaotic input, excessive stimulation
        activity_score = 1.0f - ((activity_record - ACTIVITY_OPTIMAL_MAX) / 2.0f);
        activity_score = std::max(activity_score, 0.0f);
    }
    
    float signal_received_score = (received_signals > 0) ? 1.0f : 0.0f;

    // Combine scores: need both signal reception and optimal activity level
    float coherence = (activity_score * 0.7f + signal_received_score * 0.3f);
 
    return std::min(std::max(coherence, 0.0f), 1.0f);
}

 