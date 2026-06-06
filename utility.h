#pragma once
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
};

