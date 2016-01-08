
//
//  Import Parsing
//

#include <string.h>

#include "import.h"
#include "../bytecode.h"


// Returns the position of the last occurrence of the given character.
int last_occurrence(char *string, char ch) {
	int last = strlen(string) - 1;
	while (last >= 0 && string[last] != ch) {
		last--;
	}
	return last;
}


// Returns the position of the path separator (`/`) that begins the final
// path component in a filesystem path, or -1 if one doesn't exist.
int last_path_component(char *path) {
	return last_occurrence(path, '/');
}


// Resolves the path for a package. If the given path is absolute, or the
// importing package is not a file, then the path is
char * import_package_path(Package *importer, char *path) {
	// Find the position of the last path component of the importing
	// package's path
	int last = importer->file != NULL ? last_path_component(importer->file) : -1;

	// Copy the path across if it's absolute (begins with `/`), or the path
	// is relative to the current directory (rather than the importing package)
	if (path[0] == '/' || last == -1) {
		return path;
	} else {
		// Append the path to the importing package's path (excluding the last
		// path component of the importing package's path)
		char *result = (char *) malloc(sizeof(char) * (strlen(path) + last + 2));
		strncpy(result, importer->file, last + 1);
		strcpy(&result[last + 1], path);
		return result;
	}
}


