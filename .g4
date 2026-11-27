int f()
  post(r : r == 1)
{
  return 1;
}
int i = f();    // Postcondition check succeeds.
— end example]
[Example 2: 
struct A {};
struct B {
  B() {}
  B(const B&) {}
};

template <typename T>
T f(T* const ptr)
  post(r: &r == ptr)
{
  return {};
}

int main() {
  A a = f(&a);  // The postcondition check can fail if the implementation introduces
                // a temporary for the return value ([class.temporary]).
  B b = f(&b);  // The postcondition check succeeds, no temporary is introduced.
}
— end example]
2
#
When the declared return type of a non-templated function contains a placeholder type, a postcondition-specifier with a result-name-introducer shall be present only on a definition.
[Example 3: 
auto g(auto&)
  post (r: r >= 0);     // OK, g is a template.

auto h()
  post (r: r >= 0);     // error: cannot name the return value

auto k()
  post (r: r >= 0)      // OK
{
  return 0;
}
— end 
