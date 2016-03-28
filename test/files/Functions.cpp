struct Z {
  Z();
  int foo() const;
  ~Z();
};

int foo(bool x, bool y, bool z);

int foo(int x, int y, Z z = Z{}) {
  x = y = z.foo();
  z.~Z();
  return x + y + z.foo() + foo(x, y, z.foo());
}

template <typename T> void fn(T &&x);

template <typename T> void fn(T &&x) { x++; }
int main() { fn<int>(5); }

int Z::foo() const { return 42; }

struct X {
  int foo(bool, bool, bool);
  int bar() { return foo(false, false, false); }
};

