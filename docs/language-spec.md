# TUL-X Language Specification

**TUL-X** is a dynamically-typed scripting language with clean, C-style syntax designed to be simple, expressive, and pedagogically rich.

---

## 1. Lexical Grammar

### Comments
Single-line comments start with `//` and extend to the end of the line:
```tul
// This is a single-line comment in TUL-X
```

### Data Types & Literals
* **Numbers**: Double-precision floating point (e.g. `123`, `3.14159`).
* **Strings**: Double-quoted string literals with escape sequences (e.g. `"Hello, World!"`).
* **Booleans**: `true` and `false`.
* **Nil / Null**: `null` (representing the absence of value).

### Keywords
Reserved keywords in TUL-X:
`and`, `class`, `else`, `false`, `for`, `fun`, `if`, `null`, `or`, `print`, `return`, `super`, `this`, `true`, `var`, `while`.

---

## 2. Expressions & Operators

### Arithmetic & Unary
* Binary: `+`, `-`, `*`, `/`
* Unary: `-` (negation), `!` (logical NOT)

### Comparison & Equality
* Comparison: `<`, `<=`, `>`, `>=`
* Equality: `==`, `!=`

### Logical
* Logical AND: `and` (short-circuiting)
* Logical OR: `or` (short-circuiting)

### Grouping
* Parentheses `( ... )` for explicit precedence grouping.

---

## 3. Statements & Declarations

### Variable Declaration & Assignment
```tul
var name = "Sagar";
var count = 10;
count = count + 1;
```

### Print Statement
```tul
print "The result is:";
print count;
```

### Blocks & Lexical Scope
```tul
var a = "global";
{
    var a = "local";
    print a; // "local"
}
print a; // "global"
```

### Control Flow
```tul
if (x > 10) {
    print "large";
} else {
    print "small";
}

while (count > 0) {
    count = count - 1;
}

for (var i = 0; i < 5; i = i + 1) {
    print i;
}
```

### Functions & Closures
```tul
fun add(a, b) {
    return a + b;
}

fun makeCounter() {
    var c = 0;
    fun count() {
        c = c + 1;
        return c;
    }
    return count;
}
```

### Classes & Inheritance
```tul
class Animal {
    speak() {
        print "generic sound";
    }
}

class Dog < Animal {
    speak() {
        print "woof!";
    }
}
```
