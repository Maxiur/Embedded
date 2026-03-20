#include <algorithm>
#include <iostream>
#include <random>
#include <vector>
#include <utility>
#include <fstream>

constexpr int MIN_WEIGHT = 1;
constexpr int MAX_WEIGHT = 100;

constexpr int HC_MIN_TIME = 100;
constexpr int HC_MAX_TIME = 200;
constexpr int PP_MIN_TIME = 250;
constexpr int PP_MAX_TIME = 400;
constexpr int TIME_LIMIT = 8;

constexpr int HC_MIN_COST = 250;
constexpr int HC_MAX_COST = 400;
constexpr int PP_MIN_COST = 100;
constexpr int PP_MAX_COST = 200;

constexpr int BUS_MIN_COST = 20;
constexpr int BUS_MAX_COST = 35;
constexpr int BUS_MIN_BANDWIDTH = 10;
constexpr int BUS_MAX_BANDWIDTH = 15;

namespace input {
    int tasks;
    int hc;
    int pp;
    int bus;
    std::string output;
    bool hasWeights = true;
}

struct Processor {
    // 0 HC, 1 PP
    int type;
    int cost;
};

// source kernel
std::random_device rd;
std::mt19937_64 gen(rd());

std::vector<std::vector<std::pair<int, int>>> generateGraph() {
    std::uniform_int_distribution<> percent(1, 100);
    std::uniform_int_distribution<> dist_weigh(MIN_WEIGHT, MAX_WEIGHT);
    std::vector<std::vector<std::pair<int, int>>> graph(input::tasks);

    for (int i = 1; i < input::tasks; i++) {
        bool has_predecessor = false;
        for (int j = 0; j < i; j++) {
            // 60%
            if (percent(gen) <= 60) {
                has_predecessor = true;
                if (input::hasWeights) {
                    int weight = dist_weigh(gen);
                    graph[j].emplace_back(i, weight);
                }
                else {
                    graph[j].emplace_back(i, 0);
                }
            }
        }
        if (!has_predecessor) {
            std::uniform_int_distribution dist_forced(0, i - 1);
            int forced_source = dist_forced(gen);
            if (input::hasWeights) {
                int weight = dist_weigh(gen);
                graph[forced_source].emplace_back(i, weight);
            }
            else {
                graph[forced_source].emplace_back(i, 0);
            }
        }
    }
    return graph;
}

void printGraph(std::vector<std::vector<std::pair<int, int>>>& graph, std::ostream& out = std::cout) {
    int i = 0;
    for (const auto& row : graph) {
        out <<"T"<< i++ << " " << row.size() << " ";
        for (const auto& [target, weight] : row) {
            out << target << "(" << weight << ") ";
        }
        out << std::endl;
    }
}

std::vector<Processor> printProcessors(std::ostream& out = std::cout) {
    std::uniform_int_distribution<> proc_cost(0, 1000);
    std::vector<Processor> processors;

    for (int i = 0; i < input::hc; i++) {
        processors.push_back({0, proc_cost(gen)});
    }

    for (int i = 0; i < input::pp; i++) {
        processors.push_back({1, proc_cost(gen)});
    }

    std::sort(processors.begin(), processors.end(), [](const Processor& a, const Processor& b) {
        return a.cost < b.cost;
    });

    for (const auto& proc : processors) {
        out << proc.cost << " " << 0 << " " << proc.type << std::endl;
    }
    return processors;
}

std::vector<std::vector<int>> printTimes(const std::vector<Processor>& processors, std::ostream& out = std::cout) {
    std::uniform_int_distribution<> HC_COST(HC_MIN_TIME, HC_MAX_TIME);
    std::uniform_int_distribution<> PP_COST(PP_MIN_TIME, PP_MAX_TIME);

    std::vector times(input::tasks, std::vector<int>(input::hc + input::pp));
    for (int i = 0; i < input::tasks; i++) {
        for (int j = 0; j < input::hc + input::pp; j++) {
            if (processors[j].type == 0) {
                int cost = HC_COST(gen);
                if (HC_MAX_TIME - cost <= TIME_LIMIT)
                    times[i][j] = -1;
                else
                    times[i][j] = HC_COST(gen);
            }
            else {
                int cost = PP_COST(gen);
                if (PP_MAX_TIME - cost <= TIME_LIMIT)
                    times[i][j] = -1;
                else
                    times[i][j] = PP_COST(gen);
            }
            out << times[i][j] << " ";
        }
        out << std::endl;
    }

    return times;
}

