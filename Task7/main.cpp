#include <algorithm>
#include <iostream>
#include <random>
#include <vector>
#include <utility>
#include <fstream>
#include <string>

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

// Funkcja dzieląca wartość całkowitą na losowe, sumujące się części (Stick Breaking)
std::vector<int> splitValue(int total, int parts) {
    std::vector<int> result(parts);
    if (total == -1) {
        std::fill(result.begin(), result.end(), -1);
        return result;
    }

    std::vector<int> cuts;
    std::uniform_int_distribution<> dist(1, total - 1);

    // Losujemy unikalne punkty cięcia
    while (cuts.size() < parts - 1) {
        int cut = dist(gen);
        if (std::find(cuts.begin(), cuts.end(), cut) == cuts.end()) {
            cuts.push_back(cut);
        }
    }

    // Sortujemy miejsca cięcia
    std::sort(cuts.begin(), cuts.end());

    // Liczymy wielkość poszczególnych "kawałków kija"
    int prev = 0;
    for (size_t i = 0; i < cuts.size(); ++i) {
        result[i] = cuts[i] - prev;
        prev = cuts[i];
    }
    result.back() = total - prev; // Ostatni kawałek to reszta do końca

    return result;
}

std::pair<std::vector<std::vector<std::pair<int, int>>>, std::vector<int>> generateGraph() {
    std::uniform_int_distribution<> percent(1, 100);
    std::uniform_int_distribution<> dist_weigh(MIN_WEIGHT, MAX_WEIGHT);
    std::uniform_int_distribution<> subtasks_dist(2, 5); // Od 2 do 5 subtasków

    std::vector<std::vector<std::pair<int, int>>> graph(input::tasks);
    std::vector<int> subtasks(input::tasks, 0);

    for (int i = 0; i < input::tasks; i++) {
        // 30% szans na to, że zadanie będzie rozszerzone
        if (percent(gen) <= 30) {
            subtasks[i] = subtasks_dist(gen);
        }

        if (i == 0) continue;

        bool has_predecessor = false;
        for (int j = 0; j < i; j++) {
            if (percent(gen) <= 60) {
                has_predecessor = true;
                if (input::hasWeights) {
                    graph[j].emplace_back(i, dist_weigh(gen));
                } else {
                    graph[j].emplace_back(i, 0);
                }
            }
        }

        if (!has_predecessor) {
            std::uniform_int_distribution dist_forced(0, i - 1);
            int forced_source = dist_forced(gen);
            if (input::hasWeights) {
                graph[forced_source].emplace_back(i, dist_weigh(gen));
            } else {
                graph[forced_source].emplace_back(i, 0);
            }
        }
    }
    return {graph, subtasks};
}

