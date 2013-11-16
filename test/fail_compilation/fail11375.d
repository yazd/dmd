/*
TEST_OUTPUT:
---
fail_compilation/fail11375.d(17): Error: constructor this is not nothrow
fail_compilation/fail11375.d(15): Error: function 'D main' is nothrow yet may throw
---
*/

class B {
    this() {}
}

class D() : B {}

void main() nothrow
{
    auto d = new D!()();
}
