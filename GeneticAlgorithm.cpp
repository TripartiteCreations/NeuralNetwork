#include "GeneticAlgorithm.hpp"

#include <algorithm>
#include <cmath>
#include <iostream>

GeneticAlgorithm::GeneticAlgorithm(
    int populationSize,
    int genomeSize,
    float mutationRate,
    float mutationStrength)

    :

    populationSize(populationSize),
    genomeSize(genomeSize),
    mutationRate(mutationRate),
    mutationStrength(mutationStrength)
{
    std::random_device rd;
    rng.seed(rd());
}

void GeneticAlgorithm::initialize()
{
    std::uniform_real_distribution<float> weightDist(-1.0f, 1.0f);

    population.clear();

    for(int i = 0; i < populationSize; i++)
    {
        Genome genome;

        genome.genes.resize(genomeSize);

        for(float& gene : genome.genes)
        {
            gene = weightDist(rng);
            std::cout << "Gene" << i << "  : " << gene << std::endl;
        }

        population.push_back(genome);
    }
}

Genome GeneticAlgorithm::tournamentSelection(int tournamentSize)
{
    std::uniform_int_distribution<int> indexDist(
        0,
        population.size() - 1);

    Genome best =
        population[indexDist(rng)];

    for(int i = 1; i < tournamentSize; i++)
    {
        Genome candidate =
            population[indexDist(rng)];

        if(candidate.fitness > best.fitness)
        {
            best = candidate;
        }
    }

    return best;
}

Genome GeneticAlgorithm::crossover(
    const Genome& parent1,
    const Genome& parent2)
{
    Genome child;

    child.genes.resize(genomeSize);

    std::uniform_real_distribution<float> chance(0.0f, 1.0f);

    for(int i = 0; i < genomeSize; i++)
    {
        if(chance(rng) < 0.5f)
        {
            child.genes[i] = parent1.genes[i];
        }
        else
        {
            child.genes[i] = parent2.genes[i];
        }
    }

    return child;
}

void GeneticAlgorithm::mutate(Genome& genome)
{
    std::uniform_real_distribution<float> chance(0.0f, 1.0f);

    std::normal_distribution<float> mutation(
        0.0f,
        mutationStrength);

    for(float& gene : genome.genes)
    {
        if(chance(rng) < mutationRate)
        {
            gene += mutation(rng);

            gene = std::clamp(
                gene,
                -5.0f,
                5.0f);
        }
    }
}

void GeneticAlgorithm::elitism(
    std::vector<Genome>& newPopulation)
{
    auto best =
        std::max_element(
            population.begin(),
            population.end(),
            [](const Genome& a, const Genome& b)
            {
                return a.fitness < b.fitness;
            });

    newPopulation.push_back(*best);
}

void GeneticAlgorithm::setFitnessFunction(std::function<float(const std::vector<float>&)> func) {
    fitnessFunc = func; // set/replace anytime
}

void GeneticAlgorithm::evaluate() {
    if(!fitnessFunc) {
        std::cerr << "Error: Fitness function not set!" << std::endl;
        return;
    }
    for(Genome& agent : population) {
        agent.fitness = fitnessFunc(agent.genes); // compute per agent
    }
}

void GeneticAlgorithm::evolve() {
    evaluate(); // <-- compute fitness for current population first

    std::vector<Genome> newPopulation;
    elitism(newPopulation);
    while(newPopulation.size() < populationSize) {
        Genome parent1 = tournamentSelection(5);
        Genome parent2 = tournamentSelection(5);
        Genome child = crossover(parent1, parent2);
        mutate(child);
        newPopulation.push_back(child);
    }
    population = std::move(newPopulation);
}

const Genome& GeneticAlgorithm::getBestGenome() const
{
    return *std::max_element(
        population.begin(),
        population.end(),
        [](const Genome& a, const Genome& b)
        {
            return a.fitness < b.fitness;
        });
}

std::vector<Genome>& GeneticAlgorithm::getPopulation()
{
    return population;
}

void GeneticAlgorithm::printPopulation(int gen) const {
    std::cout << "\n=== Generation " << gen << " ===" << std::endl;
    for(int i = 0; i < population.size(); i++) {
        std::cout << "Agent " << i << " | Fitness: " << population[i].fitness << " | Genes: [";
        for(int j = 0; j < genomeSize; j++) {
            std::cout << population[i].genes[j];
            if(j < genomeSize - 1) std::cout << ", ";
        }
        std::cout << "]" << std::endl;
    }
}

void GeneticAlgorithm::runUntilOptimal(float targetSum, int maxGenerations, int printEveryNGen) {
    if(!fitnessFunc) {
        std::cerr << "Error: Set fitness function first!" << std::endl;
        return;
    }

    for(int gen = 0; gen < maxGenerations; gen++) {
        evaluate(); // compute fitness for all agents

        // Print every N generations
        if(gen % printEveryNGen == 0) {
            printPopulation(gen);
            std::cout << "Best fitness: " << getBestGenome().fitness << " / Target: " << targetSum << "\n" << std::endl;
        }

        // Stop if optimal found
        if(getBestGenome().fitness >= targetSum) {
            std::cout << " OPTIMAL FOUND at Gen " << gen << "!" << std::endl;
            printPopulation(gen);
            break;
        }

        evolve(); // create next gen
    }
}