# TUL-X Technical Interview Cheat-Sheet & Notes

*Curated for technical interviews (Accenture, FAANG, and Systems Engineering).*

---

## 1. Fundamentals of Language Implementation

### Q1: What is the fundamental difference between a Compiler and an Interpreter?
* **Compiler**: Translates high-level source code into another language (usually machine code or bytecode) **before** execution. The resulting artifact runs independently of the compiler.
  * *Advantage*: Faster execution, ahead-of-time optimizations.
  * *Disadvantage*: Longer build phase, platform-dependent binaries.
* **Interpreter**: Reads source code (or intermediate representation) and executes the instructions **directly at runtime**.
  * *Advantage*: Rapid prototyping, dynamic typing, immediate execution, cross-platform portability.
  * *Disadvantage*: Slower execution due to interpretation overhead.

### Q2: What is a Just-In-Time (JIT) Compiler?
* A hybrid approach (used in JVM HotSpot, V8 JavaScript engine) that starts by interpreting code or running bytecode, profiles running code to find "hot spots" (frequently executed loops/functions), and compiles those hot spots to native machine code on-the-fly at runtime.

### Q3: What is the standard Compiler / Interpreter Pipeline?
```text
Source Code
   ↓ (Lexical Analysis)
Tokens (Token Stream)
   ↓ (Syntactic Analysis / Parsing)
Abstract Syntax Tree (AST)
   ↓ (Semantic Analysis / Type Checking)
Decorated AST / Intermediate Representation (IR)
   ↓
[Interpreter: Direct Tree-Walk Evaluation]
   OR
[Compiler: Optimization → Code Generation → Machine Code / Bytecode VM]
```

---

## 2. Lexical Analysis (Scanning / Lexing)

### Q4: What is the difference between a Lexeme and a Token?
* **Lexeme**: The raw substring of characters in the source code matching a pattern (e.g., `var`, `123.45`, `counter`).
* **Token**: The structured metadata representation of that lexeme produced by the scanner. It consists of:
  * A `TokenType` enum (e.g., `TOKEN_VAR`, `TOKEN_NUMBER`, `TOKEN_IDENTIFIER`).
  * Location information (line number, column).
  * Pointer to the lexeme source substring (`start`, `length`).
  * Converted runtime literal value (e.g. `double 123.45` or string pointer).

### Q5: Why is Lookahead necessary in a Lexer?
* Some character sequences are ambiguous with only 1 character:
  * `!` vs `!=` (Is it logical NOT or NOT EQUAL?)
  * `=` vs `==` (Assignment or equality check?)
  * `<` vs `<=` (Less than or less than equal?)
  * `123.` vs `123.45` (Number followed by method access vs floating point literal)
* Looking ahead one character (`peek()` or `peekNext()`) without advancing the cursor resolves this ambiguity deterministically (LL(1) / LL(k) lexical lookahead).

### Q6: How does the Scanner avoid unnecessary memory allocation in C?
* Instead of allocating a new heap string (`malloc`) for every single identifier or operator lexeme, `Token` stores a pointer directly into the original source code buffer (`const char* start`) alongside an `int length`.
* Heap allocation is only performed when necessary (e.g., parsed string literals stripped of quotes, dynamically converted numbers).

### Q7: How does Dynamic Array growth work in C?
* The token stream is stored in a dynamically resizing array (`Token* tokens`).
* When `count >= capacity`, capacity is grown geometrically (e.g. `capacity * 2`) using `realloc()`.
* **Amortized Analysis**: While a single resizing copy takes $O(N)$ time, doubling capacity ensures resizing happens exponentially less often, giving an **amortized $O(1)$** insertion time per token.

### Q8: How are Reserved Keywords distinguished from User Identifiers?
* When the scanner scans an identifier (e.g., `variable_name`), it first checks if the scanned string matches any reserved language keywords (`var`, `if`, `fun`, `class`, etc.).
* If a match is found in the keyword table (or Trie/prefix tree), it returns the specific keyword token type (e.g., `TOKEN_VAR`); otherwise, it returns `TOKEN_IDENTIFIER`.

---

## 3. Parsing & Syntax Trees

### Q9: What is the difference between a Parse Tree (CST) and an Abstract Syntax Tree (AST)?
* **Concrete Syntax Tree (CST / Parse Tree)**: A 1-to-1 representation of every concrete syntax element in the grammar, including punctuation, parentheses, commas, semicolons, and whitespace tokens.
* **Abstract Syntax Tree (AST)**: A pruned, simplified hierarchical tree representing only the structural and semantic meaning of the code (e.g., operator + operands), discarding trivial syntax markers like parentheses and semicolons.

### Q10: What is Recursive Descent Parsing?
* A **top-down parsing technique** where the grammar is structured such that each non-terminal grammar rule corresponds directly to a C function.
* As the parser descends through function calls corresponding to grammar precedence levels, it naturally handles nested expressions and precedence.

### Q11: How is Operator Precedence resolved in Recursive Descent?
* Grammar rules are ordered from **lowest precedence to highest precedence**:
  1. `expression` -> `equality` (`==`, `!=`)
  2. `equality` -> `comparison` (`<`, `<=`, `>`, `>=`)
  3. `comparison` -> `term` (`+`, `-`)
  4. `term` -> `factor` (`*`, `/`)
  5. `factor` -> `unary` (`!`, `-`)
  6. `unary` -> `primary` (numbers, strings, `(grouping)`)
* Lower precedence functions call higher precedence functions, ensuring operations with higher precedence are placed deeper in the AST and evaluated first.

---

## 4. C Programming & Memory Management

### Q12: Stack vs Heap Memory — What is the difference?
| Feature | Stack Memory | Heap Memory |
| :--- | :--- | :--- |
| **Allocation** | Automatic by CPU / compiler on function entry. | Manual via `malloc()`, `calloc()`, `realloc()`. |
| **Lifetime** | Limited to the enclosing stack frame scope. | Persists until explicitly `free()`d or GC'd. |
| **Speed** | Extremely fast (pointer decrement/increment). | Slower (search for free chunk, fragmentation). |
| **Size** | Small (typically 1-8 MB default stack limit). | Large (bounded by physical RAM and virtual memory). |

### Q13: What are Common C Memory Bugs and How Do We Prevent Them?
* **Memory Leak**: Allocating heap memory (`malloc`) without later releasing it (`free`).
* **Dangling Pointer**: Accessing a pointer after the memory it points to has been freed.
* **Double Free**: Calling `free()` twice on the same pointer.
* **Buffer Overflow / Out-of-Bounds**: Reading or writing past the allocated bounds of an array.
* *Mitigation*:
  * Strict ownership models (who allocates is responsible for freeing).
  * Nullify pointers immediately after `free(ptr); ptr = NULL;`.
  * Compile with AddressSanitizer (`-fsanitize=address,undefined`).

### Q14: What is a Tagged Union in C?
* A pattern combining an `enum` (the tag) and a `union` inside a `struct`.
* Because `union` members share the same memory space, the struct size is only as large as its largest member plus the enum tag.
* In TUL-X, `Expr` uses a tagged union (`ExprType` + `union { binary, unary, literal, grouping }`) to implement polymorphic AST nodes in pure C without class hierarchies.

### Q15: How is the Visitor Pattern implemented in C?
* In Java/C++, interfaces and virtual tables (`vtable`) handle dynamic dispatch.
* In C, we implement a `struct ExprVisitor` containing function pointers (`void* (*visitBinary)(Expr*)`, etc.) and a central dispatch function `exprAccept(Expr* expr, ExprVisitor* visitor)`.
