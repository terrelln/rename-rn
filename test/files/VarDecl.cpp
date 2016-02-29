int x;
int y = 5;

int fn() {
  int x;
  x = 5;
  int y = x = x;
  return x;
}
