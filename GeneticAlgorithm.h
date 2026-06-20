#include <functional> // add this
#include <vector>
#include <random>

struct Genome {
    std::vector<float> genes;
    float fitness = 0.0f;
};

class GeneticAlgorithm {
private:
    int populationSize, genomeSize;
    float mutationRate, mutationStrength;
    std::mt19937 rng;
    std::vector<Genome> population;


    std::function<float(const std::vector<float>&)> fitnessFunc;

public:
    GeneticAlgorithm(int populationSize, int genomeSize, float mutationRate, float mutationStrength);

    void initialize();
    void evolve();
    void setFitnessFunction(std::function<float(const std::vector<float>&)> func);
    void evaluate();

    Genome tournamentSelection(int tournamentSize);
    Genome crossover(const Genome& parent1, const Genome& parent2);
    void mutate(Genome& genome);
    void elitism(std::vector<Genome>& newPopulation);
    const Genome& getBestGenome() const;
    std::vector<Genome>& getPopulation();
    
    
    void runUntilOptimal(float targetSum, int maxGenerations = 1000, int printEveryNGen = 1);

    void printPopulation(int gen) const;
};
