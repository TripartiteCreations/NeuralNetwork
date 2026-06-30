#pragma once
class Signal {
public:
    float signal = 0;
    float sent_T = 0;
    float delay = 0;
    Neurons* sender;
};