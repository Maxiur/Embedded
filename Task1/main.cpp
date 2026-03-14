#include <iostream>
#include <random>
#include <vector>

std::vector<std::vector<int>> generateGraph(int tasks, int uni, int dedi, int lines) {
    std::random_device rd;
    std::mt19937_64 gen(rd());
    std::uniform_int_distribution<> dist(0, 1);
    std::vector<std::vector<int>> graph(tasks);

    for (int i = 0; i < tasks; i++) {
        for (int j = 0; j < i; j++) {
            if (dist(gen) == 1) {
                graph[i].push_back(j);
            }
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

    std::vector<std::vector<int>> adjList = generateGraph(tasks);


    return 0;
}