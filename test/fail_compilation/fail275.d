/*
TEST_OUTPUT:
---
fail_compilation/fail275.d(10): Error: circular reference to 'fail275.C.x'
---
*/

struct C
{
    const x = C.x;
}
