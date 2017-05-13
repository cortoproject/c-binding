/* $CORTO_GENERATED
 *
 * compositeUnionType.cpp
 *
 * Only code written between the begin and end tags will be preserved
 * when the file is regenerated.
 */

#include <include/test.h>

int16_t _test_compositeUnionType_init(
    test_compositeUnionType* _this)
{
/* $begin(test/compositeUnionType/init) */

    _this->d = 2;
    _this->is.str = corto_strdup("foo");

    return 0;
/* $end */
}
