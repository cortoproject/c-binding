/* $CORTO_GENERATED
 *
 * compositeUnionType.c
 *
 * Only code written between the begin and end tags will be preserved
 * when the file is regenerated.
 */

#include <include/test.h>

int16_t _test_compositeUnionType_init(
    test_compositeUnionType* this)
{
/* $begin(test/compositeUnionType/init) */

    this->d = 2;
    this->is.str = corto_strdup("foo");

    return 0;
/* $end */
}
