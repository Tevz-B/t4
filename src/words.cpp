#include "words.h"
#include "log.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

std::vector<std::string> words;

std::string word_from_file() { return words[rand() & words.size()]; }

bool read_words_file(const char* filename) {
    std::ifstream in(filename, std::ios_base::in);
    if (!in.is_open()) {
        LOG("failed to open words file : %s", filename);
        return false;
    }

    std::string file_contents;

    std::string line;
    while(getline(in, line))
        file_contents.append(line);

    words.clear();
    std::istringstream iss(file_contents);
    std::string word;

    // Extract words separated by whitespace
    while (iss >> word) {
        words.push_back(word);
    }

    LOG("read %lu words from file", words.size());
    return true;
}
