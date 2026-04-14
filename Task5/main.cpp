#include <iostream>
#include <fstream>
#include <vector>
#include <queue>
#include <algorithm>
#include <utility>
#include <string>
#include <random>

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
            graph.graph[i].emplace_back(Edge{target, weight, 0});
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

struct ScheduleResult {
    int makespan;
    int cost;
    std::vector<int> task_processor;
    std::vector<int> task_start_time;
    std::vector<int> task_finish_time;
};

ScheduleResult scheduleTasks(const TaskGraph &graph, int T_max, const int LIMIT = 5'000'000) {
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

    while (attempts < LIMIT) {
        attempts++;

        std::vector<int> current_in_degree = in_degree;

        std::vector<int> task_processor(graph.tasks, -1);
        std::vector<int> task_start_time(graph.tasks, 0);
        std::vector<int> task_finish_time(graph.tasks, 0);
        std::vector<int> proc_available_time(graph.proc, 0);

        std::queue<int> queue;
        for (int i = 0; i < graph.tasks; ++i) {
            if (current_in_degree[i] == 0) queue.push(i);
        }

        bool hard_constraint_failed = false;

        while (!queue.empty()) {
            int current_task = queue.front();
            queue.pop();

            int chosen_proc = -1;

            if (predecessors[current_task].empty()) {
                std::uniform_int_distribution<int> dist(0, graph.proc - 1);
                chosen_proc = dist(rng);
            } else {
                int prev_task = predecessors[current_task][0].first;
                int prev_proc = task_processor[prev_task];

                if (graph.processors[prev_proc].type == 0) {
                    std::uniform_int_distribution<int> dist(0, graph.proc - 2);
                    do {
                        chosen_proc = dist(rng) % graph.proc;
                    } while (chosen_proc == prev_proc);
                } else {
                    std::uniform_int_distribution<int> coin(0, 1);
                    if (coin(rng) == 0) {
                        chosen_proc = prev_proc;
                    } else {
                        std::uniform_int_distribution<int> dist(0, graph.proc - 1);
                        chosen_proc = dist(rng);
                    }
                }
            }

            task_processor[current_task] = chosen_proc;

            int max_pred_finish_time = 0;

            for (const auto& pred : predecessors[current_task]) {
                int p_id = pred.first;
                if (task_finish_time[p_id] > max_pred_finish_time) {
                    max_pred_finish_time = task_finish_time[p_id];
                }
            }

            // OBLICZANIE CZASU STARTU I ZAKONCZENIA
            int start_time = std::max(proc_available_time[chosen_proc], max_pred_finish_time);
            int finish_time = start_time + graph.times[current_task][chosen_proc];

            // SPRAWDZENIE HARD TIME CONSTRAINT
            if (finish_time > T_max) {
                hard_constraint_failed = true;
                break;
            }

            // ZAPIS STANÓW
            task_start_time[current_task] = start_time;
            task_finish_time[current_task] = finish_time;
            proc_available_time[chosen_proc] = finish_time;

            for (const auto& edge : graph.graph[current_task]) {
                current_in_degree[edge.target]--;
                if (current_in_degree[edge.target] == 0) {
                    queue.push(edge.target);
                }
            }
        }

        // JEŚLI ZNALEZIONO ROZWIĄZANIE:
        if (!hard_constraint_failed) {
            int final_makespan = *std::ranges::max_element(task_finish_time);

            int final_cost = 0;
            std::vector<bool> used_procs(graph.proc, false);
            for (int p : task_processor) {
                if (p != -1) used_procs[p] = true;
            }
            for (int i = 0; i < graph.proc; ++i) {
                if (used_procs[i]) {
                    final_cost += graph.processors[i].cost;
                }
            }

            return {final_makespan, final_cost, task_processor, task_start_time, task_finish_time};
        }
    }
    return {};
}

// Funkcja pomocnicza do wyszukiwania wspolnego kanalu miedzy procesorami
int findUsedChannel(const TaskGraph& graph, int p1, int p2) {
    int max_bw = -1;
    int best_c = -1;
    for (int c = 0; c < graph.bus; ++c) {
        if (graph.channels[c].connected_processor[p1] == 1 && graph.channels[c].connected_processor[p2] == 1) {
            if (graph.channels[c].bandwidth > max_bw) {
                max_bw = graph.channels[c].bandwidth;
                best_c = c;
            }
        }
    }
    return best_c;
}


int main() {
    std::string input;
    std::cout << "Podaj nazwe pliku wejsciowego (razem z .txt): \n";
    std::cin >> input;

    TaskGraph graph = readGraph(input);

    if (graph.tasks == 0) return 1;

    int T_max;
    std::cout << "Podaj granice czasowa (hard constraint): \n";
    std::cin >> T_max;

    ScheduleResult result = scheduleTasks(graph, T_max);

    if (result.task_processor.empty()) {
        std::cout << "\nNie udalo sie znalezc rozwiazania dla T_max = " << T_max << ".\n";
        return 0;
    }

    std::cout << "\nCzas calkowity (Makespan): " << result.makespan << "\n";
    std::cout << "Koszt architektury: " << result.cost << "\n\n";

    std::cout << "Wykorzystana Architektura:\n";

    std::vector<std::string> proc_names(graph.proc);
    std::vector<bool> used_processors(graph.proc, false);

    int hc_counter = 0;
    int pp_counter = 0;

    // Generowanie nazw (HC0, PP0...) i wypisywanie przydzialu
    for (int p = 0; p < graph.proc; ++p) {
        std::string prefix;
        if (graph.processors[p].type == 0) {
            prefix = "HC" + std::to_string(hc_counter++);
        } else {
            prefix = "PP" + std::to_string(pp_counter++);
        }
        proc_names[p] = prefix;

        std::cout << prefix << ": ";

        bool has_tasks = false;
        bool first_task = true;

        for (int i = 0; i < graph.tasks; ++i) {
            if (result.task_processor[i] == p) {
                has_tasks = true;
                used_processors[p] = true;

                if (!first_task) std::cout << ", ";
                std::cout << "T" << i << "(" << result.task_start_time[i] << ")";
                first_task = false;
            }
        }

        if (!has_tasks) {
            std::cout << "brak";
        }
        std::cout << "\n";
    }

    std::cout << "\nWykorzystane Kanaly:\n";
    std::vector<bool> used_channels(graph.bus, false);

    for (int i = 0; i < graph.tasks; ++i) {
        for (const auto& edge : graph.graph[i]) {
            int p_src = result.task_processor[i];
            int p_dst = result.task_processor[edge.target];

            if (p_src != p_dst) {
                int channel = findUsedChannel(graph, p_src, p_dst);
                if (channel != -1) {
                    used_channels[channel] = true;
                }
            }
        }
    }

    bool any_channel_used = false;
    for (int c = 0; c < graph.bus; ++c) {
        if (used_channels[c]) {
            any_channel_used = true;
            std::cout << "CHAN" << c << ": ";

            bool first_conn = true;
            for (int p = 0; p < graph.proc; ++p) {
                if (graph.channels[c].connected_processor[p] == 1 && used_processors[p]) {
                    if (!first_conn) std::cout << ", ";
                    std::cout << proc_names[p];
                    first_conn = false;
                }
            }
            std::cout << "\n";
        }
    }

    if (!any_channel_used) {
        std::cout << "brak\n";
    }

    return 0;
}
