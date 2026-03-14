#include <iostream>
#include <random>
#include <vector>
#include <utility>

std::vector<std::vector<std::pair<int, int>>> generateGraph(int tasks) {
    std::random_device rd;
    std::mt19937_64 gen(rd());
    std::uniform_int_distribution<> dist(1, 100);
    std::uniform_int_distribution<> dist_weigh(1, 100);
    std::vector<std::vector<std::pair<int, int>>> graph(tasks);

    for (int i = 1; i < tasks; i++) {
        for (int j = 0; j < i; j++) {
            // 60%
            if (dist(gen) <= 60) {
                int weight = dist_weigh(gen);
                graph[i].push_back({j, weight});
            }
        }
        if (graph[i].empty()) {
            std::uniform_int_distribution dist_forced(0, i - 1);
            int forced_parent = dist_forced(gen);
            int weight = dist_weigh(gen);
            graph[i].push_back({forced_parent, weight});
        }
    }
    return graph;
}

bool is_cyclic(int u, std::vector<std::vector<int>>& graph, std::vector<int>& colors) {
    // 0 - not visited, 1 visiting, 2 visited
    if (colors[u] == 1) return true;
    if (colors[u] == 2) return false;

    colors[u] = 1; // visiting

    for (int v : graph[u]) {
        if (is_cyclic(v, graph, colors)) return true;
    }

    colors[u] = 2; // visited
    return false;
}

void printGraph(std::vector<std::vector<int>>& graph) {
    int i = 0;
    for (auto row : graph) {
        std::cout << i++ << " ";
        for (auto col : row) {
            std::cout << col << " ";
        }
        std::cout << std::endl;
    }
}

int main(int argc, char* argv[]) {
    if (argc < 6) {
        std::cerr << "Brakuje danych! Nalezy wpisać - Liczbe zadań, jednostek dedykowanych, uniwersalnych, liczbę lin komunikacyjnych oraz nazwe pliku wyjsciowego\n";
        return 1;
    }

    int tasks = std::stoi(argv[1]);
    int uni = std::stoi(argv[2]);
    int dedi = std::stoi(argv[3]);
    int lines = std::stoi(argv[4]);
    std::string output = argv[5];

    std::cout << tasks << " " << uni << " " << dedi << " " << lines << " " << output << std::endl;

    // std::vector<std::vector<int>> adjList = generateGraph(tasks);
    // printGraph(adjList);
    
for (int i = 0; i < 10000; i++) {
    auto graph = generateGraph(tasks);

    std::vector<int> colors(tasks, 0);

    bool cycle_found = false;
    for (int v = 0; v < tasks; ++v) {
        if (colors[v] == 0 && is_cyclic(v, graph, colors)) {
            cycle_found = true;
            break;
        }
    }

    if (cycle_found) {
        std::cout << "O kurcze, wylosowalo cykl!\n";
        printGraph(graph); // Zakładam, że taką funkcję już masz
    }
}


    return 0;
}
