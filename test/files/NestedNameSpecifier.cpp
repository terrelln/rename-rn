namespace S {
struct A {
  union B {
    class C {
    public:
      enum class W { X };
      enum Y { Z };
    };
  };
  using D = B::C::W;
};
using E = A::B::C;
using F = A::B;
using A::B::C::W::X;
}
using S::A::B::C::Y::Z;
using G = S::A::B::C;
G::W enumx = S::X;
S::A::B::C c = S::A::B::C{};
S::A::B b = S::F{};
G::Y enumz = G::Y::Z;
