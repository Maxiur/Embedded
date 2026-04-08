#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>
#include <random>
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

void scheduleTasks(const TaskGraph& graph, int T_max) {
    std::vector<int> in_degree(graph.tasks, 0);
    std::vector<std::vector<std::pair<int, int>>> predecessors(graph.tasks);

    for (int u = 0; u < graph.tasks; ++u) {
        for (const auto& edge : graph.graph[u]) {
            in_degree[edge.target]++;
            predecessors[edge.target].emplace_back(u, edge.weight);
        }
    }

    std::random_device rd;
    std::mt19937 rng(rd());

    int attempts = 0;

    while (true) {
        attempts++;

        // Kopia in-degree, bo będziemy ją modyfikować w trakcie próby
        std::vector<int> current_in_degree = in_degree;

        std::vector<int> task_processor(graph.tasks, -1); // na jakim procku jest zadanie
        std::vector<int> task_finish_time(graph.tasks, 0); // o ktorej zadanie sie konczy
        std::vector<int> proc_available_time(graph.proc, 0); // o ktorej procek bedzie wolny

        std::queue<int> queue;
        for (int i = 0; i < graph.tasks; ++i) {
            if (current_in_degree[i] == 0) queue.push(i);
        }

        bool hard_constraint_failed = false;

        while (!queue.empty()) {
            int current_task = queue.front();
            queue.pop();

            int chosen_proc = -1;

            // 2. WYBÓR PROCESORA
            if (predecessors[current_task].empty()) {
                // Zadanie startowe (T0) - losujemy dowolny
                std::uniform_int_distribution<int> dist(0, graph.proc - 1);
                chosen_proc = dist(rng);
            } else {
                int prev_task = predecessors[current_task][0].first;
                int prev_proc = task_processor[prev_task];

                if (graph.processors[prev_proc].type == 0) { // 0 to u Ciebie HC
                    // Poprzednio był HC - MUSIMY wziąć nowy procesor
                    std::uniform_int_distribution<int> dist(0, graph.proc - 2);
                    do {
                        chosen_proc = dist(rng) % graph.proc;
                    } while (chosen_proc == prev_proc);
                } else {
                    // Nie było HC - losujemy: ten sam (np. 50% szans) czy inny?
                    std::uniform_int_distribution<int> coin(0, 1);
                    if (coin(rng) == 0) {
                        chosen_proc = prev_proc;
                    } else {
                        // losujemy inny
                        std::uniform_int_distribution<int> dist(0, graph.proc - 1);
                        chosen_proc = dist(rng);
                    }
                }
            }

            task_processor[current_task] = chosen_proc;

            // 3. KALKULACJA CZASU (Z uwzględnieniem szyny)
            int max_pred_finish_time = 0;

            for (const auto& pred : predecessors[current_task]) {
                int p_id = pred.first;
                int weight = pred.second;
                int p_proc = task_processor[p_id];

                int comm_delay = 0;
                if (p_proc != chosen_proc) {
                    comm_delay = weight; // Placeholder!
                }

                int time_when_data_arrives = task_finish_time[p_id] + comm_delay;
                if (time_when_data_arrives > max_pred_finish_time) {
                    max_pred_finish_time = time_when_data_arrives;
                }
            }

            int start_time = std::max(proc_available_time[chosen_proc], max_pred_finish_time);
            int finish_time = start_time + graph.times[current_task][chosen_proc];

            // 4. SPRAWDZENIE HARD TIME CONSTRAINT
            if (finish_time > T_max) {
                hard_constraint_failed = true;
                break;
            }

            // Zapisujemy stany
            task_finish_time[current_task] = finish_time;
            proc_available_time[chosen_proc] = finish_time;

            // 5. ODBLOKOWANIE NASTĘPNIKÓW
            for (const auto& edge : graph.graph[current_task]) {
                current_in_degree[edge.target]--;
                if (current_in_degree[edge.target] == 0) {
                    queue.push(edge.target);
                }
            }
        }

        if (!hard_constraint_failed) {
            std::cout << "Znaleziono rozwiązanie po " << attempts << " próbach!\n";
            break;
        }
    }
}

int main() {
    TaskGraph graph = readGraph("../input.txt");

    int T_max = 81;

    if (graph.tasks > 0) {
        scheduleTasks(graph, T_max);
    }

    return 0;
}
