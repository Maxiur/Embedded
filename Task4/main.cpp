#include <iostream>
#include <fstream>
#include <vector>
#include <queue>
#include <string>
#include <tuple>
#include <algorithm>

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

std::tuple<std::vector<double>, std::vector<double>, double> BFS(const TaskGraph& graph, const std::vector<int>& TaskTimes, int PP_ProcessorID) {
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

    std::vector task_start_time(graph.tasks, 0.0);
    std::vector earliest_start(graph.tasks, 0.0);
    std::vector task_finish_time(graph.tasks, 0.0);
    double makespan = 0.0;

    while (!queue.empty()) {
        int current = queue.front();
        queue.pop();

        task_start_time[current] = earliest_start[current];
        task_finish_time[current] = earliest_start[current] + TaskTimes[current];
        makespan = std::max(makespan, task_finish_time[current]);

        for (const auto& edge : graph.graph[current]) {

            earliest_start[edge.target] = std::max(earliest_start[edge.target], task_finish_time[current]);

            in_degree[edge.target]--;
            if (in_degree[edge.target] == 0) {
                queue.push(edge.target);
            }
        }
    }
    return { task_start_time, task_finish_time, makespan };
}
