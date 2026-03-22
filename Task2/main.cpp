#include <iostream>
#include <fstream>
#include <vector>

std::vector<std::vector<std::pair<int, int>>> readGraph(std::string& file_name, std::istream& in) {
    std::ifstream input(file_name);
    if (!input) {
        std::cerr << "Nie mozna otworzyc pliku wejsciowego\n";
        return {};
    }

    std::vector<std::vector<std::pair<int, int>>> graph;
    std::string line;

    int tasks = 0;
    if (std::getline(input, line)) {
        std::string target = "@tasks ";
        size_t position = line.find(target);
        if (position != std::string::npos) {
            std::string valueString = line.substr(position + target.length());
            tasks = std::stoi(valueString);
        }
        std::cout << tasks << std::endl;
    }

    for (int i = 0; i < tasks; i++) {

    }

    return graph;
}

int main() {
    std::string input = "../input.txt";

    readGraph(input, std::cin);
    return 0;
}