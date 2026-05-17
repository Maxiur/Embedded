#include "functions.h"

// --- NODE ---
std::unique_ptr<Node> Node::clone() const {
    auto clonedNode = std::make_unique<Node>(type, targetTaskId, targetProcessorId);

    for (const auto& child : children) {
        clonedNode->children.push_back(child->clone());
    }

    return clonedNode;
}

void Node::process(Phenotype currentState, std::vector<Phenotype>& finalPhenotypeFromLeaves) const {
    switch (this->type) {
        case FunctionType::CHANGE_PROCESSOR:
            // logika
            break;
        case FunctionType::SWAP_RESOURCES:
            // ...
            break;
    }

    // dodajemy kiedy jest lisciem
    if (children.empty()) {
        finalPhenotypeFromLeaves.push_back(currentState);
    }
    else {
        // przepychanie do dolu fenotypu
        for (const auto& child : children) {
            child->process(currentState, finalPhenotypeFromLeaves);
        }
    }
}

// --- DECISION TREE ---

// constructor with rootNode
DecisionTree::DecisionTree(std::unique_ptr<Node> rootNode) : root(std::move(rootNode)) {}

// Copy constructor
DecisionTree::DecisionTree(const DecisionTree& other) {
    if (other.root) {
        root = other.root->clone();
    }
}

DecisionTree& DecisionTree::operator=(const DecisionTree& other) {
    if (this != &other && other.root) {
        root = other.root->clone();
    }

    return *this;
}

Phenotype DecisionTree::decode(const Phenotype& baseSolution, FitnessEvaluator& judge) const {
    // worek na liscie
    std::vector<Phenotype> leavesBag;

    if (root) {
        root->process(baseSolution, leavesBag);
    }

    // ochrona przed pustym workiem --> wtedy najlepsze rozwiazanie to baseSolution
    if (leavesBag.empty()) leavesBag.push_back(baseSolution);

    return judge.findBest(leavesBag);
}