std::vector<std::vector<int>> printCosts(const std::vector<Processor>& processors, std::ostream& out = std::cout) {
    std::uniform_int_distribution<> HC_COST(HC_MIN_COST, HC_MAX_COST);
    std::uniform_int_distribution<> PP_COST(PP_MIN_COST, PP_MAX_COST);
    std::vector costs(input::tasks, std::vector<int>(input::hc + input::pp));

    for (int i = 0; i < input::tasks; i++) {
        for (int j = 0; j < input::hc + input::pp; j++) {
            if (processors[j].type == 0) {
                costs[i][j] = HC_COST(gen);
            }
            else {
                costs[i][j] = PP_COST(gen);
            }
            out << costs[i][j] << " ";
        }
        out << std::endl;
    }

    return costs;
}

std::vector<std::vector<int>> printBus(std::ostream& out = std::cout) {
    std::uniform_int_distribution<> bus_cost(BUS_MIN_COST, BUS_MAX_COST);
    std::uniform_int_distribution<> bus_bandiwidth(BUS_MIN_BANDWIDTH, BUS_MAX_BANDWIDTH);
    std::uniform_int_distribution<> isReachable(0, 1);

    std::vector bus(input::bus, std::vector<int>(2 + input::hc + input::pp));

    for (int i = 0; i < input::bus; i++) {
        bus[i][0] = bus_cost(gen);
        bus[i][1] = bus_bandiwidth(gen);
    }

    for (int i = 0; i < input::bus; i++) {
        for (int j = 2; j < 2 + input::hc + input::pp; j++) {
            bus[i][j] = isReachable(gen);
        }
    }

    // checking if any processor isnt connected
    for (int j = 2; j < input::hc + input::pp + 2; j++) {
        bool is_connected = false;
        for (int i = 0; i < input::bus; i++) {
            if (bus[i][j] == 1) {
                is_connected = true;
                break;
            }
        }
        if (!is_connected) {
            std::uniform_int_distribution<> randomRow(0, input::bus - 1);
            bus[randomRow(gen)][j] = 1;
        }
    }

    // printing
    for (int i = 0; i < input::bus; i++) {
        out << "CHAN" << i << " ";
        for (int j = 0; j < 2 + input::hc + input::pp; j++) {
            out << bus[i][j] << " ";
        }
        out << std::endl;
    }
    return bus;
}

void generateOutput(std::vector<std::vector<std::pair<int, int>>>& graph) {
    std::fstream output;
    output.open("../" + input::output + ".txt", std::ios::out);
    if (!output) {
        std::cerr << "Nie mozna otworzyc pliku wyjsciowego\n";
        return;
    }
    output << "@tasks " << input::tasks << std::endl ;
    printGraph(graph, output);

    output << "@proc " << input::hc + input::pp << std::endl;
    std::vector<Processor> processors = printProcessors(output);

    output << "@times" << std::endl;
    printTimes(processors, output);

    output << "@cost" << std::endl;
    printCosts(processors, output);

    output << "@comm " << input::bus << std::endl;
    printBus(output);
    output << std::endl;

    output.close();
}

int main(int argc, char* argv[]) {
    if (argc < 7) {
        std::cerr << "Brakuje danych! Nalezy wpisać - Liczbe zadań, jednostek dedykowanych, uniwersalnych, liczbę lin komunikacyjnych oraz nazwe pliku wyjsciowego\n";
        return 1;
    }

    input::tasks = std::stoi(argv[1]);
    input::hc = std::stoi(argv[2]);
    input::pp = std::stoi(argv[3]);
    input::bus = std::stoi(argv[4]);
    input::hasWeights = static_cast<bool>(std::stoi(argv[5]));
    input::output = argv[6];

    auto adjList = generateGraph();

    generateOutput(adjList);

    return 0;
}
