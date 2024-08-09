#include "words.h"
#include <ncurses.h>
#include <regex>
#include <string>
#include <vector>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

/*********************************************************/
/*             Globals                                   */
/*********************************************************/

FILE *lf; // Log file
uint16_t screen_width = 20;
uint16_t screen_height = 5;
uint16_t input_line_index = screen_height;
char blank_char = '-';
std::string input_line_prefix = "$ :";

// Color pairs
#define CP_RED 1
#define CP_BLUE 2
#define CP_GREEN 3

#define LOG(...) fprintf(lf, __VA_ARGS__);fprintf(lf,"\n");fflush(lf)

/*********************************************************/
/*             Structs                                   */
/*********************************************************/

// Proposition:
// create a structure that only saves words, and their location.
// clear screen with blanks, and then add words

// Create a structure that represents an array of strings and empty strings
// loop through structure and print objects
// Grid:
//
// +==================+
// |     |      |     |
// |     | Cell |     | Line
// |     |      |     |
// +- - - - - - -  - -+
// |     |      |     |
// |     |      |     |
// |     |      |     |
// +==================+
// |                  |
// |      ...         |

struct Grid {
    // using Line = std::vector<std::string>;
    struct Line {
        std::vector<std::string> cells;
        std::vector<bool> blank;

        void insert(std::string && word) {
            if (cells.size() != 1) {
                // TODO handle
                std::printf("Cells size is not 1");
                exit(1);
            }
            auto ostr = std::move(cells[0]);
            auto left = ostr.substr(0,5);
            auto right = ostr.substr(word.length() + 5);
            cells[0] = std::move(left);
            cells.push_back(std::move(word));
            cells.push_back(std::move(right));
            blank[1] = false;
            blank[2] = true;
        }
    };
    std::vector<Line> lines;
};

/*********************************************************/
/*             Utils                                     */
/*********************************************************/

Grid emptyScreen() {
    Grid ret;
    Grid::Line line{
        .cells = {std::string(screen_width, blank_char)},
        .blank = {true},
    };
    for (uint16_t i = 0; i < screen_height; i++) {
        ret.lines.push_back(line);
    }
    return ret;
}

void printGrid(const Grid& grid, const std::string& input_line) {
    for (const auto& line : grid.lines) {
        for (size_t i = 0; i < line.cells.size(); i++) {
            auto cell = line.cells[i];
            if (!line.blank[i]) {
                // check match and color print
                std::regex re(input_line);
                std::smatch m;
                if (std::regex_search(cell, m, re)) {
                    if (m.prefix().length() == 0 && m.suffix().length() == 0) {
                        // complete match
                        printw("%s", std::string(m[0].length(), blank_char).c_str());
                        continue;
                    }
                    printw("%s", m.prefix().str().c_str());
                    attron(COLOR_PAIR(CP_BLUE));
                    printw("%s", m[0].str().c_str());
                    attroff(COLOR_PAIR(CP_BLUE));
                    printw("%s", m.suffix().str().c_str());
                    LOG("tevz");
                    continue;
                }
            }
            // normal print
            printw("%s", cell.c_str());
        }
        printw("\n");
    }
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

    start_color();        // Start color functionality
    use_default_colors(); // Use default terminal colors
    init_color_pairs();
    // Initialize screen
    cbreak();             // Line buffering disabled
    keypad(stdscr, TRUE); // We get F1, F2, etc...
    noecho();             // Don't echo() while we do getch
    return 0;
}

int main() {
    if (init()) {
        printf("Failed to init\n");
        return 1;
    }
    LOG("begin");

    std::string input_line;
    // Create a buffer to store lines of text
    Grid grid = emptyScreen();

    // TODO
    // Insert word onto screen
    // grid.lines[0].cells[0].replace(5, word().length(), word());
    grid.lines[0].insert(word());

    int y = input_line_index;
    int x = input_line_prefix.length();

    while (true) {
        clear(); // Clear the screen
        // Print Grid
        printGrid( grid, input_line );
        // for (i = 0; i < grid.lines.size(); ++i) {
        //     mvprintw(i, 0, "%s", lines[i].c_str());
        // }

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