<div align="center">
  <h1>Tul-X</h1>
  <p>A lightweight interpreter-based programming language built in C.</p>
  <p><i>Demonstrating core concepts of compiler & interpreter design from scratch.</i></p>

  <br/>

  ![C](https://img.shields.io/badge/C-00599C?style=flat-square&logo=c&logoColor=white)
  ![Interpreter](https://img.shields.io/badge/Interpreter-Design-black?style=flat-square)
  ![AST](https://img.shields.io/badge/AST-Generation-6e40c9?style=flat-square)
  ![Status](https://img.shields.io/badge/Status-Active-brightgreen?style=flat-square)
</div>

---

## What is Tul-X?

Tul-X is a handcrafted programming language interpreter written in pure C. It walks through every stage of language processing — from raw source text all the way to evaluated output — making it a clean, educational implementation of how real languages work under the hood.

---

## How It Works

```
Source Code  ──▶  Lexer  ──▶  Tokens  ──▶  Parser  ──▶  AST  ──▶  Interpreter  ──▶  Output
```

| Step | Component | Description |
|------|-----------|-------------|
| 1 | **Scanner / Lexer** | Converts raw source code into a stream of tokens |
| 2 | **Parser** | Builds an Abstract Syntax Tree using recursive descent |
| 3 | **Interpreter** | Walks and evaluates the AST node by node |
| 4 | **Output** | Prints the final evaluated result |

---

## Features

<table>
  <tr>
    <td valign="top" width="50%">
      <h3>🔍 Lexical Analysis</h3>
      Tokenizes raw source code into meaningful symbols for the parser to consume.
    </td>
    <td valign="top" width="50%">
      <h3>🌳 AST Generation</h3>
      Builds a structured Abstract Syntax Tree that represents program logic hierarchically.
    </td>
  </tr>
  <tr>
    <td valign="top" width="50%">
      <h3>⚙️ Recursive Descent Parser</h3>
      Parses tokens using a hand-written recursive descent strategy — no external tools.
    </td>
    <td valign="top" width="50%">
      <h3>🧮 Expression Evaluation</h3>
      Evaluates expressions with correct operator precedence and associativity.
    </td>
  </tr>
  <tr>
    <td valign="top" width="50%">
      <h3>🧱 Modular Architecture</h3>
      Clean separation between scanning, parsing, and evaluation layers.
    </td>
    <td valign="top" width="50%">
      <h3>🚨 Error Handling</h3>
      Basic error detection and reporting during lexing and parsing stages.
    </td>
  </tr>
</table>

---

## Tech Stack

| | |
|---|---|
| **Language** | C |
| **Core Concepts** | Compiler Design · Interpreter · Recursive Descent Parsing · AST |

---

<div align="center">
  <sub>Built from scratch in C · No external libraries · Pure compiler theory in practice</sub>
</div>
