
Standard Library
================


# Table of Contents

* `io`
* `fs`
* `str`
* `math`
* `iter`
* `collections`
* `net`
* `regex`


## `io`

* `print(things...)`
	* Prints any value to the standard output. Does not append a system line ending to the end of the string.
	* Numbers are converted to strings, and any objects have their `to_string` method called, if it exists. If not, then a textual representation is printed.
* `println(things...)`
	* Prints the values to the standard output, followed by a system line ending.
* `printf(format, arguments...)`
	* Prints a formatted string, similar to the `printf` function in C. Format contains modifiers (such as `%s`) which will be replaced by their corresponding argument in the arguments list.
	* Supported modifiers include:
		* `%s` - prints the string representation of a value.
		* `%f` - prints a number.
		* `%x` - prints a number in lowercase hexadecimal.
		* `%X` - prints a number in uppercase hexadecimal.
		* `%.nf` - prints a number to `n` decimal places, where `n` is any number.
		* `%p` - prints the location in memory of an object or value.


## `fs`

* `copy(from, to)`
	* Copies the file at `from` to the file at `to`, or into the directory at `to`, if it is a directory.
	* Overrides the file at `to` if it already exists.
	* Returns `nil` on success, or `io.Error` on failure.
* `move(from, to)`
	* Moves the file at `from` to the file at `to`, or into the directory at `to`, if it is a directory.
	* Overrides the file at `to` if it already exists.
	* Returns `nil` on success, or `io.Error` on failure.
* `delete(path)`
	* Deletes the file at `path`, returning an error if no file exists.
	* Returns `nil` on success, or `io.Error` on failure.
* `read(path)`
	* Returns contents of file at `path`.
	* Returns `io.Error` on failure.
* `write(path, contents)`
	* Writes `contents` to the file at `path`, creating the file if it doesn't exist and overriding it if it does.
	* Returns `nil` on success or `io.Error` on failure.
* `list(path)`
	* Returns an array of `fs.Path`s, representing the contents of the directory at `path`.
	* Returns `io.Error` on failure.

* Class `Reader`
	* `static file(path)`
		* Opens the file at `path` for reading.
		* Returns `File` on success, or `io.Error` on failure.
	* `all()`
		* Returns the entire contents of the file as a string.
		* Moves the file cursor to the end of the file.
		* Returns `io.Error` on failure.
	* `lines()`
		* Returns all lines of the file as an array of strings.
		* Moves the file cursor to the end of the file.
		* Returns `io.Error` on failure.
	* `len()`
		* Returns the total number of characters the file contains.
		* Does not affect the file cursor.
		* Returns `io.Error` on failure.
	* `read(count)`
		* Reads `count` number of characters, returning them as a string.
		* Moves the file cursor forward by `count` bytes.
		* Returns `io.Error` on failure.
	* `readln()`
		* Reads 1 line of the file, returning it as a string without the ending new line character.
		* Lines are delimited by `\n` or `\r`, or `\n\r`
		* Moves the file cursor forward.
		* Returns `io.Error` on failure.
	* `seek(offset)`
		* Moves the file cursor forward by `offset` characters (or backwards if `offset` is negative).
		* Returns `nil` on success, or `io.Error` on failure.
	* `close()`
		* Closes the file, preventing further reads.

* Class `BinaryReader`
	* `static file(path)`
		* Opens the file at `path` for binary reading.
		* Returns `File` on success, or `io.Error` on failure.
	* `all()`
		* Returns the entire contents of the file as a byte array.
		* Moves the file cursor to the end of the file.
		* Returns `io.Error` on failure.
	* `len()`
		* Returns the total number of bytes the file contains.
		* Does not affect the file cursor.
		* Returns `io.Error` on failure.
	* `read(count)`
		* Reads `count` number of bytes, returning them as a byte array.
		* Moves the file cursor forward by `count` bytes.
		* Returns `io.Error` on failure.
	* `seek(offset)`
		* Moves the file cursor forward by `offset` bytes (or backwards if `offset` is negative).
		* Returns `nil` on success, or `io.Error` on failure.
	* `close()`
		* Closes the file, preventing further reads.

