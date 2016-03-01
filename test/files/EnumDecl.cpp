enum E { A, B, C };
enum class F;
enum class F { X, Y, Z };

enum class G;

int a = E::A;
E b = E::B;
auto c = C;

int x = static_cast<int>(F::X);
F y = F::Y;
auto z = F::Z;

using E::A;
using F::X;
