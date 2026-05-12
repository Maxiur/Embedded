#include <algorithm>
#include <iostream>
#include <random>
#include <vector>
#include <utility>
#include <fstream>
#include <string>
#include <tuple>

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
    int type; // 0 HC, 1 PP
    int cost;
};

std::random_device rd;
std::mt19937_64 gen(rd());

std::string gen_single_cond() {
    std::vector<std::string> names = {"czas", "zmienna", "temp", "cisnienie"};
    std::vector<std::string> units = {"s", "", "C", "Pa"};
    std::vector<std::string> signs = {">", "<="};

    std::uniform_int_distribution<> d_type(0, names.size() - 1);
    std::uniform_int_distribution<> d_val(10, 500);
    std::uniform_int_distribution<> d_sign(0, signs.size() - 1);

    int idx = d_type(gen);
    int val = d_val(gen);
    std::string name = names[idx];
    std::string unit = units[idx];
    std::string sign = signs[d_sign(gen)];

    return "[" + name + sign + std::to_string(val) + unit + "]";
}

std::vector<std::vector<std::tuple<int, int, std::string>>> generateGraph() {
    std::uniform_int_distribution<> percent(1, 100);
    std::uniform_int_distribution<> dist_weigh(MIN_WEIGHT, MAX_WEIGHT);

    std::vector<std::vector<std::tuple<int, int, std::string>>> graph(input::tasks);

    for (int i = 0; i < input::tasks; i++) {
        if (i == 0) continue;

        bool has_predecessor = false;
        for (int j = 0; j < i; j++) {
            if (percent(gen) <= 60) {
                has_predecessor = true;
                graph[j].emplace_back(i, input::hasWeights ? dist_weigh(gen) : 0, "");
            }
        }

        if (!has_predecessor) {
            std::uniform_int_distribution dist_forced(0, i - 1);
            int forced_source = dist_forced(gen);
            graph[forced_source].emplace_back(i, input::hasWeights ? dist_weigh(gen) : 0, "");
        }
    }

    // Aplikujemy warunki
    for (int i = 0; i < input::tasks; i++) {
        if (graph[i].size() > 1 && percent(gen) <= 50) {
            std::get<2>(graph[i][0]) = gen_single_cond();
            std::get<2>(graph[i][1]) = gen_single_cond();
        }
    }

    return graph;
}

void printGraph(const std::vector<std::vector<std::tuple<int, int, std::string>>>& graph, std::ostream& out = std::cout) {
    for (size_t i = 0; i < graph.size(); i++) {
        out << i << " " << graph[i].size();

        for (const auto& edge : graph[i]) {
            out << " " << std::get<0>(edge);
            out << "(" << std::get<1>(edge) << ")";
            out << std::get<2>(edge);
        }
        out << std::endl;
    }
}

std::vector<Processor> printProcessors(std::ostream& out) {
    std::uniform_int_distribution<> proc_cost(0, 1000);
    std::vector<Processor> processors;
    for (int i = 0; i < input::hc; i++) processors.push_back({0, proc_cost(gen)});
    for (int i = 0; i < input::pp; i++) processors.push_back({1, proc_cost(gen)});
    std::sort(processors.begin(), processors.end(), [](const Processor& a, const Processor& b) { return a.cost < b.cost; });

    for (const auto& proc : processors) {
        out << proc.cost << " " << 0 << " " << proc.type << std::endl;
    }
    return processors;
}

std::vector<std::vector<int>> printTimes(const std::vector<Processor>& processors, std::ostream& out) {
    std::uniform_int_distribution<> HC_T(HC_MIN_TIME, HC_MAX_TIME);
    std::uniform_int_distribution<> PP_T(PP_MIN_TIME, PP_MAX_TIME);
    std::vector<std::vector<int>> times(input::tasks, std::vector<int>(processors.size()));

    for (int i = 0; i < input::tasks; i++) {
        for (size_t j = 0; j < processors.size(); j++) {
            int t = (processors[j].type == 0) ? HC_T(gen) : PP_T(gen);
            times[i][j] = t;
            out << t << (j == processors.size() - 1 ? "" : " ");
        }
        out << std::endl;
    }
    return times;
}

std::vector<std::vector<int>> printCosts(const std::vector<Processor>& processors, std::ostream& out) {
    std::uniform_int_distribution<> HC_C(HC_MIN_COST, HC_MAX_COST);
    std::uniform_int_distribution<> PP_C(PP_MIN_COST, PP_MAX_COST);
    std::vector<std::vector<int>> costs(input::tasks, std::vector<int>(processors.size()));

    for (int i = 0; i < input::tasks; i++) {
        for (size_t j = 0; j < processors.size(); j++) {
            int c = (processors[j].type == 0) ? HC_C(gen) : PP_C(gen);
            costs[i][j] = c;
            out << c << (j == processors.size() - 1 ? "" : " ");
        }
        out << std::endl;
    }
    return costs;
}

std::vector<std::vector<int>> printBus(std::ostream& out) {
    std::uniform_int_distribution<> b_c(BUS_MIN_COST, BUS_MAX_COST);
    std::uniform_int_distribution<> b_b(BUS_MIN_BANDWIDTH, BUS_MAX_BANDWIDTH);
    std::vector<std::vector<int>> bus(input::bus, std::vector<int>(2 + input::hc + input::pp));

    for (int i = 0; i < input::bus; i++) {
        bus[i][0] = b_c(gen); bus[i][1] = b_b(gen);
        for (int j = 2; j < 2 + input::hc + input::pp; j++) bus[i][j] = (gen() % 2);
        out << "CHAN" << i << " ";
        for (size_t j = 0; j < bus[i].size(); j++) {
            out << bus[i][j] << (j == bus[i].size() - 1 ? "" : " ");
        }
        out << std::endl;
    }
    return bus;
}

void generateOutput(const std::vector<std::vector<std::tuple<int, int, std::string>>>& graph) {
    std::fstream output(input::output + ".txt", std::ios::out);
    if (!output) return;

    output << "@tasks " << input::tasks << "\n";
    printGraph(graph, output);

    output << "\n@proc " << input::hc + input::pp << std::endl;
    auto procs = printProcessors(output);

    output << "\n@times\n";
    printTimes(procs, output);

    output << "\n@cost\n";
    printCosts(procs, output);

    output << "\n@comm " << input::bus << std::endl;
    printBus(output);

    output.close();
}

int main() {
    int weights_input;

    std::cout << "--- GENERATOR GRAFOW WARUNKOWYCH ---\n";

    std::cout << "Podaj liczbe zadan (tasks): ";
    std::cin >> input::tasks;

    std::cout << "Podaj liczbe jednostek dedykowanych (HC): ";
    std::cin >> input::hc;

    std::cout << "Podaj liczbe jednostek uniwersalnych (PP): ";
    std::cin >> input::pp;

    std::cout << "Podaj liczbe kanalow komunikacyjnych (bus): ";
    std::cin >> input::bus;

    std::cout << "Czy krawedzie maja miec wagi? (1 - tak, 0 - nie): ";
    std::cin >> weights_input;
    input::hasWeights = static_cast<bool>(weights_input);

    std::cout << "Podaj nazwe pliku wyjsciowego (bez .txt): ";
    std::cin >> input::output;

    auto adjList = generateGraph();
    generateOutput(adjList);

    return 0;
}
