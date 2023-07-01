# Learning C compiler
Learning to build a multi-pass C compiler with [tutorial](https://norasandler.com/2017/11/29/Write-a-Compiler.html), including lexer, parser, and generating x64 assembly code


## Usage

```
make # compile with gcc
./main <file> # or ./run.sh <file> to check result
```

## Support syntax
- declare int variables and assignment of variables
- arithmetic expression including ternary, binary, and unary operator
- keyword `return`, `break`, `continue`
- support compound statements
- loops `for`, `while`, `do-while`
- `if-else` statement
- **haven't** support `,`, `++` and `--`
- define function with `int` return type and 0 to many `int` parameters 
- support call function recursively
### Example
```
int fib(int n) {
  if (n == 0 || n == 1) {
    return n;
  } else {
    return fib(n - 1) + fib(n - 2);
  }
}

int main() {
  int n = 6;
  return fib(n);
}
```

## Known Bugs
- Memory Leak (To be fixed with further code refactor)
- no check number of parameter while calling function


## Reference
https://norasandler.com/2017/11/29/Write-a-Compiler.html
https://github.com/wbowling/rcc
https://github.com/jserv/MazuCC
https://en.cppreference.com/w/c/11
