
Standard Library
================


* `io`
	* `print(things...)`
	* `println(things...)`
	* `printf(format, arguments...)`


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
	* `new(path)`
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
	* `new(path)`
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
	* `new(path)`
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
	* `new(path)`
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
