module;

#include "ctype.h"

#include <cctype>
#include <cwctype>

module fd.ctype;

CTYPE_IMPL(to, lower);
CTYPE_IMPL(to, upper);
CTYPE_IMPL(is, alnum);
CTYPE_IMPL(is, lower);
CTYPE_IMPL(is, upper);
CTYPE_IMPL(is, digit);
CTYPE_IMPL(is, xdigit);
CTYPE_IMPL(is, cntrl);
CTYPE_IMPL(is, graph);
CTYPE_IMPL(is, space);
CTYPE_IMPL(is, print);
CTYPE_IMPL(is, punct);
