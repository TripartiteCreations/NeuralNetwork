#pragma once
#include <math.h>
class utility {
public :
    float clamp(float val, float low, float up) {
        if (val > up) {
            return up;
        }
        if (val < low) {
            return low;
        }
        return val;
    }
    float logistic_sigmoid(float x, float k = 1.0f, float x0 = 0.0f, float L = 1.0f) {
        return L / (1.0f + std::expf(-k * (x - x0)));
	}
    float exponantial_decay(float dt, float tau) {
        return std::expf(-dt / tau);
	}
    float random_int(int min, int max) {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<int> distrib(min, max);
        return distrib(gen);
    }

    float random_float(int min, int max) {
        std::random_device ra;
        std::mt19937 ge(ra());
        std::uniform_real_distribution<> dist(0, 1);
        
        return dist(ge);
    }

};