* Class `Writer`
	* `static file(path)`
		* Opens the file at `path` for writing.
	* `write(object)`
		* Converts `object` to a string if it is not one already.
		* Writes `object` to the file at the current file cursor position.
		* Returns `nil` on success, or `io.Error` on failure.
	* `writeln(object)`
		* Converts `object` to a string, if it is not one already.
		* Writes `object` to the file, followed by the standard system line ending, at the current cursor position.
		* Returns `nil` on success, or `io.Error` on failure.
	* `seek(offset)`
		* Moves the file cursor forward by `offset` bytes (or backwards if `offset` is negative).
		* Returns `nil` on success, or `io.Error` on failure.
	* `close()`
		* Closes the file, preventing further writes.


* Class `BinaryWriter`
	* `static file(path)`
		* Opens the file at `path` for writing.
	* `write(object)`
		* Converts `object` to a string if it is not one already.
		* Writes `object` to the file at the current file cursor position.
		* Returns `nil` on success, or `io.Error` on failure.
	* `seek(offset)`
		* Moves the file cursor forward by `offset` bytes (or backwards if `offset` is negative).
		* Returns `nil` on success, or `io.Error` on failure.
	* `close()`
		* Closes the file, preventing further writes.


* Class `Path`
	* `static new(path)`
		* Returns a new path object.
	* `append(path)`
		* Appends `path` to the end of this path.
		* `path` can either be a string or another path object.
	* `components()`
		* Returns an iterator function over all path components in the path.
	* `file_name()`
		* Returns the file name, including extension.
	* `extension()`
		* Returns the file extension, or `nil` if one doesn't exist.
	* `enclosing_dir()`
		* Returns the parent directory of the file/directory specified by this path.


## `str`

* Class `String`
	* `len()`
		* Returns the number of characters in the string.
	* `push(str)`
		* Appends `str` to the end of this string.
	* `lower()`
		* Converts all uppercase letters to lowercase.
	* `upper()`
		* Converts all lowercase letters to uppercase.
	* `find(substring)`
		* Returns the character index of the first occurrence of `substring`, beginning the search from the left of the string.
		* If `substring` isn't found, then -1 is returned.
		* `item` can be another string, or a `regex.Regex` object.
	* `find_right(substring)`
		* Similar to `String.find`, but searching from the right of the string.
	* `substring(start, length)`
		* Returns a portion of the string, starting at the character index `start` and containing `length` characters.
		* If the substring extends beyond the bounds of the string, the function returns the part of the substring that lies within the bounds of the string.
			* Eg. the substring starting at 3 with length 100 of `"hello"` is `"lo"`.
		* If no part of the substring lies within the bounds of the string, then an empty string is returned.
			* Eg. the substring starting at -2 with length 1 of `"hello"` is `""`.
	* `letters()`
		* Returns an iterator function over all the letters in the string.
	* `words()`
		* Returns an iterator function over all the words in the string.
		* Words are considered to be separated by one or more whitespace characters.
	* `lines()`
		* Returns an iterator function over all the lines in the string, where lines are separated by `\n`, `\r`, or `\r\n`.
	* `bytes()`
		* Returns an array of the UTF-8 bytes for the string.
	* `replace(substring, replacement)`
		* Replaces all occurrences of `substring` with `replacement`.
		* If `substring` isn't found, then the string is left unchanged.
		* Returns the number of replacements made.
		* `substring` can be another string, or a `regex.Regex` object.
	* `replace(substring, replacement, count)`
		* Replaces up to `count` occurrences of `substring` with `replacement`.
		* Returns the number of replacements made, which may be `count` or less.
		* `substring` can be another string, or a `regex.Regex` object.
	* `contains(substring)`
		* Returns true if the string contains `substring`, or false otherwise.
		* `substring` can be another string, or a `regex.Regex` object.
	* `contains(substring, start)`
		* Returns true if the string contains `substring` after the character index specified by `start`.
		* `substring` can be another string, or a `regex.Regex` object.
	* `at(index)`
		* Returns the character found at `index`, or `nil` if no character exists at `index`.
	* `split(substring)`
		* Returns an array of substrings formed by splitting the string with the delimiter `substring`.
		* If `substring` isn't found, then an array containing a single item - the whole string - is returned.
		* `substring` can be another string, or a `regex.Regex` object.
	* `split(substring, count)`
		* Similar to `split(substring)`, but limits the number of splits in the string to `count`, leaving the remaining string as the last item in the array.
	* `split_right(substring, count)`
		* Similar to `split(substring, count)`, but starts the search for occurrences of splitting points from the right of the string.
	* `starts_with(string)`
		* Returns true if the string starts with `string`.
		* `string` can be another string, or a `regex.Regex` object.
	* `ends_with(string)`
		* Returns true if the string ends with `string`.
		* `string` can be another string, or a `regex.Regex` object.
	* `trim()`
		* Removes all whitespace from the start or end of the string.
	* `trim(characters)`
		* Removes all characters found in `characters` from the start or end of the string.
		* `characters` is either a string, array, or `regex.Regex` object.
	* `trim_left()`
		* Removes all whitespace from the left of the string.
	* `trim_left(characters)`
		* Similar to `trim(characters)`, but only from the start of the string.
	* `trim_right()`
		* Removes all whitespace from the end of the string.
	* `trim_right(characters)`
		* Similar to `trim(characters)`, but only from the end of the string.


