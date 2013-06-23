// PERMUTE_ARGS:
// REQUIRED_ARGS: -c -Icompilable/extra-files

void test1()
{
    import pkgDIP37.datetime;
    def();
    pkgDIP37.datetime.def();
    pkgDIP37.datetime.common.def();
}

void test3()
{
    import pkgDIP37.datetime.common;
    def();
    pkgDIP37.datetime.def();
    pkgDIP37.datetime.common.def();
}

void test4()
{
    import pkgDIP37.datetime : def;
    def();
    static assert(!__traits(compiles, pkgDIP37.datetime.def()));
    static assert(!__traits(compiles, pkgDIP37.datetime.common.def()));
}


void test7()
{
    static import pkgDIP37.datetime;
    static assert(!__traits(compiles, def()));
    pkgDIP37.datetime.def();
    pkgDIP37.datetime.common.def();
}

