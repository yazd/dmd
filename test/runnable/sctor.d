// REQUIRED_ARGS:
// PERMUTE_ARGS: -w -d -de -dw

extern(C) int printf(const char*, ...);

/***************************************************/
// mutable field

struct S1A
{
    int v;
    this(int)
    {
        v = 1;
        v = 2;  // multiple initialization
    }
}

struct S1B
{
    int v;
    this(int)
    {
        if (true) v = 1; else v = 2;
        v = 3;  // multiple initialization
    }
    this(long)
    {
        if (true) v = 1;
        v = 3;  // multiple initialization
    }
    this(string)
    {
        if (true) {} else v = 2;
        v = 3;  // multiple initialization
    }
}

struct S1C
{
    int v;
    this(int)
    {
        true ? (v = 1) : (v = 2);
        v = 3;  // multiple initialization
    }
    this(long)
    {
        auto x = true ? (v = 1) : 2;
        v = 3;  // multiple initialization
    }
    this(string)
    {
        auto x = true ? 1 : (v = 2);
        v = 3;  // multiple initialization
    }
}

/***************************************************/
// with control flow

struct S2
{
    immutable int v;
    immutable int w;
    int x;
    this(int)
    {
        if (true) v = 1;
        else      v = 2;

        true ? (w = 1) : (w = 2);

        x = 1;  // initialization
    L:  x = 2;  // assignment after labels
    }
    this(long n)
    {
        if (n > 0)
            return;
        v = 1;  // skipped initialization

        // w skipped initialization

        x = 1;  // initialization
        foreach (i; 0..1) x = 2;  // assignment in loops
    }
}

/***************************************************/
// with immutable constructor

struct S3
{
    int v;
    int w;
    this(int) immutable
    {
        if (true) v = 1;
        else      v = 2;

        true ? (w = 1) : (w = 2);
    }
}

/***************************************************/
// in typeof

struct S4
{
    immutable int v;
    this(int)
    {
        static assert(is(typeof(v = 1)));
        v = 1;
    }
}

/***************************************************/
// 9665

struct X9665
{
    static uint count;
    ulong payload;
    this(int n) { payload = n; count += 1; }
    this(string s) immutable { payload = s.length; count += 10; }
    void opAssign(X9665 x) { payload = 100; count += 100; }
}

struct S9665
{
              X9665 mval;
    immutable X9665 ival;
    this(int n)
    {
        X9665.count = 0;
        mval = X9665(n);                // 1st, initializing
        ival = immutable X9665("hi");   // 1st, initializing
        mval = X9665(1);                // 2nd, assignment
        static assert(!__traits(compiles, ival = immutable X9665(1)));  // 2nd, assignment
        //printf("X9665.count = %d\n", X9665.count);
        assert(X9665.count == 112);
    }
    this(int[])
    {
        X9665.count = 0;
        mval = 1;       // 1st, initializing (implicit constructor call)
        ival = "hoo";   // ditto
        assert(X9665.count == 11);
    }
}

void test9665()
{
    S9665 s1 = S9665(1);
    assert(s1.mval.payload == 100);
    assert(s1.ival.payload == 2);

    S9665 s2 = S9665([]);
    assert(s2.mval.payload == 1);
    assert(s2.ival.payload == 3);
}

/***************************************************/
// 11246

struct Foo11246
{
    static int ctor = 0;
    static int dtor = 0;
    this(int i)
    {
        ++ctor;
    }

    ~this()
    {
        ++dtor;
    }
}

struct Bar11246
{
    Foo11246 foo;

    this(int)
    {
        foo = Foo11246(5);
        assert(Foo11246.ctor == 1);
        assert(Foo11246.dtor == 0);
    }
}

void test11246()
{
    {
        auto bar = Bar11246(1);
        assert(Foo11246.ctor == 1);
        assert(Foo11246.dtor == 0);
    }
    assert(Foo11246.ctor == 1);
    assert(Foo11246.dtor == 1);
}

/***************************************************/

int main()
{
    test9665();
    test11246();

    printf("Success\n");
    return 0;
}