// ZMIENIONA FUNKCJA - klamerki lądują przy następnikach
void printGraph(const std::vector<std::vector<std::pair<int, int>>>& graph, const std::vector<int>& subtasks, std::ostream& out = std::cout) {
    for (size_t i = 0; i < graph.size(); i++) {
        // Główny task czysty: T<id> <liczba_nastepnikow>
        out << "T" << i << " " << graph[i].size() << " ";

        // Lecimy po następnikach
        for (const auto& [target, weight] : graph[i]) {
            out << target;
            // Jeśli cel (następnik) jest rozszerzony, walimy klamerkę z ilością subtasków
            if (subtasks[target] >= 2) {
                out << "{" << subtasks[target] << "}";
            }
            out << "(" << weight << ") ";
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

std::vector<std::vector<int>> printTimes(const std::vector<Processor>& processors, const std::vector<int>& subtasks, std::ostream& out = std::cout) {
    std::uniform_int_distribution<> HC_COST(HC_MIN_TIME, HC_MAX_TIME);
    std::uniform_int_distribution<> PP_COST(PP_MIN_TIME, PP_MAX_TIME);

    std::vector<std::vector<int>> times(input::tasks, std::vector<int>(input::hc + input::pp));

    for (int i = 0; i < input::tasks; i++) {
        for (int j = 0; j < input::hc + input::pp; j++) {
            int total_time = 0;
            if (processors[j].type == 0) {
                int cost = HC_COST(gen);
                total_time = (HC_MAX_TIME - cost <= TIME_LIMIT) ? -1 : cost;
            } else {
                int cost = PP_COST(gen);
                total_time = (PP_MAX_TIME - cost <= TIME_LIMIT) ? -1 : cost;
            }

            times[i][j] = total_time;

            if (subtasks[i] >= 2) {
                auto split = splitValue(total_time, subtasks[i]);
                out << "{";
                for (size_t k = 0; k < split.size(); ++k) {
                    out << split[k] << (k < split.size() - 1 ? "," : "");
                }
                out << "} ";
            } else {
                out << total_time << " ";
            }
        }
        out << std::endl;
    }

    return times;
}

std::vector<std::vector<int>> printCosts(const std::vector<Processor>& processors, const std::vector<int>& subtasks, std::ostream& out = std::cout) {
    std::uniform_int_distribution<> HC_COST(HC_MIN_COST, HC_MAX_COST);
    std::uniform_int_distribution<> PP_COST(PP_MIN_COST, PP_MAX_COST);
    std::vector<std::vector<int>> costs(input::tasks, std::vector<int>(input::hc + input::pp));

    for (int i = 0; i < input::tasks; i++) {
        for (int j = 0; j < input::hc + input::pp; j++) {
            int total_cost = (processors[j].type == 0) ? HC_COST(gen) : PP_COST(gen);
            costs[i][j] = total_cost;

            if (subtasks[i] >= 2) {
                auto split = splitValue(total_cost, subtasks[i]);
                out << "{";
                for (size_t k = 0; k < split.size(); ++k) {
                    out << split[k] << (k < split.size() - 1 ? "," : "");
                }
                out << "} ";
            } else {
                out << total_cost << " ";
            }
        }
        out << std::endl;
    }

    return costs;
}

std::vector<std::vector<int>> printBus(std::ostream& out = std::cout) {
    std::uniform_int_distribution<> bus_cost(BUS_MIN_COST, BUS_MAX_COST);
    std::uniform_int_distribution<> bus_bandiwidth(BUS_MIN_BANDWIDTH, BUS_MAX_BANDWIDTH);
    std::uniform_int_distribution<> isReachable(0, 1);

    std::vector<std::vector<int>> bus(input::bus, std::vector<int>(2 + input::hc + input::pp));

    for (int i = 0; i < input::bus; i++) {
        bus[i][0] = bus_cost(gen);
        bus[i][1] = bus_bandiwidth(gen);
    }

    for (int i = 0; i < input::bus; i++) {
        for (int j = 2; j < 2 + input::hc + input::pp; j++) {
            bus[i][j] = isReachable(gen);
        }
    }

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

    for (int i = 0; i < input::bus; i++) {
        out << "CHAN" << i << " ";
        for (int j = 0; j < 2 + input::hc + input::pp; j++) {
            out << bus[i][j] << " ";
        }
        out << std::endl;
    }
    return bus;
}

void generateOutput(const std::vector<std::vector<std::pair<int, int>>>& graph, const std::vector<int>& subtasks) {
    std::fstream output;
    output.open("../" + input::output + ".txt", std::ios::out);
    if (!output) {
        std::cerr << "Nie mozna otworzyc pliku wyjsciowego\n";
        return;
    }

    output << "@tasks " << input::tasks << std::endl;
    printGraph(graph, subtasks, output);

    output << "@proc " << input::hc + input::pp << std::endl;
    std::vector<Processor> processors = printProcessors(output);

    output << "@times" << std::endl;
    printTimes(processors, subtasks, output);

    output << "@cost" << std::endl;
    printCosts(processors, subtasks, output);

    output << "@comm " << input::bus << std::endl;
    printBus(output);
    output << std::endl;

    output.close();
    std::cout << "Mamy to. Zapisano graf do: ../" << input::output << ".txt\n";
}

int main() {
    int weights_input;

    std::cout << "--- GENERATOR GRAFOW ZADAN ---\n";

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

    std::cout << "Podaj nazwe pliku wyjsciowego (np. graf1): ";
    std::cin >> input::output;

    std::cout << "\nGenerowanie w toku...\n";

    auto [adjList, subtasks] = generateGraph();
    generateOutput(adjList, subtasks);

    return 0;
}
