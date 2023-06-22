# Learning C compiler
Learning to build a small c compiler with tutorial on https://norasandler.com/2017/11/29/Write-a-Compiler.html


## Usage

```
make # compile with gcc
./main <file>
```

## Support syntax
- declare int variables
- arithmetic expression and assignment
- keyword return
### Example
```
int main() {
  int a = 2; // declare variables
  int b = 1;
  a += 2;
  a = a + b;
  return a;
}
```

## Known Bugs
- [Work in Process] `!a = 2;` should have error in parsing stage, but is parsed into `!(a = 2);`, which is valid.
- [Work in Process] `a || (a = 2) || (a = 4);` is a valid statement, but may cause parsing error.



## Reference
https://norasandler.com/2017/11/29/Write-a-Compiler.html
https://github.com/wbowling/rcc
https://github.com/jserv/MazuCC
