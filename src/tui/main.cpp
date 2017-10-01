#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include <curses.h>

#include <rosegarden-version.h>

static void help (const char* app_name)
{
    printf(
        "Usage: %s"
        , app_name
    );
}

class Command
{
    public:
        virtual ~Command ();
        virtual const char* describe () { return ""; }
        virtual void undo () = 0;
};

class Status
{
    private:
        char buffer [256];

    public:
        Status () {
            this->buffer[0] = 0;
        }

        int printf (const char* format, ...) {
            va_list va;
            va_start(va, format);
            int n_char_written = vsnprintf(this->buffer, sizeof(this->buffer), format, va);
            va_end(va);
            return n_char_written;
        }

        const char* unwrap () const {
            return this->buffer;
        }
};

static WINDOW* init_ncurses (void) {
    WINDOW* window = initscr();
    cbreak();
    noecho();
    return window;
}

namespace td {
    class Element {
        public:
            virtual ~Element () {}
            virtual bool is_pair () const { return false; }
            virtual bool is_text () const { return false; }
            virtual const char* text () const { abort(); }
            virtual const Element* first () const { abort(); }
            virtual const Element* second () const { abort(); }
    };
    class Text : Element {
        private:
            const char* str;
        public:
            Text (const char* str) {
                assert(str != NULL);
                this->str = str;
            }
            bool is_text () const { return true; }
            const char* text () const { return this->str; }
    };
    class Pair : Element {
        private:
            const Element* a;
            const Element* b;
        public:
            Pair (const Element* a, const Element* b) {
                this->a = a;
                this->b = b;
            }
            bool is_pair () const { return true; }
            const Element* first () const { return this->a; }
            const Element* second () const { return this->b; }
    };
}

#undef getch

namespace nc {
    class Window {
        private:
            WINDOW* _window;
            bool _error;
            void check (int e) {
                if (e == ERR) { _error = true; }
            }

        public:
            Window (WINDOW* window) {
                _window = window;
                _error = false;
            }
            Window* clear_error () {
                _error = false;
                return this;
            }
            bool has_error () const {
                return _error;
            }
            Window* print (const char* str) {
                check(wprintw(_window, str));
                return this;
            }
            Window* printf (const char* format, ...) {
                char buffer [256];
                va_list va;
                va_start(va, format);
                int n_char_written = vsnprintf(buffer, sizeof(buffer), format, va);
                if ((n_char_written < 0) || (n_char_written >= (int) sizeof(buffer))) {
                    _error = true;
                } else {
                    this->print(buffer);
                }
                va_end(va);
                return this;
            }
            Window* set_keypad (bool flag) {
                check(keypad(_window, flag));
                return this;
            }
            Window* set_bold (bool flag) {
                check(
                    flag
                        ? wattron(_window, A_BOLD)
                        : wattroff(_window, A_BOLD)
                );
                return this;
            }
            Window* refresh () {
                check(wrefresh(_window));
                return this;
            }
            int getch () {
                const int c = wgetch(_window);
                check(c);
                return c;
            }
    };
}

int main (int argc, char** argv) {
    const char* app_name = (argc > 0) ? argv[0] : "rosegarden-tui";
    if (argc < 2) {
        help(app_name);
        return EXIT_SUCCESS;
    }
    const char* work_file = argv[1];
    nc::Window window_(init_ncurses());
    nc::Window* window = &window_;
    window
        ->set_keypad(true)
        ->set_bold(true)
        ->printf("Rosegarden %s\n\n", VERSION)
        ->set_bold(false)
        ->printf("Opening file \"%s\"\n", work_file)
        ->refresh();
    for (;;) {
        const int c = window->getch();
        if (c == 'q') {
            break;
        }
    }
    endwin();
    return EXIT_SUCCESS;
}

// vim: set nocindent:
