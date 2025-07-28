#!/bin/bash

assert() {
  expected="$1"
  input="$2"

  ./9cc "$input" > tmp.s
  cc -g -o tmp tmp.s
  ./tmp
  actual="$?"

  if [ "$actual" = "$expected" ]; then
    echo "$input => $actual"
  else
    echo "$input => $expected expected, but got $actual"
    exit 1
  fi
}

assert 0 "int main() { return 0; }"
assert 42 "int main() { return 42; }"
assert 30 "int foo() { return 30; } int main() { return foo(); }"
assert 15 "int foo(int a, int b) { return a + b; } int main() { return foo(10, 5); }"
assert 25 "int foo(int a, int b) { int c; c = a + b; return c; } int main() { int c; c = foo(10, 5); return 10 + c; }"
assert 3 "int main() { int a; a = 3; int *b; b = &a; return *b; }"
assert 3 "int main() { int x; int *y; y = &x; *y = 3; return x; }"

echo OK
