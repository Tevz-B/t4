#include <fcntl.h>
#include <ncurses.h>
#include <regex>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <vector>

#include "log.h"
#include "words.h"

// TODO: read from input
// - window size
// - random seed
// -

/*********************************************************/
/*             Globals                                   */
/*********************************************************/

static const uint16_t screen_width = 40;
static const uint16_t screen_height = 20;
static const uint16_t input_line_index = screen_height;
static const char blank_char = '-';
static const std::string input_line_prefix = "$ :";

using Location = std::pair<int, int>;
using Words = std::vector<std::pair<Location, std::string>>;
Words current_words;

// Color pairs
#define CP_RED 1
#define CP_BLUE 2
#define CP_GREEN 3

/*********************************************************/
/*             Structs                                   */
/*********************************************************/

/*********************************************************/
/*             Utils                                     */
/*********************************************************/

std::string new_word_old() {
    const std::string words[]{"hello", "something", "hi", "goat", "cpp"};
    const int word_count = 5;
    const int word_idx = rand() % word_count;
    return words[word_idx];
}

std::string new_generated_word() {
    const int word_count = 2;
    const std::string words[word_count]{"j", "k", /* "up", "down" */};
    const int word_idx = rand() % word_count;
    auto rnd_wrd_suff = words[word_idx];
    auto rnd_wrd_pre = std::to_string(rand() % 1000);
    return rnd_wrd_pre + rnd_wrd_suff;
}

std::pair<int, int> new_word_location(int word_length) {
    int x = rand() % (screen_width - word_length);
    int y = rand() % (screen_height - 1); // exclude last line (input)
    LOG("new loc: y:%i, x:%i", y, x);
    return {y, x};
}

auto& add_new_word() {
    // auto w = new_generated_word();
    auto w = word_from_file();
    auto loc = new_word_location((int)w.length());
    current_words.push_back({loc, w});
    return current_words.back();
}

void print_empty() {
    printw("Press Control-D to exit :^D\n");           // First line
    for (uint16_t i = 0; i < screen_height - 1; i++) { // The rest
        printw("%s\n", std::string(screen_width, blank_char).c_str());
    }
}

void print_words(std::string& input_line, int& y, int& x) {
    std::vector<std::string> rm_words;
    for (const auto& [loc, w] : current_words) {
        std::regex re(input_line);
        std::smatch m;
        if (std::regex_search(w, m, re)) {
            // full match
            if (m.prefix().length() == 0 && m.suffix().length() == 0) {
                y = input_line_index;
                x = input_line_prefix.length();
                input_line.clear();
                rm_words.push_back(w);
                continue;
            }

            // partial match
            int x, y;
            getyx(stdscr, y, x);
            move(loc.first, loc.second);
            printw("%s", m.prefix().str().c_str());
            attron(COLOR_PAIR(CP_RED));
            printw("%s", m[0].str().c_str());
            attroff(COLOR_PAIR(CP_RED));
            printw("%s", m.suffix().str().c_str());
            move(x, y);
            continue;
        }
        mvprintw(loc.first, loc.second, "%s", w.c_str());
    }

    for (const auto& w : rm_words) {
        // remove typed word
        current_words.erase(
            std::remove_if(current_words.begin(), current_words.end(),
                           [&w](const auto& i) { return i.second == w; }),
            current_words.end());
        // current_words.pop_back();

        auto& [loc, word] = add_new_word();
        // Print new word
        mvprintw(loc.first, loc.second, "%s", word.c_str());
    }
}

/*********************************************************/
/*             Main                                      */
/*********************************************************/

int init() {
    // Init debug log
    init_log();

    if( !read_words_file("words.txt") ) {
        printf("Failed to read words file\n");
        return 1;
    }

    // init terminal
    initscr();
    if (!has_colors()) {
        endwin();
        printf("Your terminal does not support color\n");
        return 1;
    }


    std::srand(42);

    start_color();                    // Start color functionality
    use_default_colors();             // Use default terminal colors
    init_pair(CP_RED, COLOR_RED, -1); // -1 = transparent background
    init_pair(CP_BLUE, COLOR_BLUE, -1);
    init_pair(CP_GREEN, COLOR_GREEN, -1);
    // Initialize screen
    cbreak();             // Line buffering disabled
    keypad(stdscr, TRUE); // We get F1, F2, etc...
    noecho();             // Don't echo() while we do getch

    add_new_word();
    add_new_word();
    add_new_word();

    return 0;
}

int main() {
    if (init()) {
        printf("Failed to init\n");
        return 1;
    }
    LOG("begin");

    std::string input_line;
    int y = (int)input_line_index;
    int x = (int)input_line_prefix.length();
    while (true) {
        clear(); // Clear the screen
        // INFO: print empty screen
        print_empty();
        // INFO: print words on top
        print_words(input_line, y, x);

        move(y, 0); // Move cursor
        // Print input line
        attron(COLOR_PAIR(CP_GREEN));
        printw("%s", (input_line_prefix + input_line).c_str());
        attroff(COLOR_PAIR(CP_GREEN));
        move(y, x); // Move cursor to correct position
        LOG("move y:%d x:%d", y, x);

        // Handle Input
        int ch = getch(); // Get user input
        switch (ch) {
                // backspace
            case KEY_BACKSPACE:
            case 127:
                if (x > (int)input_line_prefix.length()) {
                    input_line.erase(x - input_line_prefix.length() - 1, 1);
                    x--;
                }
                break;
            // whitespace - ignore
            case '\n':
            case '\t':
            case '\r':
            case ' ':
                break;
            // quit (Control-D)
            case 4:
                goto exitLoop;
                break;
            // other chars
            default:
                // insert into input line
                input_line.insert(x - input_line_prefix.length(), 1, (char)ch);
                x++;
                break;
        }
    }
exitLoop:

    // Clean up
    endwin();

    return 0;
}
