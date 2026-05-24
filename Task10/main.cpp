#include <iostream>
#include <fstream>
#include <vector>
#include <queue>
#include <algorithm>
#include <iomanip>
#include <string>
#include <sstream>

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
    std::vector<std::vector<Edge>> parents;
    std::vector<int> unassigned_tasks;
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
    graph.parents.resize(graph.tasks);

    for (int i = 0; i < graph.tasks; ++i) {
        int edges;
        input >> trash_string >> edges;

        if (trash_string.substr(0, 2) == "UT") {
            graph.unassigned_tasks.push_back(std::stoi(trash_string.substr(2)));
        }

        for (int e = 0; e < edges; ++e) {
            int target, weight;
            input >> target >> trash_char >> weight >> trash_char;
            graph.graph[i].emplace_back(Edge{target, weight, 0});
            graph.parents[target].emplace_back(Edge{i, weight, 0});
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

struct ScheduledTask {
    int id;
    int start_time;
};

// żeby pamiętać jak procek się nazywa
struct ProcPlan {
    std::string name;
    std::vector<ScheduledTask> tasks;
};

std::vector<ProcPlan> readArchitecture(const std::string& filename, int proc_count, const TaskGraph& graph) {
    std::vector<ProcPlan> arch(proc_count);

    // Inicjalizacja nazw procesorów
    int hc_cnt = 0, pp_cnt = 0;
    for (int i = 0; i < proc_count; ++i) {
        if (graph.processors[i].type == 0) { // HC
            arch[i].name = "HC" + std::to_string(hc_cnt++);
        } else { // PP
            arch[i].name = "PP" + std::to_string(pp_cnt++);
        }
    }

    std::ifstream file(filename);
    if (!file) return arch;

    std::string line;
    while (std::getline(file, line)) {
        if (line.empty() || line.find(':') == std::string::npos) continue;

        size_t colon_pos = line.find(':');
        std::string proc_name = line.substr(0, colon_pos);

        int target_proc = -1;
        for (int i = 0; i < proc_count; ++i) {
            if (arch[i].name == proc_name) {
                target_proc = i;
                break;
            }
        }
        if (target_proc == -1) continue;

        std::string tasks_str = line.substr(colon_pos + 1);
        std::stringstream ss(tasks_str);
        std::string token;
        while (ss >> token) {
            size_t t_pos = token.find('T');
            size_t open_b = token.find('(');
            size_t close_b = token.find(')');
            if (t_pos != std::string::npos && open_b != std::string::npos && close_b != std::string::npos) {
                int id = std::stoi(token.substr(t_pos + 1, open_b - t_pos - 1));
                int time = std::stoi(token.substr(open_b + 1, close_b - open_b - 1));
                arch[target_proc].tasks.push_back({id, time});
            }
        }
    }
    return arch;
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

void assignUnassignedTasks(const TaskGraph& graph, std::vector<ProcPlan>& architecture) {
    struct TaskInfo {
        int proc_id = -1;
        int end_time = 0;
    };
    std::vector<TaskInfo> task_info(graph.tasks);
    std::vector<int> proc_ready_time(graph.proc, 0);
    std::vector<int> proc_task_count(graph.proc, 0);

    for (int p = 0; p < graph.proc; ++p) {
        proc_task_count[p] = architecture[p].tasks.size();
        for (const auto& t : architecture[p].tasks) {
            task_info[t.id].proc_id = p;
            int end_t = t.start_time + graph.times[t.id][p];
            task_info[t.id].end_time = end_t;
            proc_ready_time[p] = std::max(proc_ready_time[p], end_t);
        }
    }

    std::vector<bool> is_ut(graph.tasks, false);
    for (int ut : graph.unassigned_tasks) is_ut[ut] = true;

    auto get_comm_time = [&](int p1, int p2, int weight) {
        if (p1 == p2) return 0;
        int min_t = 1e9;
        for (const auto& bus : graph.channels) {
            if (bus.connected_processor[p1] && bus.connected_processor[p2]) {
                int t = (weight + bus.bandwidth - 1) / bus.bandwidth;
                min_t = std::min(min_t, t);
            }
        }
        return (min_t == 1e9) ? 0 : min_t;
    };

    std::vector<int> topo = topologicalSort(graph);

    for (int task : topo) {
        if (!is_ut[task]) continue;

        int best_pp = -1;
        int min_tasks = 1e9;
        int earliest_end = 1e9;

        for (int p = 0; p < graph.proc; ++p) {
            if (graph.processors[p].type == 1) { // PP ma type == 1
                if (proc_task_count[p] <= min_tasks) {
                    int max_parent_end = 0;
                    for (const auto& parent : graph.parents[task]) {
                        int parent_proc = task_info[parent.target].proc_id;
                        if (parent_proc == -1) continue;
                        int comm_time = get_comm_time(parent_proc, p, parent.weight);
                        max_parent_end = std::max(max_parent_end, task_info[parent.target].end_time + comm_time);
                    }

                    int start_time = std::max(proc_ready_time[p], max_parent_end);
                    int end_time = start_time + graph.times[task][p];

                    if (proc_task_count[p] < min_tasks || (proc_task_count[p] == min_tasks && end_time < earliest_end)) {
                        min_tasks = proc_task_count[p];
                        best_pp = p;
                        earliest_end = end_time;
                    }
                }
            }
        }

        if (best_pp != -1) {
            int max_parent_end = 0;
            for (const auto& parent : graph.parents[task]) {
                int parent_proc = task_info[parent.target].proc_id;
                int comm_time = get_comm_time(parent_proc, best_pp, parent.weight);
                max_parent_end = std::max(max_parent_end, task_info[parent.target].end_time + comm_time);
            }

            int start_time = std::max(proc_ready_time[best_pp], max_parent_end);

            task_info[task].proc_id = best_pp;
            task_info[task].end_time = start_time + graph.times[task][best_pp];
            proc_ready_time[best_pp] = task_info[task].end_time;
            proc_task_count[best_pp]++;

            architecture[best_pp].tasks.push_back({task, start_time});

            std::sort(architecture[best_pp].tasks.begin(), architecture[best_pp].tasks.end(),
                [](const ScheduledTask& a, const ScheduledTask& b) {
                    return a.start_time < b.start_time;
            });
        }
    }
}

int main() {
    TaskGraph graph = readGraph("../GRAF.200.txt");
    auto architecture = readArchitecture("../architectureGRAF.txt", graph.proc, graph);

    assignUnassignedTasks(graph, architecture);

    std::vector<int> task_to_proc(graph.tasks, -1);
    for (int p = 0; p < architecture.size(); ++p) {
        for (const auto& task : architecture[p].tasks) {
            task_to_proc[task.id] = p;
        }
    }

    std::cout << "\n--- Zaktualizowana Architektura ---\n";
    for (const auto& p : architecture) {
        std::cout << p.name << ": ";
        for (const auto& task : p.tasks) {
            std::cout << "T" << task.id << "(" << task.start_time << ") ";
        }
        std::cout << "\n";
    }

    std::cout << "\nWykorzystane Kanaly:\n";
    std::vector<bool> used_channels(graph.bus, false);

    // które kanały są używane na podstawie mapowania task_to_proc
    for (int i = 0; i < graph.tasks; ++i) {
        int p_source = task_to_proc[i];
        if (p_source == -1) continue; // Zadanie nieprzypisane

        for (const auto& edge : graph.graph[i]) {
            int p_target = task_to_proc[edge.target];
            if (p_target != -1 && p_source != p_target) {
                for (int c = 0; c < graph.bus; ++c) {
                    if (graph.channels[c].connected_processor[p_source] == 1 &&
                        graph.channels[c].connected_processor[p_target] == 1) {
                        used_channels[c] = true;
                    }
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
                if (graph.channels[c].connected_processor[p] == 1) {
                    if (!first_conn) std::cout << ", ";
                    std::cout << architecture[p].name;
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
