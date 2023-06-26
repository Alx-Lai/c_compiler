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
- keyword return
- support compound statements for keyword if-else
- support pure compound statements
- **haven't** support `,`, `++` and `--`
### Example
```
int main() {
  int a = 2; // declare variables
  int b = 1;
  a += 2;
  a = a + b;
  a || (a = 2) || (b = 4);
  return a;
}
```

## Known Bugs
- Memory Leak (To be fixed with further code refactor)


## Reference
https://norasandler.com/2017/11/29/Write-a-Compiler.html
https://github.com/wbowling/rcc
https://github.com/jserv/MazuCC
https://en.cppreference.com/w/c/11
