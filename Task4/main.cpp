#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>
#include <tuple>
#include <queue>

struct Edge {
    int target;
    int weight;
    int totalSum{};
};

struct Processor {
    int cost;
    int type; // 0 - HC, 1 - PP
};

struct Channel {
    int cost{};
    int bandwidth{};
    std::vector<bool> connected_processor;
};

struct TaskGraph {
    int tasks{};
    int proc{};
    int bus{};

    std::vector<std::vector<Edge>> graph;
    std::vector<Processor> processors;
    std::vector<Channel> channels;
    std::vector<std::vector<int>> times;
    std::vector<std::vector<int>> costs;
};

TaskGraph readGraph(const std::string& file_name) {
    std::ifstream input(file_name);
    TaskGraph graph;

    if (!input) {
        std::cerr << "Nie mozna otworzyc pliku wejsciowego\n";
        return graph;
    }

    std::string trash_string;
    char trash_char;

    // Zadania (@tasks)
    input >> trash_string >> graph.tasks;
    graph.graph.resize(graph.tasks);

    for (int i = 0; i < graph.tasks; ++i) {
        int edges;
        input >> trash_string >> edges;

        for (int e = 0; e < edges; ++e) {
            int target, weight;
            input >> target >> trash_char >> weight >> trash_char;
            graph.graph[i].emplace_back(target, weight);
        }
    }

    // Procesory (@proc)
    input >> trash_string >> graph.proc;
    graph.processors.resize(graph.proc);

    for (int i = 0; i < graph.proc; ++i) {
        int dummy_zero;
        input >> graph.processors[i].cost >> dummy_zero >> graph.processors[i].type;
    }

    // Czasy (@times)
    input >> trash_string;
    graph.times.resize(graph.tasks, std::vector<int>(graph.proc));

    for (int i = 0; i < graph.tasks; ++i) {
        for (int j = 0; j < graph.proc; ++j) {
            input >> graph.times[i][j];
        }
    }

    // Koszty (@cost)
    input >> trash_string;
    graph.costs.resize(graph.tasks, std::vector<int>(graph.proc));

    for (int i = 0; i < graph.tasks; ++i) {
        for (int j = 0; j < graph.proc; ++j) {
            input >> graph.costs[i][j];
        }
    }

    // Kanały (@comm)
    input >> trash_string >> graph.bus;
    graph.channels.resize(graph.bus);

    for (int i = 0; i < graph.bus; ++i) {
        input >> trash_string >> graph.channels[i].cost >> graph.channels[i].bandwidth;
        graph.channels[i].connected_processor.resize(graph.proc);

        for (int j = 0; j < graph.proc; ++j) {
            int temp;
            input >> temp;
            graph.channels[i].connected_processor[j] = temp;
        }
    }

    return graph;
}

int CalculateDP(TaskGraph& graph, int TaskIndex, int PP_ProcessorID, std::vector<int>& dp) {
    if (dp[TaskIndex] != -1) {
        return dp[TaskIndex];
    }

    int max_child_time = 0;

    for (const auto& edge : graph.graph[TaskIndex] ) {
        max_child_time = std::max(max_child_time, CalculateDP(graph, edge.target, PP_ProcessorID, dp));
    }

    dp[TaskIndex] = max_child_time + graph.times[TaskIndex][PP_ProcessorID];
    return dp[TaskIndex];
}

std::tuple<std::vector<double>, double> BFS(TaskGraph& graph, int PP_ProcessorID) {
    std::vector<int> dp(graph.tasks, -1);
    CalculateDP(graph, 0, PP_ProcessorID, dp);

    std::vector<int> in_degree(graph.tasks, 0);
    for (int i = 0; i < graph.tasks; ++i) {
        for (const auto& edge : graph.graph[i]) {
            in_degree[edge.target]++;
        }
    }

    auto compare = [&dp](int left, int right) {
        return dp[left] < dp[right];
    };

    std::priority_queue<int, std::vector<int>, decltype(compare)> heap(compare);
    heap.push(0);

    std::vector<double> task_start_time(graph.tasks, 0.0);
    double makespan = 0.0;

    while (!heap.empty()) {
        int current = heap.top();
        heap.pop();

        // we assume that time won't be -1
        task_start_time[current] = makespan;
        makespan += graph.times[current][PP_ProcessorID];

        for (const auto& edge : graph.graph[current]) {
            in_degree[edge.target]--;

            if (in_degree[edge.target] == 0) {
                heap.push(edge.target);
            }
        }
    }
    return {task_start_time, makespan};
}

int Choose_PP_Index(const TaskGraph& graph) {
    for (int i = 0; i < graph.proc; ++i) {
        if (graph.processors[i].type == 1) {
            bool cant_do = false;
            for (int j = 0; j < graph.tasks; ++j) {
                if (graph.times[j][i] == -1) {
                    cant_do = true;
                    break;
                }
            }
            if (!cant_do)
                return i;
        }
    }
    return -1;
}

int main() {
    TaskGraph graph = readGraph("../input.txt");

    const int Used_PP_Processor = Choose_PP_Index(graph);
    if (Used_PP_Processor == -1) {
        throw std::runtime_error("Nie ma dostepnych procesorow typu PP, ktore moga wykonac wszystkie zadania");
    }

    auto [task_start_time, makespan] = BFS(graph, Used_PP_Processor);

    std::cout << "Najszybszy czas calkowity (Makespan): " << makespan << "\n\n";
    std::cout << "Wykorzystana Architektura:\n";

    std::string prefix = "PP_" + std::to_string(Used_PP_Processor);
    std::cout << prefix << ": ";
    for (int i = 0; i < graph.tasks; ++i) {
        std::cout << "T" << i << "(" << task_start_time[i] << ")";
        if (i != graph.tasks - 1) std::cout << ", ";
    }

    std::cout << "\n";
}