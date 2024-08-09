#include <fcntl.h>
#include <ncurses.h>
#include <regex>
#include <stdio.h>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <vector>

/*********************************************************/
/*             Globals                                   */
/*********************************************************/

FILE* lf; // Log file
uint16_t screen_width = 20;
uint16_t screen_height = 5;
uint16_t input_line_index = screen_height;
char blank_char = '-';
std::string input_line_prefix = "$ :";

using Location = std::pair<int, int>;
using Words = std::vector<std::pair<Location, std::string>>;
Words current_words;

// Color pairs
#define CP_RED 1
#define CP_BLUE 2
#define CP_GREEN 3

#define LOG(...)                                                               \
    fprintf(lf, __VA_ARGS__);                                                  \
    fprintf(lf, "\n");                                                         \
    fflush(lf)

/*********************************************************/
/*             Structs                                   */
/*********************************************************/

/*********************************************************/
/*             Utils                                     */
/*********************************************************/

std::string new_word() {
    const std::string words[]{"hello", "something", "hi", "goat", "cpp"};
    const int word_count = 5;
    const int word_idx = rand() % word_count;
    return words[word_idx];
}
std::pair<int, int> new_word_location(int word_length) {
    int x = (rand() % screen_width) - word_length;
    int y = rand() % screen_height;
    return {x, y};
}

void print_empty() {
    for (uint16_t i = 0; i < screen_height; i++) {
        printw("%s\n", std::string(screen_width, blank_char).c_str());
    }
}

void print_words(std::string& input_line) {
    for (const auto& [loc, w] : current_words) {
        std::regex re(input_line);
        std::smatch m;
        if (std::regex_search(w, m, re)) {
            if (m.prefix().length() == 0 && m.suffix().length() == 0) {
                // TODO:
                // auto w = new_word();
                // auto loc = new_word_location((int)w.length());
                // current_words.push_back(
                //     std::pair<Location, std::string>(loc, w));
                // int y = input_line_index;
                // int x = input_line_prefix.length();
                // move(x,y);
                // input_line.clear();
                // continue;
            }

            int x, y;
            getyx(stdscr, y, x);
            move(loc.first, loc.second);
            printw("%s", m.prefix().str().c_str());
            attron(COLOR_PAIR(CP_RED));
            printw("%s", m[0].str().c_str());
            attroff(COLOR_PAIR(CP_RED));
            printw("%s", m.suffix().str().c_str());
            move(x, y);
            // LOG("tevz");
            continue;
        }
        mvprintw(loc.first, loc.second, "%s", w.c_str());
    }
}

void generate_words() {
    auto w = new_word();
    auto loc = new_word_location((int)w.length());
    current_words = {std::pair<Location, std::string>(loc, w)};
}

void init_color_pairs() {
    // Define color pairs
    init_pair(CP_RED, COLOR_RED, -1);
    init_pair(CP_BLUE, COLOR_BLUE, -1);
    init_pair(CP_GREEN, COLOR_GREEN, -1);
}

/*********************************************************/
/*             Main                                      */
/*********************************************************/

int init() {
    // Init debug log
    lf = fopen("t4.log", "w+");

    // init terminal
    initscr();
    if (!has_colors()) {
        endwin();
        printf("Your terminal does not support color\n");
        return 1;
    }

    std::srand(42);

    start_color();        // Start color functionality
    use_default_colors(); // Use default terminal colors
    init_color_pairs();
    // Initialize screen
    cbreak();             // Line buffering disabled
    keypad(stdscr, TRUE); // We get F1, F2, etc...
    noecho();             // Don't echo() while we do getch

    generate_words();

    return 0;
}

int main() {
    if (init()) {
        printf("Failed to init\n");
        return 1;
    }
    LOG("begin");

    std::string input_line;
    int y = input_line_index;
    int x = input_line_prefix.length();
    while (true) {
        clear(); // Clear the screen
        // INFO: print empty screen
        print_empty();
        // INFO: print words on top
        print_words(input_line);

        move(y, 0); // Move cursor to correct position
        // Print input line
        attron(COLOR_PAIR(CP_GREEN));
        printw("%s", (input_line_prefix + input_line).c_str());
        attroff(COLOR_PAIR(CP_GREEN));
        move(y, x); // Move cursor to correct position

        int ch = getch(); // Get user input

        switch (ch) {
        case KEY_BACKSPACE:
        case 127: // Handle backspace
            if (x > input_line_prefix.length()) {
                input_line.erase(x - input_line_prefix.length() - 1, 1);
                x--;
            }
            // } else if (y > 0) {
            //     x = lines[y - 1].length();
            //     lines[y - 1] += lines[y];
            //     lines.erase(lines.begin() + y);
            //     y--;
            // }
            break;
        // case '\n': // Handle new line
        //     lines.insert(lines.begin() + y + 1, lines[y].substr(x));
        //     lines[y] = lines[y].substr(0, x);
        //     y++;
        //     x = 0;
        //     break;
        // case KEY_LEFT:
        //     if (x > 0) {
        //         x--;
        //     } else if (y > 0) {
        //         y--;
        //         x = lines[y].size();
        //     }
        //     break;
        // case KEY_RIGHT:
        //     if (x < lines[y].size()) {
        //         x++;
        //     } else if (y < lines.size() - 1) {
        //         y++;
        //         x = 0;
        //     }
        //     break;
        // case KEY_UP:
        //     if (y > 0) {
        //         y--;
        //         x = std::min(x, (int)lines[y].size());
        //     }
        //     break;
        // case KEY_DOWN:
        //     if (y < lines.size() - 1) {
        //         y++;
        //         x = std::min(x, (int)lines[y + 1].size());
        //     }
        //     break;
        default:
            // insert into input line
            input_line.insert(x - input_line_prefix.length(), 1, (char)ch);
            x++;
            break;
        }
    }

    // Clean up
    endwin();

    return 0;
}