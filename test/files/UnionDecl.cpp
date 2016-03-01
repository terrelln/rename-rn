namespace x {
union  TP;
union  TP {
  TP();
  TP(const TP &);
  explicit TP(int TP) { TP; }
  ~TP();

  static TP foo(TP);

  union  X {
    int TP;
  };
};

TP x = TP::foo(TP());

}
union  TP {
  TP();
  ~TP();
};
namespace y {
using x::TP;
}
