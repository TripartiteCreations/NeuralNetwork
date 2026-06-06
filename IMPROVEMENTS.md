# Neural Network Improvements - Survival & Accuracy Enhancements

## Overview
This update implements two major improvements to the neural network simulator:
1. **More Accurate Calculations** - Improved STDP and signal processing
2. **Survival Mechanism** - Each neuron is its own organism that thrives in structured signals and dies in chaos

---

## Key Improvements

### 1. Enhanced STDP (Spike-Timing-Dependent Plasticity)

**Previous Implementation:**
- Simple exponential decay: `exp(spike_diff / 100) - 1`
- No asymmetry between LTP (long-term potentiation) and LTD (long-term depression)

**New Implementation:**
- Proper STDP curve with separate learning rates for potentiation and depression
- Uses realistic time constant (tau = 20.0ms)
- Asymmetric learning rates: `A_pos = A_neg = 0.01`
- Formula:
  - If post-synaptic spike follows: `changes = A_pos * exp(-spike_diff / tau)`
  - If pre-synaptic spike follows: `changes = -A_neg * exp(spike_diff / tau)`

**Benefits:**
- More biologically accurate weight adaptation
- Better spike timing relationships
- Improved network learning stability

### 2. Signal Coherence Calculation

New method `calculateSignalCoherence()` measures how structured or chaotic signals are:

```cpp
float total_coherence = (activity_coherence * 0.6f + variance_coherence * 0.4f);
```

- **Activity Coherence (60%)**: Based on neuron's activity record
- **Variance Coherence (40%)**: Based on input signal stability
- Range: 0.0 (chaos) to 1.0 (perfectly structured)

### 3. Neuron Survival Mechanism

Each neuron is now an autonomous organism with:

#### Health System (0.0 to 1.0)
- **Structured Signals** (coherence > 0.4):
  - Health gain: +0.02 per update
  - Chaos accumulation decreases
  - Neuron **thrives**

- **Chaotic Signals** (coherence ? 0.4):
  - Health loss: -0.01 per update
  - Chaos accumulation increases
  - Neuron **gradually dies**

- **Weak Connections**:
  - Neurons with avg weight < 0.1 lose health faster (×0.98)
  - Encourages network pruning of inactive neurons

#### Neuron Properties
- `health` - Current vitality (0.0 = dead, 1.0 = thriving)
- `signal_consistency` - Tracks coherence of received signals
- `chaos_accumulation` - Accumulates noise exposure
- `neuron_id` - Unique identifier for tracking individuals
- `isAlive()` - Boolean query for survival status

### 4. Bug Fixes

**Fixed Signal Processing:**
- Added `i--` after erasing signals to prevent skipping elements
- Prevents index out of bounds errors when multiple signals arrive

**Improved Input Decay:**
- Better decay mechanism for input signals
- Prevents signal bleeding between updates

---

## Usage Example

### Creating a Survival Network

```cpp
std::vector<Neurons> neurons(5);

// Connect neurons
neurons[0].connectTo(&neurons[1], 0.5);
neurons[1].connectTo(&neurons[2], 0.5);
neurons[2].connectTo(&neurons[3], 0.5);

// Send periodic structured signals (helps neurons survive)
if (time % 15 == 0) {
    neurons[0].sendSignal(5, 1);
}

// Update and monitor health
for (auto& neuron : neurons) {
    if (neuron.isAlive()) {
        neuron.update(time);
        std::cout << "Health: " << neuron.getHealth() << std::endl;
    }
}
```

### Monitoring Neural Fitness

```cpp
// Check if neuron survived
if (neuron.isAlive()) {
    // Neuron is healthy
    float health = neuron.getHealth();
}

// Track signal quality
float coherence = calculateSignalCoherence();
```

---

## Evolutionary Behavior

### Thriving Neurons (Structured Environment)
- Receive periodic, organized signals
- Health increases gradually
- Weights stabilize around optimal values
- Network forms stable learned patterns

### Dying Neurons (Chaotic Environment)
- Receive random, noisy signals
- Health decreases over time
- Eventually `isAlive()` returns false
- Can be removed from network for efficiency

### Mixed Networks
- Some neurons die, others thrive
- Network self-prunes based on signal quality
- Emergent specialization of surviving neurons

---

## Constants (Tunable)

```cpp
HEALTH_GAIN_STRUCTURED = 0.02f    // Health gain per update in structured signals
HEALTH_LOSS_CHAOS = 0.01f         // Health loss per update in chaotic signals
COHERENCE_THRESHOLD = 0.4f        // Threshold for signal structure
```

Adjust these to control:
- How quickly neurons adapt to environment
- Sensitivity to signal chaos
- Network survival rates

---

## Integration with Existing Code

All changes are backward compatible:
- Existing `update()`, `connectTo()`, `sendSignal()` work as before
- New methods are additive: `getHealth()`, `isAlive()`, `updateHealth()`
- STDP improvements are internal (better but same interface)

---

## Next Steps

Potential enhancements:
1. **Neuron Reproduction**: Create new neurons from healthy parents
2. **Speciation**: Group neurons by learning patterns
3. **Energy Consumption**: Track metabolic cost of computation
4. **Environmental Adaptation**: Changing signal patterns over time
5. **Population Statistics**: Track average health, survival rates
