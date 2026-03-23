#include <iostream>
#include <fstream>
#include <vector>
#include <queue>
#include <climits>

struct Edge {
    int target;
    int weight;
};

struct Processor {
    int cost;
    int type; // 0 - HC, 1 - PP
};

struct Channel {
    int cost{};
    int bandwidth{};
    std::vector<int> connected_processor;
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
            input >> graph.channels[i].connected_processor[j];
        }
    }

    return graph;
}

std::vector<int> BFS(TaskGraph& graph, std::vector<int>& TaskTimes) {
    std::vector in_degree(graph.tasks, 0);

    for (int i = 0; i < graph.tasks; ++i) {
        for (const auto& edge : graph.graph[i]) {
            in_degree[edge.target]++;
        }
    }

    std::queue<int> queue;
    for (int i = 0; i < graph.tasks; ++i) {
        if (in_degree[i] == 0) {
            queue.push(i);
        }
    }

    std::vector<int> topological_sort;
    std::vector times(graph.tasks, 0);
    std::vector task_start_time(graph.tasks, 0);
    int time = 0;

    while (!queue.empty()) {
        int current = queue.front();
        queue.pop();
        topological_sort.push_back(current);
        times[current] += TaskTimes[current];
        task_start_time[current] = time;

        for (const auto& edge : graph.graph[current]) {
            in_degree[edge.target]--;
            if (in_degree[edge.target] == 0) {
                queue.push(edge.target);
            }
        }
    }
    return topological_sort;
}
int LowestTime(TaskGraph& graph) {
    std::vector<int> TaskProcessors(graph.tasks);
    std::vector<int> TaskTimes(graph.tasks);

    for (int i = 0; i < graph.tasks; i++) {
        int min_time = INT_MAX;
        int min_proc = -1;
        for (int j = 0; j < graph.proc; j++) {
            if (graph.times[i][j] != -1 && min_time > graph.times[i][j]) {
                min_time = graph.times[i][j];
                min_proc = j;
            }
        }
        TaskProcessors[i] = min_proc;
        TaskTimes[i] = min_time;
    }
    std::vector<int> topological_sort = BFS(graph, TaskTimes);

    return 0;
}


int main() {
    std::string input= "../input.txt";
    TaskGraph system = readGraph(input);

    int time = LowestTime(system);

    return 0;
}