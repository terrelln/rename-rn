namespace n {
void fn(int);
void fn(bool);
}

using n::fn;
int main() {
  fn(5);
  fn(true);
}
