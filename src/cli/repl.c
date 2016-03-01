
//
//  REPL
//

#include "repl.h"

#include <hydrogen.h>
#include <hylib.h>

#include <termios.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>

#include <vec.h>

#include "help.h"
#include "err.h"


// The prompt to print before every line typed in the console
#define PROMPT "> "

// Important keys
#define KEY_ESC       27
#define KEY_BACKSPACE 127
#define KEY_CTRL_C    3
#define KEY_CTRL_D    4

// Control codes
#define SEQ            "\033["
#define SEQ_LEFT       SEQ "%dD"
#define SEQ_RIGHT      SEQ "%dC"
#define SEQ_UP         SEQ "%dA"
#define SEQ_DOWN       SEQ "%dB"
#define SEQ_X_POS      SEQ "%dG"
#define SEQ_LINE_START "\r"
#define SEQ_CLEAR_LINE SEQ "2K"


// Save a copy of the original terminal state so we can disable raw mode easily
static struct termios original;


// The type of a line
typedef Vec(char) Line;


// Information required for reading a line of user input from stdin
typedef struct {
	// The row and column of the cursor. Since all movement is relative to the
	// starting location of the cursor, we use (0, 0) to represent this starting
	// location. The position is absolute (ie. doesn't depend on the scroll
	// offsets)
	uint32_t x, y;

	// The vertical and horizontal scroll offsets
	uint32_t scroll_x, scroll_y;

	// The contents of each line of the text field
	Vec(Line) lines;
} Input;


// What to do after a key is pressed
typedef enum {
	RESULT_FINISH,
	RESULT_TERMINATE,
	RESULT_OK,
} InputResult;



//
//  Input Reading
//

// Moves the cursor left by `amount` cells
static void input_left(Input *input, uint32_t amount) {
	if (input->x > 0) {
		printf(SEQ_LEFT, amount);
		input->x--;
	}
}


// Moves the cursor right by `amount` cells
static void input_right(Input *input, uint32_t amount) {
	Line *line = &vec_at(input->lines, input->y);
	if (input->x < vec_len(*line)) {
		printf(SEQ_RIGHT, amount);
		input->x++;
	}
}


// Moves the cursor up by `amount` rows
static void input_up(Input *input, uint32_t amount) {
	if (input->y > 0) {
		printf(SEQ_UP, amount);
		input->y--;
	}
}


// Moves the cursor down by `amount` rows
static void input_down(Input *input, uint32_t amount) {
	if (input->y < vec_len(input->lines) - 1) {
		printf(SEQ_DOWN, amount);
		input->y++;
	}
}


// Moves the cursor to the correct horizontal position on the line
static void input_restore_cursor(Input *input) {
	// The x coordinate is 1 indexed (for some strange reason)
	printf(SEQ_X_POS, input->x + (int) strlen(PROMPT) + 1);
}


// Clears the line the cursor is currently on. Moves the cursor to the start of
// the line
static void input_clear_line(void) {
	printf(SEQ_CLEAR_LINE SEQ_LINE_START);
}


// Redraws the input on the current terminal line
static void input_draw_line(Input *input) {
	// Clear the line
	input_clear_line();

	if (input->y == 0) {
		// If this is the first line, write the prompt
		fputs(PROMPT, stdout);
	} else {
		// Otherwise write spaces up to the length of the prompt
		for (uint32_t i = 0; i < strlen(PROMPT); i++) {
			putc(' ', stdout);
		}
	}

	// Write the line itself
	Line *line = &vec_at(input->lines, input->y);
	if (vec_len(*line) > 0) {
		for (uint32_t i = 0; i < vec_len(*line); i++) {
			putc(vec_at(*line, i), stdout);
		}
	}

	// Move the cursor to the correct position on the line
	input_restore_cursor(input);
}


// Backspaces the character behind the cursor location
static void input_backspace(Input *input) {
	if (input->x > 0) {
		Line *line = &vec_at(input->lines, input->y);
		vec_remove(*line, input->x - 1);
		input->x--;
		input_draw_line(input);
	} else {
		// Merge this line and the previous one
		// TODO
	}
}


// Forward deletes the character in front of the cursor
static void input_delete(Input *input) {
	Line *line = &vec_at(input->lines, input->y);
	vec_remove(*line, input->x);
}


// Insert a newline at the current cursor location
static void input_newline(Input *input) {
	// TODO
}


// Insert a character at the current cursor location
static void input_insert(Input *input, char ch) {
	Line *line = &vec_at(input->lines, input->y);
	vec_insert(*line, input->x, ch);
	input->x++;
	input_draw_line(input);
}


// Handles an escape sequence
static void input_esc(Input *input, char ch) {
	switch (ch) {
	case 'A':
		// Up arrow key
		input_up(input, 1);
		break;
	case 'B':
		// Down arrow key
		input_down(input, 1);
		break;
	case 'C':
		// Right arrow key
		input_right(input, 1);
		break;
	case 'D':
		// Left arrow key
		input_left(input, 1);
		break;
	case 'H':
		// Home key
		input->x = 0;
		input_restore_cursor(input);
		break;
	case 'F':
		// End key
		input->x = vec_len(vec_at(input->lines, input->y));
		input_restore_cursor(input);
		break;
	}
}


