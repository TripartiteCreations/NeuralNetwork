#pragma once
class Neurons;
class Connection {
public:
    Neurons* to;
    Neurons* from;
    float weight = 0.5;
    float delay = 0;
    float delay_scale = 0.01f;

};

