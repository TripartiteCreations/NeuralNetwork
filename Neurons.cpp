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


    std::uniform_real_distribution<> distrib(0.01, 1);

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



void Neurons::update(float t, float dt) {
    this->dt = dt;
    simulation_time = t;
    update_count++;

    // Decay existing activity and input
    this->activity_record *= util.exponantial_decay(dt, a_r_d);
    this->excitability += util.exponantial_decay(dt, ex_decay) * (1.0f - excitability);
    this->neuron_availability *= util.exponantial_decay(dt, n_availability_decay);
    this->reward *= util.exponantial_decay(dt, reward_decay);
    this->trace_firing *= util.exponantial_decay(dt, trace_decay); // Decay firing trace
    this->chaos_decay *= util.exponantial_decay(dt, chaos_decay);
    input *= util.exponantial_decay(dt, decay);


    float s = 0;


    float dif = input;
   
    SpikeHandler(simulation_time);
    applySTDP();
    propagate();
	dif = input - dif;
    // Calculate signal coherence before propagating
    calculateSignalCoherence();
    RewardHandler(reward, activity_record);
    /// F_T = (1.0 - target_activity) + 1.0f * (0.5 - (1-excitability) - (1 - activity_record));
    F_T = (target_activity)+1 * (0.5f - (1 - excitability) - (1 - (util.logistic_sigmoid(activity_record, (10), target_activity)))) + ((activity_record - target_activity) + std::abs(activity_record - target_activity)) / 2;
    chaos_accumulation += util.logistic_sigmoid(activity_record, -((activity_record - CHAOS_THRESHOLD) * CHAOS_SENSITIVITY), (CHAOS_THRESHOLD), chaos_scale);
    chaos_accumulation = util.clamp(chaos_accumulation, -1.0f, 1.0f);
    excitability += chaos_accumulation;
    excitability = util.clamp(excitability, 0.1f, 2.0f);
    F_T = util.clamp(F_T, 0.1f, 1.0f); // Clamp F_T to a reasonable range 
    if ((dif == 0)) return;
   //	std::cout << "Neuron " << neuron_id << " Neuron input :  " << this->input << " Firing Threshold : " << F_T << " activity : " << activity_record << " Neuron Availability : " << neuron_availability << " excitability : " << excitability << " chaos : " << chaos_accumulation << std::endl;
    

}



void Neurons::propagate() {
    int C_S = connect.size();

	
    for (int i = 0; i < C_S; ++i) {
        Connection* c = connect[i].get();


        //c->to->input *= decay;
        c->weight = util.clamp(c->weight, -1, 1);
        c->weight *= util.exponantial_decay(dt, weight_decay);
        // std::cout << "c : " << c->delay << std::endl;
        if (input != 0) {
            c->to->sendSignal((input * c->weight) * excitability, c->delay);
        }


    }

}



void Neurons::STDP(Connection* c) {
    Neurons* from = c->from;
    Neurons* to = c->to;
    float f_signal_T = from->trace_firing;
    float t_signal_T = to->trace_firing;




    float changes = 0;
    if (t_signal_T < 1e-2f) {
        to->trace_firing = 0;
        t_signal_T = 0;
    }

    if (f_signal_T < 1e-2f) {
        from->trace_firing = 0;
        f_signal_T = 0;
    }

    float d = f_signal_T - t_signal_T;
    float balance = -99;
    float coincidence = expf(-50.0f * d * d);

    changes =
        ((f_signal_T * t_signal_T) *
            (tanhf(balance * d) * expf(-(d * d)) + coincidence)) * learning_rate;

    if (changes != 0) {
        // std::cout << "Neuron " << from->neuron_id << " -> " << to->neuron_id 
           //        << " weight_change: " << changes << " pre synaptic: " << f_signal_T << " post synaptic: " << t_signal_T << std::endl;
    }
    c->delay_scale += changes * 0.1f;

    c->weight += changes;


}

void Neurons::applySTDP() {
    for (auto& c : connect) {
        STDP(c.get());
    }
}

void Neurons::SpikeHandler(float t) {
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
        
        fire = firing_threshold(active_signal_sum, this);
     //   std::cout << "Active Signal Sum: " << active_signal_sum  << "fire : " << fire << std::endl;
    }


    if (fire && neuron_availability < 1e-4f) {

        this->current_RT = t;
      // std::cout << "Neuron " << neuron_id << " fired at time: " << t << " with input: " << active_signal_sum << std::endl;
        trace_firing += 1.f; // Update firing trace
        neuron_availability = neuron_availability_timer; // Reset availability after firing
        this->input += spike;


        for (int i = signals_to_remove.size() - 1; i >= 0; --i) {
            signals.erase(signals.begin() + signals_to_remove[i]);
        }
    }
    else {
        for (int i = signals_to_remove.size() - 1; i >= 0; --i) {
            signals.at(i)->signal *= signal_decay;
            if (signals.at(i)->signal == 0) {
                signals.erase(signals.begin() + signals_to_remove[i]);
            }
        }

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

    excitability += util.logistic_sigmoid(coherence, -(coherence - target_activity) * r_s, target_activity, reward * r_r);

}

float Neurons::calculateSignalCoherence() {


    // Track how many signals this neuron received
    int received_signals = signals.size();

    // Calculate activity level from input magnitude
    activity_record += std::abs(input) * a_r;


    float activity_score = 1.0f;

    if (activity_record < ACTIVITY_OPTIMAL_MIN) {
        // Too silent - neuron is not receiving enough signal
        activity_score = activity_record / ACTIVITY_OPTIMAL_MIN;
    }
    else if (activity_record > ACTIVITY_OPTIMAL_MAX) {
        // Too active - chaotic input, excessive stimulation
        activity_score = 1.0f - ((activity_record - ACTIVITY_OPTIMAL_MAX) / 2.0f);
        activity_score = std::max(activity_score, 0.0f);
    }

    float signal_received_score = (received_signals > 0) ? 1.0f : 0.0f;

    // Combine scores: need both signal reception and optimal activity level
    float coherence = (activity_score * 0.7f + signal_received_score * 0.3f);

    return std::min(std::max(coherence, 0.0f), 1.0f);
}

void Neurons::connectionHandler(std::vector<Neurons*> n) {
    int neuron = util.random_int(0, neuron_counter);
    Neurons* ns = n[neuron_counter];
    if (this->connect.empty()) {
        

        float weight = util.random_float(-1, 1);
        
        float probability = util.random_float(0, 1) + util.logistic_sigmoid(ns->activity_record, -(ns->activity_record - ns->target_activity) * 99, ns->target_activity, ns->activity_record * 2);
        if (this->neuron_id == neuron_counter) return;
        if (probability > this->NoConnectionProbability) return;
        this->connectTo(n[neuron], weight);
        return;
    }

    for (auto& c : connect) {
        float weight = util.random_float(-1, 1);
        
        this->connectTo(c->to, weight);
    }
}

bool Neurons::isSpiked() {
    return firing_threshold(this->input, this);
}