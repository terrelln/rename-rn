namespace x {
struct TP;
struct TP {
  TP();
  TP(const TP &);
  explicit TP(int TP) { TP; }
  ~TP();

  static TP foo(TP);

  struct X {
    int TP;
  };
};

TP x = TP::foo(TP());

}
struct TP {
  TP();
  ~TP();
};
namespace y {
using x::TP;
}
