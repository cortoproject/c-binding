/* $CORTO_GENERATED
 *
 * test.cpp
 *
 * Only code written between the begin and end tags will be preserved
 * when the file is regenerated.
 */

#include <include/test.h>

corto_void _test_f_collectionReferenceArg(
    test_PointArray3 *pts)
{
/* $begin(test/f_collectionReferenceArg) */

    /* << Insert implementation >> */

/* $end */
}

corto_void _test_f_compositeArg(
    test_Point *p)
{
/* $begin(test/f_compositeArg) */

    /* << Insert implementation >> */

/* $end */
}

corto_void _test_f_compositeReferenceArg(
    test_Point *p)
{
/* $begin(test/f_compositeReferenceArg) */

    /* << Insert implementation >> */

/* $end */
}

corto_void _test_f_noArgs(void)
{
/* $begin(test/f_noArgs) */

    /* << Insert implementation >> */

/* $end */
}

corto_void _test_f_primitiveArg(
    uint32_t a)
{
/* $begin(test/f_primitiveArg) */

    /* << Insert implementation >> */

/* $end */
}

corto_void _test_f_primitiveReferenceArg(
    uint32_t *a)
{
/* $begin(test/f_primitiveReferenceArg) */

    /* << Insert implementation >> */

/* $end */
}

corto_void _test_f_referenceArg(
    test_Foo f)
{
/* $begin(test/f_referenceArg) */

    /* << Insert implementation >> */

/* $end */
}

int testMain(int argc, char *argv[]) {
/* $begin(main) */
    int result = 0;
    test_Runner runner = test_RunnerCreate("corto/gen/c4cpp/function", argv[0], (argc > 1) ? argv[1] : NULL);
    if (!runner) return -1;
    if (corto_ll_size(runner->failures)) {
        result = -1;
    }
    corto_delete(runner);
    return result;
/* $end */
}
