int foo(int x, double y);

int foo(int x, double);

int foo(int y, double x);
int foo(int, double x);

int foo(int x, double y) { return 2*x;}

int foo(double x, double y);

int main() {
  int x;
  double y;
  foo(5, 6.0);
  foo(x, y);
  foo(y, y);
}
