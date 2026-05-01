#include <iostream>
#include <fstream>
#include <vector>
#include <queue>
#include <climits>
#include <algorithm>
#include <utility>
#include <string>
#include <tuple>

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
            graph.graph[i].emplace_back(Edge{target, weight});
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

std::pair<std::vector<int>, std::vector<int>> LowestTime(const TaskGraph& graph) {
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

    return { TaskProcessors, TaskTimes };
}

std::tuple<std::vector<int>, std::vector<int>, int> BFS(const TaskGraph& graph, const std::vector<int>& TaskTimes, const std::vector<int>& TaskProcessors) {
    std::vector<int> in_degree(graph.tasks, 0);

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

    std::vector<int> task_start_time(graph.tasks, 0);
    std::vector<int> earliest_start(graph.tasks, 0);
    std::vector<int> task_finish_time(graph.tasks, 0);
    int makespan = 0;

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

int CalculateCost(const TaskGraph& graph, const std::vector<int>& TaskProcessors) {
    int total_cost = 0;
    std::vector<int> pe_usage(graph.proc, 0);

    for (int i = 0; i < graph.tasks; ++i) {
        int p = TaskProcessors[i];
        pe_usage[p]++;
        total_cost += graph.costs[i][p];
    }

    for (int p = 0; p < graph.proc; ++p) {
        if (pe_usage[p] > 0 && graph.processors[p].type == 1) {
            total_cost += graph.processors[p].cost;
        }
    }

    for (int c = 0; c < graph.bus; ++c) {
        for (int p = 0; p < graph.proc; ++p) {
            if (pe_usage[p] > 0 && graph.channels[c].connected_processor[p] == 1) {
                if (graph.processors[p].type == 1) {
                    total_cost += graph.channels[c].cost;
                }
                else {
                    total_cost += graph.channels[c].cost * pe_usage[p];
                }
            }
        }
    }

    return total_cost;
}

int main() {
    std::string input;
    int deadline;

    std::cout << "Podaj nazwe pliku wejsciowego (razem z .txt): ";
    std::cin >> input;

    std::cout << "Podaj deadline (maksymalny czas wykonania): ";
    std::cin >> deadline;
    std::cout << "\n";

    TaskGraph graph = readGraph(input);
    if (graph.tasks == 0) return 1;

    auto [TaskProcessors, TaskTimes] = LowestTime(graph);
    auto [task_start_time, task_finish_time, makespan] = BFS(graph, TaskTimes, TaskProcessors);

    std::cout << "--- START (Najszybsza konfiguracja) ---\n";
    std::cout << "Makespan: " << makespan << "\n";
    std::cout << "Koszt poczatkowy: " << CalculateCost(graph, TaskProcessors) << "\n";

    std::vector<bool> can_be_slowed(graph.tasks, true);

    while (true) {
        int target_task = -1;
        int max_cost = -1;

        // Szukamy najdroższej opcji, która jeszcze nie została zablokowana
        for (int i = 0; i < graph.tasks; ++i) {
            if (!can_be_slowed[i]) continue;
            int p = TaskProcessors[i];
            if (graph.costs[i][p] > max_cost) {
                max_cost = graph.costs[i][p];
                target_task = i;
            }
        }

        // Jak już nie ma co zwalniać, to koniec
        if (target_task == -1) {
            break;
        }

        int current_p = TaskProcessors[target_task];
        int current_time = TaskTimes[target_task];

        // Szukamy najszybszej z wolniejszych opcji
        int best_slower_p = -1;
        int best_slower_time = INT_MAX;

        for (int p = 0; p < graph.proc; ++p) {
            if (graph.times[target_task][p] > current_time && graph.times[target_task][p] < best_slower_time) {
                best_slower_time = graph.times[target_task][p];
                best_slower_p = p;
            }
        }

        if (best_slower_p == -1) {
            can_be_slowed[target_task] = false;
            continue;
        }

        // Testowa podmianka
        TaskProcessors[target_task] = best_slower_p;
        TaskTimes[target_task] = best_slower_time;

        auto [new_starts, new_finishes, new_makespan] = BFS(graph, TaskTimes, TaskProcessors);

        if (new_makespan > deadline) {
            // Przestrzeliliśmy deadline. Cofamy i sprawdzamy inne taski.
            TaskProcessors[target_task] = current_p;
            TaskTimes[target_task] = current_time;
            can_be_slowed[target_task] = false;
            continue;
        }

        // Zmiana zaakceptowana, nadpisujemy globalny stan
        task_start_time = new_starts;
        task_finish_time = new_finishes;
        makespan = new_makespan;
    }

    // Faza 3: Drukujemy co z tego wyszło
    std::cout << "\n--- WYNIK KONCOWY ---\n";
    std::cout << "Czas calkowity (Makespan): " << makespan << " (Limit: " << deadline << ")\n";
    std::cout << "Koszt ostateczny: " << CalculateCost(graph, TaskProcessors) << "\n\n";
    std::cout << "Wykorzystana Architektura:\n";

    int hc_counter = 0;
    int pp_counter = 0;

    for (int p = 0; p < graph.proc; ++p) {
        std::string prefix;
        if (graph.processors[p].type == 0) {
            prefix = "HC" + std::to_string(hc_counter++);
        }
        else {
            prefix = "PP" + std::to_string(pp_counter++);
        }

        std::cout << prefix << ": ";

        bool has_tasks = false;
        bool first_task = true;

        for (int i = 0; i < graph.tasks; ++i) {
            if (TaskProcessors[i] == p) {
                has_tasks = true;
                if (!first_task) std::cout << ", ";
                std::cout << "T" << i << "(" << task_start_time[i] << ")";
                first_task = false;
            }
        }

        if (!has_tasks) std::cout << "brak";
        std::cout << "\n";
    }

    return 0;
}
