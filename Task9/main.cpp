#include <iostream>
#include <fstream>
#include <vector>
#include <queue>
#include <algorithm>
#include <iomanip>
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

struct Params {
    double k1 = 0.0;
    double k2 = 0.0;
    double k3 = 0.0;
    double T_max = 0.0;
    double F_max = 0.0;
};

struct ScheduleInfo {
    std::vector<int> start_times;
    int total_time = 0;
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

std::vector<int> topologicalSort(const TaskGraph& graph) {
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

    std::vector<int> stack;
    while (!queue.empty()) {
        int current_task = queue.front();
        queue.pop();
        stack.push_back(current_task);

        for (auto& edge : graph.graph[current_task]) {
            in_degree[edge.target]--;
            if (in_degree[edge.target] == 0) {
                queue.push(edge.target);
            }
        }
    }

    return stack;
}

// Sprawdzanie czy procesor (HC) jest wolny
bool isHCAvailable(const TaskGraph& graph, const std::vector<int>& assignment, int task, int proc) {
    if (graph.processors[proc].type == 1) return true; // PP robi kolejkę, jest chill

    for (int i = 0; i < (int)assignment.size(); ++i) {
        if (i != task && assignment[i] == proc) {
            return false;
        }
    }
    return true;
}

ScheduleInfo calculateTimes(const TaskGraph& graph, const std::vector<int>& assignment, const std::vector<int>& topo) {
    std::vector<int> ready(graph.tasks, 0);
    std::vector<int> start(graph.tasks, -1);
    std::vector<int> proc_avail(graph.proc, 0);
    int total_time = 0;

    for (int task : topo) {
        int proc = assignment[task];
        if (proc == -1) continue;

        if (graph.processors[proc].type == 1) { // PP
            start[task] = std::max(ready[task], proc_avail[proc]);
            proc_avail[proc] = start[task] + graph.times[task][proc];
        } else { // HC
            start[task] = ready[task];
        }

        int finish = start[task] + graph.times[task][proc];
        total_time = std::max(total_time, finish);

        for (const auto& edge : graph.graph[task]) {
            ready[edge.target] = std::max(ready[edge.target], finish);
        }
    }

    return {start, total_time};
}

int calculateCost(const TaskGraph& graph, const std::vector<int>& assignment) {
    int cost = 0;
    std::vector<bool> proc_used(graph.proc, false);

    for (int i = 0; i < graph.tasks; ++i) {
        int proc = assignment[i];
        if (proc != -1) {
            cost += graph.costs[i][proc];
            proc_used[proc] = true;
        }
    }

    for (int p = 0; p < graph.proc; ++p) {
        if (!proc_used[p]) continue;

        cost += graph.processors[p].cost;
        for (int b = 0; b < graph.bus; ++b) {
            if (p < (int)graph.channels[b].connected_processor.size() && graph.channels[b].connected_processor[p] != 0) {
                cost += graph.channels[b].cost;
            }
        }
    }

    return cost;
}

double calculateF(const Params& p, int cost, int time) {
    double penalty = std::max(0.0, (double)time - p.T_max);
    return p.k1 * cost + p.k2 * time + p.k3 * penalty;
}

void solve(const TaskGraph& graph, const Params& params) {
    std::vector<int> assignment(graph.tasks, -1);
    std::vector<int> topo = topologicalSort(graph);

    int final_cost = 0, final_time = 0;
    double final_f = 0.0;
    std::vector<int> final_starts;

    for (int task : topo) {
        int best_proc = -1;
        int best_time = 2e9, best_cost = 2e9;
        double best_f = 0.0;
        bool found = false;

        for (int p = 0; p < graph.proc; ++p) {
            if (graph.times[task][p] < 0 || !isHCAvailable(graph, assignment, task, p)) continue;

            std::vector<int> temp_assign = assignment;
            temp_assign[task] = p;

            ScheduleInfo sched = calculateTimes(graph, temp_assign, topo);
            int cost = calculateCost(graph, temp_assign);
            double current_f = calculateF(params, cost, sched.total_time);

            if (current_f > params.F_max) continue;

            if (!found || cost < best_cost ||
               (cost == best_cost && sched.total_time < best_time) ||
               (cost == best_cost && sched.total_time == best_time && p < best_proc)) {
                best_proc = p;
                best_time = sched.total_time;
                best_cost = cost;
                best_f = current_f;
                found = true;
            }
        }

        if (!found) {
            std::cerr << "Nie istnieje lokalnie dopuszczalny zasob dla zadania T" << task << "\n";
            return;
        }

        assignment[task] = best_proc;

        // Zapisujemy stany na dla wybranego kandydata
        ScheduleInfo sched = calculateTimes(graph, assignment, topo);
        final_starts = sched.start_times;
        final_time = sched.total_time;
        final_cost = calculateCost(graph, assignment);
        final_f = calculateF(params, final_cost, final_time);
    }

    int penalty = std::max(0, final_time - (int)params.T_max);
    std::cout <<"-------------Wynik----------- \n";
    std::cout << "\nNajszybszy czas calkowity (Makespan): " << final_time << "\n";
    std::cout << "Calkowity koszt: " << final_cost << "\n";
    std::cout << "Kara: " << penalty << "\n";
    std::cout << std::fixed << std::setprecision(4) << "F: " << final_f << "\n";

    std::vector<std::string> proc_names(graph.proc);
    std::vector<int> pp_indices;
    std::vector<int> hc_indices;

    int hc_cnt = 0, pp_cnt = 0;
    for (int p = 0; p < graph.proc; ++p) {
        if (graph.processors[p].type == 0) {
            proc_names[p] = "HC" + std::to_string(hc_cnt++);
            hc_indices.push_back(p);
        } else {
            proc_names[p] = "PP" + std::to_string(pp_cnt++);
            pp_indices.push_back(p);
        }
    }

    std::cout << "\nWykorzystana Architektura:\n";
    auto print_proc_tasks = [&](int p) {
        std::cout << proc_names[p] << ": ";
        bool first = true;
        for (int t = 0; t < graph.tasks; ++t) {
            if (assignment[t] == p) {
                if (!first) std::cout << ", ";
                std::cout << "T" << t << "(" << final_starts[t] << ")";
                first = false;
            }
        }
        if (first) std::cout << "brak";
        std::cout << "\n";
    };

    for (int p : pp_indices) print_proc_tasks(p);
    for (int p : hc_indices) print_proc_tasks(p);

    std::cout << "\nWykorzystane Kanaly:\n";
    std::vector<bool> proc_used(graph.proc, false);
    for (int t = 0; t < graph.tasks; ++t) {
        if (assignment[t] != -1) {
            proc_used[assignment[t]] = true;
        }
    }

    for (int b = 0; b < graph.bus; ++b) {
        std::cout << "CHAN" << b << ": ";
        bool first = true;

        auto check_and_print_bus_proc = [&](int p) {
            if (proc_used[p] && p < (int)graph.channels[b].connected_processor.size() && graph.channels[b].connected_processor[p] != 0) {
                if (!first) std::cout << ", ";
                std::cout << proc_names[p];
                first = false;
            }
        };

        for (int p : pp_indices) check_and_print_bus_proc(p);
        for (int p : hc_indices) check_and_print_bus_proc(p);

        if (first) std::cout << "brak";
        std::cout << "\n";
    }
}

int main() {
    std::string input;
    std::cout << "Podaj nazwe pliku wejsciowego (razem z .txt): \n";
    std::cin >> input;

    TaskGraph graph = readGraph(input);

    if (graph.tasks == 0) return 1;

    Params params;

    std::cout << "Podaj k1: \n";
    std::cin >> params.k1;

    std::cout << "Podaj k2: \n";
    std::cin >> params.k2;

    std::cout << "Podaj k3 (kara): \n";
    std::cin >> params.k3;

    std::cout << "Podaj granice czasowa (hard constraint T_max): \n";
    std::cin >> params.T_max;

    std::cout << "Podaj maksymalna wartosc funkcji F (F_max): \n";
    std::cin >> params.F_max;

    solve(graph, params);
}