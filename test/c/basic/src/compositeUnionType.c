/* This is a managed file. Do not delete this comment. */

#include <include/test.h>

int16_t test_compositeUnionType_init(
    test_compositeUnionType* this)
{

    this->d = 2;
    this->is.str = corto_strdup("foo");

    return 0;
}

