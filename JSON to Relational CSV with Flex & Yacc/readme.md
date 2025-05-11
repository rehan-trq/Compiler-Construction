# JSON-to-CSV Converter (Assignment 3)

A command-line tool that parses JSON input, constructs an Abstract Syntax Tree (AST), generates a relational schema, and outputs CSV files for each object level.

---

## Features

* **JSON Parsing**: Uses Bison (`parser.y`) and Flex (`scanner.l`) to tokenize and parse JSON.
* **AST Generation**: Builds an in-memory AST representation of the JSON document.
* **Schema Creation**: Infers a relational schema from the AST, including nested objects and arrays.
* **CSV Export**: Writes out one CSV file per table (per object type), with foreign keys linking nested elements.
* **AST Printing**: Optional `--print-ast` flag to visualize the AST in the console.

---

## Repository Structure

```
CC/A4/
├── parser.y          # Bison grammar for JSON → AST
├── scanner.l         # Flex lexer definitions
├── ast.h             # AST node and table definitions
├── ast.c             # AST, schema inference, CSV generation, memory cleanup
├── main.c            # CLI handling, orchestration
├── Makefile          # Build rules for parser, scanner, and binaries
├── README.md         # This documentation
└── test/             # Example JSON inputs and expected CSV outputs
```

---

## Prerequisites

* **Flex** (GNU Flex)
* **Bison** (GNU Bison)
* **GCC** (or another compatible C compiler)
* **Make**

On Debian/Ubuntu systems:

```bash
sudo apt update && sudo apt install -y flex bison build-essential
```

---

## Building the Project

```bash
cd CC/A4
make clean      # remove any old artifacts
make            # generate parser.c, scanner.c, and compile
```

After this, you should have:

* `parser.c`, `parser.h` (from Bison)
* `scanner.c` (from Flex)
* Object files: `main.o`, `ast.o`, `parser.o`, `scanner.o`
* Executable: `connectme` (if defined in your Makefile)

---

## Usage

Read JSON from standard input and write CSVs to `./output`:

```bash
./connectme --out-dir output < input.json
```

Options:

* `--print-ast` : Print the AST to stdout before generating CSVs.
* `--out-dir DIR` : Specify an output directory (default is current directory). Creates `DIR` if it doesn’t exist.

Example:

```bash
# Pretty-print AST and write CSVs
cat sample.json | ./connectme --print-ast --out-dir csv_out
```

---

## Examples

Inside `test/` you’ll find:

* `sample.json` : A simple JSON document with nested objects and arrays.
* `expected/` : Corresponding CSVs for each table (root.csv, child.csv, etc.).

To run against the sample:

```bash
./connectme --out-dir test/result < test/sample.json
ls test/result
```

Compare `test/result` with `test/expected`.

---

## Troubleshooting

* **Flex parse errors**: Ensure `scanner.l` has the `%option noyywrap`, `%option yylineno`, and proper `%%` separators.
* **Bison warnings**: Verify that semantic actions use `$$` and correct types in `%union`.
* **Compiler errors**: Confirm that `print_ast` is declared (forward declaration in `main.c`) and not shadowed by a variable.

---

## License

MIT License

---

*Assignment 3 for Compiler Construction (CC/A3)*