## `math`

* `sin(value)`
* `cos(value)`
* `tan(value)`
* `asin(value)`
* `acos(value)`
* `atan(value)`
* `atan2(y, x)`
* `atan2(value)`
* `abs(value)`
* `floor(value)`
* `ceil(value)`
* `round(value)`
* `round_down(value)`
* `pow(value, exponent)`
* `exp(value, exponent)`
* `log2(value)`
* `log10(value)`
* `log(value, base)`
* `degrees(value)`
* `radians(value)`
* `sinh(value)`
* `cosh(value)`
* `tanh(value)`
* `asinh(value)`
* `acosh(value)`
* `atanh(value)`
* `sqrt(value)`
* `rand()`
* `rand(max)`
* `rand(min, max)`
* `rand_int()`
* `rand_int(max)`
* `rand_int(min, max)`


## `iter`

* `range(end)`
	* Returns an iterator function returning all numbers greater than or equal to 0, and less than `end`
* `range(start, end)`
	* Returns an iterator function returning all numbers greater than or equal to `start`, and less than `end`.
* `range(start, end, increment)`
	* Returns an iterator function returning numbers starting at `start` and going up by `increment`, stopping with the last number less than `end`.
* `collect(fn)`
	* Returns an array containing all items returned by the iterator function `fn`.
* `collect(fn, count)`
	* Returns an array containing `count` items returned by `fn`.


## `collections`

* Class `Array`
	* `len()`
		* Returns the number of items in the array.
	* `push(item)`
		* Appends `item` to the end of the array.
	* `insert(index, item)`
		* Inserts `item` at `index` in the array.
	* `pop()`
		* Removes the last item in the array, returning it.
	* `remove(index)`
		* Removes the item at `index`, returning it.
	* `clear()`
		* Removes all items in the array.
	* `is_empty()`
		* Returns true if the array contains no objects.
	* `contains(item)`
		* Returns true if the array contains `item`
	* `find(item)`
		* Returns the index of `item` in the array.
		* Returns -1 if the item couldn't be found.
	* `find_right(item)`
		* Similar to `find`, but starts the search from the right of the array.
	* `replace(item, replacement)`
		* Replaces all occurrences of `item` in the array with copies of `replacement`.
		* Returns the number of replacements made.
	* `replace(item, replacement, count)`
		* Replaces up to `count` occurrences of `item` in the array with copies of `replacement`.
		* Returns the number of replacements made, up to `count`.
	* `replace_right(item, replacement, count)`
		* Similar to `replace(item, replacement, count)`, but starting the replacements from the right end of the array.
	* `reverse()`
		* Reverses the order of the items in the array.
	* `sort()`
		* Sorts the items in the array.
	* `sort(fn)`
		* `fn` is a function taking 2 arguments - 2 items within the array. It should return true if the first item is less than the second, or false otherwise.
	* `remove_duplicates()`
		* Removes duplicate items from the list, keeping the first item of any duplicate sets.
	* `remove_duplicates(fn)`
		* Similar to `remove_duplicates()`, but uses the given function for equality testing.
		* `fn` takes 2 items from the list as arguments, returning true if they are equal, and false otherwise.