// Handles an extended escape sequence
static void input_extended_esc(Input *input, char ch) {
	switch (ch) {
	case '3':
		// Forward delete key
		input_delete(input);
		break;
	}
}


// Read the next character from the input. Returns false if we should stop
// reading subsequent characters
static InputResult input_next(Input *input) {
	int ch = fgetc(stdin);

	// Check for an escape sequence
	if (ch == KEY_ESC) {
		// Escape key
		ch = fgetc(stdin);
		if (ch == '[' || ch == '0') {
			// Handle the escape sequence
			ch = fgetc(stdin);
			input_esc(input, (char) ch);
		} else if (ch >= '0' && ch <= '9') {
			// Extended escape sequence, check the next byte is a tilde
			if (fgetc(stdin) != '~') {
				return true;
			}

			// Handle extended escape sequence
			input_extended_esc(input, (char) ch);
		}
	} else if (ch == KEY_CTRL_C) {
		// Terminate the process
		return RESULT_TERMINATE;
	} else if (ch == '\n' || ch == '\r' || ch == KEY_CTRL_D) {
		// Stop reading input
		return RESULT_FINISH;
	} else if (ch == KEY_BACKSPACE) {
		// Backspace key
		input_backspace(input);
	} else {
		// Insert character
		input_insert(input, ch);
	}

	return RESULT_OK;
}


// Disable raw mode to return to normal behaviour
static void input_disable_raw(void) {
	// Reset the terminal state to the originally saved one
	tcsetattr(fileno(stdin), TCSAFLUSH, &original);
}


// Enable raw mode for unbuffered reading from stdin. Returns true if we could
// successfully put the terminal in raw mode
static bool input_enable_raw(void) {
	// Only enable raw mode on terminals
	int descriptor = fileno(stdin);
	if (!isatty(descriptor)) {
		return false;
	}

	// We need to disable raw mode on exit
	atexit(input_disable_raw);

	// Save the original terminal interface
	if (tcgetattr(descriptor, &original) < 0) {
		return false;
	}

	// Copy the original terminal interface and modify it
	struct termios raw = original;
	raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
	raw.c_oflag &= ~(OPOST);
	raw.c_cflag |= (CS8);
	raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
	raw.c_cc[VMIN] = 1;
	raw.c_cc[VTIME] = 0;

	// Flush the terminal
	if (tcsetattr(descriptor, TCSAFLUSH, &raw) < 0) {
		return false;
	}

	return true;
}


// Combines all input lines into a single string
static char * input_combine_lines(Input *input) {
	// Get the total length of every line
	uint32_t total = 0;
	for (uint32_t i = 0; i < vec_len(input->lines); i++) {
		// Add 1 for the newline at end of every line
		total += vec_len(vec_at(input->lines, i)) + 1;
	}

	// Combine all lines into a single string
	char *result = malloc(total + 1);
	char *end = result;
	for (uint32_t i = 0; i < vec_len(input->lines); i++) {
		// Append the line to the result
		char *line = &vec_at(vec_at(input->lines, i), 0);
		uint32_t length = vec_len(vec_at(input->lines, i));
		strncpy(end, line, length);
		end[length] = '\n';
		end = &end[length + 1];
	}

	// Add a NULL terminator
	*end = '\0';
	return result;
}


// Create a new input struct
static Input input_new(void) {
	Input input;
	input.x = 0;
	input.y = 0;
	input.scroll_x = 0;
	input.scroll_y = 0;
	vec_new(input.lines, Line, 4);

	// Add at least 1 line
	vec_inc(input.lines);
	Line *line = &vec_last(input.lines);
	vec_new(*line, char, 64);
	return input;
}


// Read from the standard input
static char * input_read(void) {
	// Create an input value
	Input input = input_new();

	// Enable raw mode for unbuffered reading
	if (!input_enable_raw()) {
		return NULL;
	}

	// Read characters
	InputResult result_type;
	input_draw_line(&input);
	while (true) {
		result_type = input_next(&input);
		if (result_type != RESULT_OK) {
			break;
		}
	}

	// Disable raw mode
	input_disable_raw();

	char *result;
	if (result_type == RESULT_FINISH) {
		// Combine all input lines into a single string
		result = input_combine_lines(&input);

		// Print a final newline
		putc('\n', stdout);
	} else {
		result = NULL;
	}

	// Free lines
	for (uint32_t i = 0; i < vec_len(input.lines); i++) {
		vec_free(vec_at(input.lines, i));
	}
	vec_free(input.lines);
	return result;
}



//
//  Execution
//

// Run the REPL
void repl(Config *config) {
	// Print version information
	print_version();

	// Create interpreter state
	HyState *state = hy_new();
	hy_add_libs(state);
	HyPackage pkg = hy_add_pkg(state, NULL);

	while (true) {
		// Read input
		char *input = input_read();
		if (input == NULL) {
			// Exit the program
			break;
		}

		// Execute input
		HyError *err;
		if (config->show_bytecode) {
			err = hy_print_bytecode_string(state, pkg, input);
		} else {
			err = hy_pkg_run_string(state, pkg, input);
		}

		// Print the error if necessary
		if (err != NULL) {
			print_err(err);
			hy_err_free(err);
		}

		free(input);
	}

	hy_free(state);
}
