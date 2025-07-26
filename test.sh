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

assert 0 "main() { return 0; }"
assert 42 "main() { return 42; }"
assert 30 "foo() { return 30; } main() { return foo(); }"
assert 15 "foo(a, b) { return a + b; } main() { return foo(10, 5); }"
assert 25 "foo(a, b) { c = a + b; return c; } main() { c = foo(10, 5); return 10 + c; }"
assert 3 "main() { a = 3; b = &a; return *b; }"

echo OK
