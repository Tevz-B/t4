#include "words.h"
#include <ncurses.h>
#include <vector>
#include <string>

uint16_t screen_width = 20;
uint16_t screen_height = 5;
uint16_t input_line_index = screen_height;
char blank_char = '-';

std::vector<std::string> emptyScreen() {
  std::vector<std::string> ret;
  for (uint16_t i = 0; i < screen_height; i++) {
    ret.push_back(std::string(screen_width, blank_char));
  }
  return ret;
}

int main() {
      if (!has_colors()) {
        endwin();
        printf("Your terminal does not support color\n");
        return;
      }
    // Initialize screen
    initscr();
    raw();                  // Line buffering disabled
    keypad(stdscr, TRUE);   // We get F1, F2, etc...
    noecho();               // Don't echo() while we do getch

    std::string input_line("$  ");
    // Create a buffer to store lines of text
    std::vector<std::string> lines = emptyScreen();
    lines.push_back(input_line); // Add input line


    // Insert word onto screen
    lines[0].replace(5, word().length(), word());

    int y = input_line_index, x = input_line.length() - 1;       // Cursor position

    bool run = true;
    while (run) {
        clear();            // Clear the screen
        for (size_t i = 0; i < lines.size(); ++i) {
            mvprintw(i, 0, "%s", lines[i].c_str());
        }
        move(y, x);        // Move cursor to correct position

        int ch = getch();  // Get user input

        switch (ch) {
            case KEY_BACKSPACE:
            case 127:       // Handle backspace
                if (x > 0) {
                    lines[y].erase(x - 1, 1);
                    x--;
                } else if (y > 0) {
                    x = lines[y - 1].length();
                    lines[y - 1] += lines[y];
                    lines.erase(lines.begin() + y);
                    y--;
                }
                break;
            case '\n':      // Handle new line
                lines.insert(lines.begin() + y + 1, lines[y].substr(x));
                lines[y] = lines[y].substr(0, x);
                y++;
                x = 0;
                break;
            case KEY_LEFT:
                if (x > 0) {
                    x--;
                } else if (y > 0) {
                    y--;
                    x = lines[y].size();
                }
                break;
            case KEY_RIGHT:
                if (x < lines[y].size()) {
                    x++;
                } else if (y < lines.size() - 1) {
                    y++;
                    x = 0;
                }
                break;
            case KEY_UP:
                if (y > 0) {
                    y--;
                    x = std::min(x, (int)lines[y].size());
                }
                break;
            case KEY_DOWN:
                if (y < lines.size() - 1) {
                    y++;
                    x = std::min(x, (int)lines[y+1].size());
                }
                break;
            case 'u':
                run = false;
                break;
            default:
                lines[y].insert(x, 1, (char)ch);
                x++;
                break;
        }
    }

    // Clean up
    endwin();

    return 0;
}

