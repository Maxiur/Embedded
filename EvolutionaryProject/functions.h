#ifndef EMBEDDED_FUNCTIONS_H
#define EMBEDDED_FUNCTIONS_H

#include <memory>
#include <random>
#include <vector>

struct SystemContext {
    int numTasks;
    int numProcessors;
    int numChannels;
    int hardTime;
    // TO DO
};

struct EvolutionParams {
    double alpha;   // Mnożnik populacji
    double beta;    // Prawdopodobieństwo mutacji
    double gamma;   // Prawdopodobieństwo krzyżowania
    double delta;   // Elityzm (klonowanie)

    int epsilon;        // Warunek stopu (ile pokoleń bez poprawy znosimy)
    int maxGenerations; // Twardy limit pokoleń
    int maxTreeDepth;   // Głębokość drzewa
};

// Typy operacji
enum class FunctionType {
    CHANGE_PROCESSOR,
    SWAP_RESOURCES,
    // TO DO
};

struct Phenotype {
    // --- Aktualny Stan (DNA) ---
    std::vector<int> taskToProcessor;

    // zapisac jakos do ktorego kanalu jest podlaczony procesor
    // std::vector<int> edgeToChannel;

    // --- Wyniki ---
    int time = -1;
    int cost = -1;
    // Czy przekracza hard time constrains
    bool isValid = false;

    double fitnessScore = 0.0;
};

// Struktura Wierzcholka
struct Node {
    FunctionType type;
    int targetTaskId;
    int targetProcessorId;

    std::vector<std::unique_ptr<Node>> children;

    Node(const FunctionType t, int taskId = -1, int processorId = -1)
        : type(t), targetTaskId(taskId), targetProcessorId(processorId) {}

    // Klonowanie
    std::unique_ptr<Node> clone() const;

    // Pchanie fenotypu do dzieci
    void process(Phenotype currentState, std::vector<Phenotype>& finalPhenotypeFromLeaves) const;

};

class FitnessEvaluator {
private:
    const SystemContext& context;

    // liczy koszt dla jednego fenotypu
    Phenotype evaluate(const Phenotype& phenotype) const;

public:
    explicit FitnessEvaluator(const SystemContext& context);

    // zwraca najlepszy fenotyp
    Phenotype findBest(const std::vector<Phenotype>& phenotypes) const;
};

class DecisionTree {
private:
    std::unique_ptr<Node> root;

public:
    DecisionTree() = delete;

    explicit DecisionTree(std::unique_ptr<Node> rootNode);
    DecisionTree(const DecisionTree& other);
    DecisionTree& operator=(const DecisionTree& other);

    DecisionTree(DecisionTree&&) noexcept = default;
    DecisionTree& operator=(DecisionTree&&) noexcept = default;

    // przetwarza i wykonuje funkcje wygenerowane wcześniej drzewo w PopulationGenerator
    Phenotype decode(const Phenotype& baseSolution, FitnessEvaluator& judge) const;

};

class PopulationGenerator {
private:
    std::mt19937_64 rng;

    // DFS do budownia galezi
    void expandNode(Node* currentNode, int currentDepth, int maxDepth);

    // generator jednego drzewa
    DecisionTree buildSingleTree(int maxDepth);

public:
    PopulationGenerator();

    // generator Generacji 0
    std::vector<DecisionTree> generatePopulationZero(int populationSize, int maxDepth);

    // generator kolejnej Generacji X
    std::vector<DecisionTree> generateNextPopulation(const std::vector<DecisionTree>& prevPopulation,
        int populationSize, int maxDepth);
};

// Tutaj funkcje do wybierania rodzicow - metoda RANKINGOWA
class ParentSelector {
private:
    std::mt19937_64 rng;

public:
    ParentSelector();
    std::vector<DecisionTree> selectParents(const std::vector<DecisionTree>& population, int numParents);
};

// bierzemy Drzewo A i Drzewo B, losujemy po jednym wezle i odcinami i zamieniamy miejscami(krzyżowanie)
class SubtreeCrossover {
private:
    std::mt19937_64 rng;

public:
    SubtreeCrossover();
    std::pair<DecisionTree, DecisionTree> crossover(const DecisionTree& parentA, const DecisionTree& parentB);

};

// losujemy dany Node i go zmieniamy(mutujemy)
class NodeMutator {
private:
    std::mt19937_64 rng;
    const SystemContext& context;
    double mutationRate;

public:
    explicit NodeMutator(const SystemContext& context, double mutationRate);

    void mutate(DecisionTree& tree);
};

class EvolutionSystem {
private:
    const SystemContext& context;
    const EvolutionParams params;

    FitnessEvaluator evaluator;
    PopulationGenerator generator;
    ParentSelector selector;
    SubtreeCrossover crossover;
    NodeMutator mutator;

public:
    EvolutionSystem(const SystemContext& context, const EvolutionParams& params);

    // uruchamia ewolucje, zwraca wygranego
    Phenotype run(const Phenotype& initialSolution);
};

#endif //EMBEDDED_FUNCTIONS_H
