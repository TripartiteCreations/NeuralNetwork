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

};