* Class `Dictionary`
	* `len()`
		* Returns the number of key/value pairs present in the dictionary.
	* `is_empty()`
		* Returns true if the dictionary contains no key/value pairs.
	* `contains(key)`
		* Returns true if `key` has an associated value.
	* `find(value)`
		* Returns the key associated with `value`.
	* `insert(key, value)`
		* Inserts the key/value pair.
	* `at(key)`
		* Returns the value associated with `key`.
	* `keys()`
		* Returns an iterator function over all keys in the dictionary.
	* `values()`
		* Returns an iterator function over all values in the dictionary.
	* `remove(key)`
		* Removes the value associated with `key`.
		* If `key` isn't in the dictionary, nothing will happen.
	* `clear()`
		* Removes all key/value pairs in the dictionary.


## `regex`

* Class `Regex`
	* `static new(expression)`
		* Returns a new regular expression object.
	* `captures(string)`
		* Returns an iterator function over the capture values generated by this regular expression when matched against `string`.

## `os`

* `platform()`
	* Returns a string representation of the platform the script is running on.
	* Can be one of `mac`, `window`, or `linux`.
* `current_dir()`
	* Returns an `fs.Path` of the current working directory.
* `home_dir()`
	* Returns the path to the user's home directory.
* `current_exe()`
	* Returns an `fs.Path` of the path to the executing script.
* `args()`
	* Returns an array of arguments passed into the script.
* `env_var(name)`
	* Returns the value of an environment variable.
* `exit()`
	* Stops the current process


## `process`

* `run(command)`
	* Execute a command, with its arguments determined by splitting the command string with whitespace.
	* Waits for the command to finish before returning.
* `run(command, args...)`
	* Execute a command, where its arguments are given as arguments to the function.
* `run(command, args)`
	* Execute a command, where its arguments are given as an array.

* Class `Process`
	* `static command(command, args...)`
		* Returns a new process.
	* `run()`
		* Starts the process.
	* `stdout()`
		* Returns an `io.Reader`, reading from the process' standard output stream.
	* `stderr()`
		* Returns an `io.Reader`, reading from the process' standard output stream.
	* `stdin()`
		* Returns an `io.Writer`, writing to the process' standard input stream.
	* `wait_for_completion()`
		* Halts the program until the process finishes.


## `time`

* `epoch()

* Class `Time`
	* `static new(time)`
		* Returns a time object based on `time`
		* If `time` is a number, it is treated as Unix epoch time in nanoseconds.
		* If `time` is a string, it is treated as an ISO 8601 time string.
	* `static now()`
		* Returns the current time.
	* `nanoseconds()`
	* `milliseconds()`
	* `seconds()`
	* `minutes()`
	* `hours()`


## `crypto`


## `compress`

* `gzip`


## `database`

* `sqlite`
* `mongodb`


## `reflect`


## `encoding`

* `base64`
* `json`
* `xml`
* `binary`
* `csv`


## `http`


## `testing`

* `assert`


## `image`


## `mime`


## `net`

* Class `TcpServer`
* Class `TcpStream`
* Class `UdpServer`
* Class `UdpStream`
