#!/bin/bash

assert() {
  expected="$1"
  input="$2"

  ./9cc "$input" > tmp.s
  cc -o tmp tmp.s
  ./tmp
  actual="$?"

  if [ "$actual" = "$expected" ]; then
    echo "$input => $actual"
  else
    echo "$input => $expected expected, but got $actual"
    exit 1
  fi
}

assert 0 '0;'
assert 42 '42;'
assert 21 '5+20-4;'
assert 41 ' 12 + 34 - 5 ;'
assert 47 '5+6*7;'
assert 15 '5*(9-6);'
assert 4 '(3+5)/2;'
assert 10 '-10+20;'
assert 1 '10==10;'
assert 1 '10==(20 - 10);'
assert 0 '10==(20 - 9);'
assert 1 '10 != (20-9);'
assert 0 '10 != (20-10);'
assert 1 '10 > 9;'
assert 0 '10 > 10;'
assert 1 '10 >= 9;'
assert 1 '10 >= 10;'
assert 0 '10 >= 11;'
assert 1 '9 < 10;'
assert 0 '10 < 10;'
assert 1 '9 <= 10;'
assert 1 '10 <= 10;'
assert 0 '11 <= 10;'
assert 5 'a = 3; b = 2; a+b;'
assert 5 'a = 3; b = 2; c = a + b; return c;'
assert 7 'foo = 3; bar = 4; return foo + bar;'
assert 0 'if (10 > 1) return 0; else 1;'
assert 1 'if (10 < 1) return 0; else 1;'

echo OK
