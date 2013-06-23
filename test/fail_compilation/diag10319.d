/*
TEST_OUTPUT:
---
fail_compilation/diag10319.d(25): Error: pure function 'D main' cannot call impure function 'diag10319.foo'
fail_compilation/diag10319.d(25): Error: safe function 'D main' cannot call system function 'diag10319.foo'
fail_compilation/diag10319.d(26): Error: pure function 'D main' cannot call impure function 'diag10319.bar!int.bar'
fail_compilation/diag10319.d(26): Error: safe function 'D main' cannot call system function 'diag10319.bar!int.bar'
fail_compilation/diag10319.d(25): Error: 'diag10319.foo' is not nothrow
fail_compilation/diag10319.d(26): Error: 'diag10319.bar!int.bar' is not nothrow
fail_compilation/diag10319.d(23): Error: function 'D main' is nothrow yet may throw
---
*/

void foo() {}

void bar(T)()
{
    static int g; g = 10;       // impure
    int x; auto p = &x;         // system
    throw new Exception("");    // may throw
}

@safe pure nothrow void main()  // L23
{
    foo();      // L25
    bar!int();  // L26
}
