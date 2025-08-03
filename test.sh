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
assert 3 "int main() { int x; int y; int *z; z = &x; z = z - 1; *z = 3; return y; }"
assert 8 "int main() { int a; return sizeof(a); }"
assert 8 "int main() { int a; return sizeof a; }"
assert 3 "int main() { int a[10]; int b; b = 3; return b; }"
assert 3 "int main() { int a; int b; int *c; c = &b; *c = 1; *(c + 1) = 2; return a + b; }"
assert 3 "int main() { int a[2]; *a = 1; *(a + 1) = 2; int *p; p = a; return *p + *(p + 1); }"
assert 3 "int main() { int a[2]; int *p; p = a; *p = 1; *(p + 1) = 2; return *p + *(p + 1); }"
assert 3 "int main() { int a[2]; int *p; p = &a; *p = 1; *(p + 1) = 2; return *p + *(p + 1); }"
assert 3 "int main() { int a[2]; *a = 1; *(a + 1) = 2; return *a + *(a + 1); }"
assert 3 "int main() { int a[2]; a[0] = 1; a[1] = 2; return a[0] + a[1]; }"

echo OK
