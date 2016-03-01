namespace x {
namespace x {
int z;
struct X {};
}
int z;
struct X {};
}
using namespace x::x;
using Y = ::x::X;
using x::x::z;
namespace t = x;
namespace u = x::x;
namespace v = t::x;
namespace w = ::v;
using namespace ::u;
using namespace v;
using namespace w;

::Y y = t::X{};
x::X xx;
x::x::X xxx = ::x::x::X{};

void foo() {
  ::z = t::z = ::u::z = v::z = ::w::z = 5;
  ::x::z = 0;
}
