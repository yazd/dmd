// PERMUTE_ARGS:
// REQUIRED_ARGS: -D -w -o-

/*
TEST_OUTPUT:
---
compilable/ddoc4899.d(16): Warning: Ddoc: Stray '('. This may cause incorrect Ddoc output. Use $(LPAREN) instead for unpaired left parentheses.
compilable/ddoc4899.d(16): Warning: Ddoc: Stray ')'. This may cause incorrect Ddoc output. Use $(RPAREN) instead for unpaired right parentheses.
---
*/

/++
       (See accompanying file LICENSE_1_0.txt or copy at
        http://www.boost.org/LICENSE_1_0.txt)
+/
module d;

void main()
{
}
