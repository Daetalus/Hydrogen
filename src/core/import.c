
//
//  Imports
//

#include <string.h>

#include <vec.h>

#include "import.h"


// Returns the position of the last occurrence of the given character, or
// `NOT_FOUND` if the character can't be found
static Index last_occurrence(char *string, char ch) {
	int32_t last = strlen(string) - 1;
	while (last >= 0 && string[last] != ch) {
		last--;
	}
	return (Index) last;
}


// Returns a heap allocated string (that needs to be freed) containing the name
// of a package based off its file path
char * hy_pkg_name(char *path) {
	// Find the last path component
	uint32_t length = strlen(path);
	Index last_path = last_occurrence(path, '/');

	// Find the last `.` for the file extension
	Index last_dot = last_occurrence(path, '.');
	if (last_path != NOT_FOUND && last_dot < last_path) {
		// The dot is before the final path component, so there's no file
		// extension
		last_dot = NOT_FOUND;
	}

	char *name;
	if (last_path == NOT_FOUND && last_dot == NOT_FOUND) {
		// No path components and no file extension, the path itself is the name
		name = malloc(length + 1);
		strcpy(name, path);
	} else if (last_path == NOT_FOUND) {
		// File extension, but no path components
		name = malloc(last_dot + 1);
		strncpy(name, path, last_dot);
		name[last_dot] = '\0';
	} else {
		// Stop before the file extension if one exists
		uint32_t stop = length;
		if (last_dot != NOT_FOUND) {
			stop = last_dot;
		}

		// Extract the last component into a new string
		// The length includes the + 1 for the NULL terminator
		uint32_t last_length = stop - last_path;
		name = malloc(last_length);
		strncpy(name, &path[last_path + 1], stop - last_path - 1);
		name[last_length - 1] = '\0';
	}

	return name;
}



// Returns the final path to a package to import from the path to the parent
// package and the import path
char * import_pkg_path(char *parent, char *child) {
	// Find the position of the last path component of the importing
	// package's path
	uint32_t last = (parent != NULL) ? last_occurrence(parent, '/') : -1;

	// Copy the path across if it's absolute (begins with `/`), or the path
	// is relative to the current directory (rather than the importing package)
	if (child[0] == '/' || last == NOT_FOUND) {
		return child;
	} else {
		// Append the path to the importing package's path (excluding the last
		// path component of the importing package's path)
		char *result = (char *) malloc(strlen(child) + last + 2);
		strncpy(result, parent, last + 1);
		strcpy(&result[last + 1], child);
		return result;
	}
}


// Returns true if a character in an import path is valid
bool import_char_is_valid(char ch) {
	return ((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') ||
		(ch >= '0' && ch <= '9') || ch == '_' || ch == '/' || ch == '.');
}


// Returns true if an import path is valid
bool import_is_valid(char *path) {
	// Check every character in the path is valid
	uint32_t length;
	for (length = 0; path[length] != '\0'; length++) {
		if (!import_char_is_valid(path[length])) {
			return false;
		}
	}

	// Ensure the path contains at least one letter
	if (length == 0) {
		return false;
	}

	// Check the last character is not a slash or dot
	if (path[length - 1] == '/' || path[length - 1] == '.') {
		return false;
	}

	// Check the only location of dots in the path is to specify the parent
	// directory (ie. ../), and ensure we don't have empty path components
	for (uint32_t i = 0; i < length; i++) {
		if (path[i] == '/' && path[i + 1] == '.') {
			// Expect `/../`
			if (i > length - 3 || path[i + 2] != '.' || path[i + 3] != '/') {
				return false;
			}
			i += 2;
		} else if (i == 0 && path[i] == '.') {
			// Expect `../`
			if (i > length - 2 || path[i + 1] != '.' || path[i + 2] != '/') {
				return false;
			}
			i++;
		} else if (path[i] == '.' || (path[i] == '/' && path[i + 1] == '/')) {
			return false;
		}
	}

	return true;
}