// Evaluates to true if `ch` is a valid character in an import path.
#define IS_VALID_PATH_CHARACTER(ch)                          \
	((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') || \
	 (ch >= '0' && ch <= '9') || ch == '_' || ch == '/' || ch == '.')


// Validates a package path, returning true if it is valid.
bool import_path_is_valid(char *path) {
	// Check every character in the path is valid
	size_t length;
	for (length = 0; path[length] != '\0'; length++) {
		if (!IS_VALID_PATH_CHARACTER(path[length])) {
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
	for (size_t i = 0; i < length; i++) {
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


// Extracts a package's actual name from its provided path.
char * import_package_name(char *path) {
	// Find the last path component
	size_t length = strlen(path);
	int last_path = last_path_component(path);

	// Find the last `.` for the file extension
	int last_dot = last_occurrence(path, '.');
	if (last_dot < last_path) {
		// The dot is before the final path component, so there's no file
		// extension
		last_dot = -1;
	}

	char *name;
	if (last_path < 0 && last_dot < 0) {
		// Copy into a new string
		name = (char *) malloc(sizeof(char) * (length + 1));
		strcpy(name, path);
	} else if (last_path < 0) {
		// File extension
		name = (char *) malloc(sizeof(char) * last_dot);
		strncpy(name, path, last_dot);
		name[last_dot] = '\0';
	} else {
		// Stop before the file extension if one exists
		size_t stop = length;
		if (last_dot >= 0) {
			stop = last_dot;
		}

		// Extract the last component into a new string
		// The length includes the + 1 for the NULL terminator
		size_t last_length = length - last_path;
		name = (char *) malloc(sizeof(char) * last_length);
		for (size_t i = last_path + 1; i < stop; i++) {
			name[i - (last_path + 1)] = path[i];
		}
		name[last_length - 1] = '\0';
	}

	return name;
}


// Loads an external package.
uint32_t import_user(Parser *parser, char *path, char *name) {
	Function *fn = &parser->vm->functions[parser->fn_index];

	// Find the requested package
	char *actual_path = import_package_path(fn->package, path);
	if (actual_path != path) {
		free(path);
	}

	uint32_t index;
	Package *package = package_new(parser->vm, &index);
	package->name = name;
	package->file = actual_path;
	package->source = read_file(actual_path);

	// Check we could actually read the file
	if (package->source == NULL) {
		ERROR("Failed to find package `%s`", name);
		return 0;
	}

	// Compile the package
	parse_package(parser->vm, package);

	// Call the package's main function
	emit(fn, instr_new_4(CALL_F, 0, package->main_fn, 0, 0));

	return index;
}


// Searches for an imported package in the parser with the given name,
// returning NULL if the package couldn't be found.
Import * import_package_find(Parser *parser, char *name, size_t length) {
	Imports *imports = parser->imports;
	VirtualMachine *vm = parser->vm;

	for (uint32_t i = 0; i < imports->imports_count; i++) {
		Import *import = &imports->imports[i];
		if (import->type == IMPORT_USER) {
			// Check a user package
			Package *package = &vm->packages[import->index];
			if (package->name != NULL && length == strlen(package->name) &&
					strncmp(package->name, name, length) == 0) {
				return import;
			}
		} else {
			// Check a native package
			HyNativePackage *package = &vm->native_packages[import->index];
			if (length == strlen(package->name) &&
					strncmp(package->name, name, length) == 0) {
				return import;
			}
		}
	}

	// Not found
	return NULL;
}


// Imports a native package with the given index.
void import_new(Parser *parser, ImportType type, int import_index) {
	Imports *imports = parser->imports;
	int index = imports->imports_count++;
	ARRAY_REALLOC(imports->imports, Import);
	Import *import = &imports->imports[index];
	import->type = type;
	import->index = import_index;
}


// Imports a package with the given path and name.
void import(Parser *parser, char *path) {
	// Ensure the path is valid
	if (!import_path_is_valid(path)) {
		free(path);
		ERROR("Invalid package path `%s`", path);
		return;
	}

	char *name = import_package_name(path);

	// Check if the package has already been imported
	Import *found = import_package_find(parser, name, strlen(name));
	if (found != NULL) {
		// Already imported
		free(path);
		free(name);
		ERROR("Package `%s` already imported", name);
		return;
	}

	// Check if we're importing a native package
	int native = native_package_find(parser->vm, path, strlen(path));
	if (native >= 0) {
		import_new(parser, IMPORT_NATIVE, native);
		free(path);
		free(name);
		return;
	}

	// Check if the package has already been loaded
	int import = -1;
	for (uint32_t i = 0; i < parser->vm->packages_count; i++) {
		Package *package = &parser->vm->packages[i];
		if (strcmp(package->name, name) == 0) {
			// Package has already been loaded
			import = i;
			break;
		}
	}

	// If the package hasn't already been loaded, load it
	if (import == -1) {
		import = import_user(parser, path, name);
	} else {
		free(name);
		free(path);
	}

	// Add the imported package to the imports list
	import_new(parser, IMPORT_USER, import);
}


// Parses a multi-import statement.
void parse_multi_import(Parser *parser) {
	Lexer *lexer = parser->lexer;

	// Ensure there's at least one string within the parentheses
	EXPECT(TOKEN_STRING, "Expected package name after `(`");

	// Expect a comma separated list of strings
	while (lexer->token.type == TOKEN_STRING) {
		// Extract the name of the package
		char *path = lexer_extract_string(lexer, &lexer->token);
		import(parser, path);

		// Consume the string
		lexer_next(lexer);

		// Expect a comma
		EXPECT(TOKEN_COMMA, "Expected `,` after package name");
		lexer_next(lexer);
	}

	// Expect a close parenthesis
	EXPECT(TOKEN_CLOSE_PARENTHESIS, "Expected `)` to close import list");
	lexer_next(lexer);
}


// Parses a single import statement.
void parse_import(Parser *parser) {
	Lexer *lexer = parser->lexer;

	// Parse a multi-import statement if the next token is an open parenthesis
	if (lexer->token.type == TOKEN_OPEN_PARENTHESIS) {
		// Consume the open parenthesis
		lexer_next(lexer);

		// Multi-import statement
		parse_multi_import(parser);
	} else if (lexer->token.type == TOKEN_STRING) {
		// Single import statement
		char *path = lexer_extract_string(lexer, &lexer->token);
		import(parser, path);

		// Consume the string token
		lexer_next(lexer);
	} else {
		// Unexpected token
		UNEXPECTED("Expected package name or `(` after `import`");
	}
}


// Parses a list of import statements at the top of a file.
void parse_imports(Parser *parser) {
	Lexer *lexer = parser->lexer;

	// Continually parse import statements
	while (lexer->token.type == TOKEN_IMPORT) {
		// Consume the `import`
		lexer_next(lexer);

		// Parse the rest of the import
		parse_import(parser);
	}
}
