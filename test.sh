#!/bin/bash

assert() {
  expected="$1"
  input="$2"

  ./9cc "$input" t > tmp.s
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
assert 4 "int main() { int a; return sizeof(a); }"
assert 4 "int main() { int a; return sizeof a; }"
assert 3 "int main() { int a[10]; int b; b = 3; return b; }"
assert 3 "int main() { int a[2]; *a = 1; *(a + 1) = 2; int *p; p = a; return *p + *(p + 1); }"
assert 3 "int main() { int a[2]; int *p; p = a; *p = 1; *(p + 1) = 2; return *p + *(p + 1); }"
assert 3 "int main() { int a[2]; int *p; p = &a; *p = 1; *(p + 1) = 2; return *p + *(p + 1); }"
assert 3 "int main() { int a[2]; *a = 1; *(a + 1) = 2; return *a + *(a + 1); }"
assert 3 "int main() { int a[2]; a[0] = 1; a[1] = 2; return a[0] + a[1]; }"
assert 3 "int a; int main() { a = 3; return a; }"
assert 3 "int a; int foo() { a = 3; } int main() { foo(); return a; }"
assert 3 "int a[2]; int main() { a[0] = 1; a[1] = 2; return a[0] + a[1]; }"
assert 8 "int a[3]; int main() { a[0] = 3; a[1] = 1; a[2] = 5; return a[0] + a[1] * a[2]; }"
assert 3 "int main() { int x[3]; x[0] = -1; x[1] = 2; int y; y = 4; return x[0] + y; }"
assert 3 "int main() { char x[3]; x[0] = -1; x[1] = 2; int y; y = 4; return x[0] + y; }"
assert 3 "int main() { int x = 3; return x; }"
assert 3 "int x = 3; int main() { return x; }"
assert 3 "char x = 3; int main() { return x; }"
assert 14 "int x = 3 * 4 + 2; int main() { return x; }"

echo OK
