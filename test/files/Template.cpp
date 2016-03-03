template <typename T> struct S { using Type = T; };
template <template <typename> class T> struct X { using Type = T<X>; };
