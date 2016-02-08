/* $CORTO_GENERATED
 *
 * anyApi.c
 *
 * Only code written between the begin and end tags will be preserved
 * when the file is regenerated.
 */

#include "test.h"

corto_void _test_anyApi_onUpdate(test_anyApi this, corto_object observable) {
/* $begin(test/anyApi/onUpdate) */

    /* << Insert implementation >> */

/* $end */
}

corto_void _test_anyApi_tc_compare(test_anyApi this) {
/* $begin(test/anyApi/tc_compare) */
    corto_int32 i1 = 10, i2 = 20;
    type_anyType a = {corto_type(corto_int32_o), &i1, FALSE};
    type_anyType b = {corto_type(corto_int32_o), &i2, FALSE};
    corto_equalityKind eq;

    eq = type_anyTypeCompare(a, b);
    test_assert(eq == CORTO_LT);

    eq = type_anyTypeCompare(b, a);
    test_assert(eq == CORTO_GT);

    eq = type_anyTypeCompare(a, a);
    test_assert(eq == CORTO_EQ);

/* $end */
}

corto_void _test_anyApi_tc_create(test_anyApi this) {
/* $begin(test/anyApi/tc_create) */

    type_anyType *o = type_anyTypeCreate(corto_string_o, "Foo");
    test_assert(o != NULL);
    test_assert(corto_typeof(o) == corto_type(type_anyType_o));
    test_assert(corto_checkState(o, CORTO_DEFINED));
    test_assert(o->type == corto_type(corto_string_o));
    test_assert(!strcmp(o->value, "Foo"));
    test_assert(o->owner = TRUE);
    test_assert(!corto_delete(o));

/* $end */
}

corto_void _test_anyApi_tc_createAuto(test_anyApi this) {
/* $begin(test/anyApi/tc_createAuto) */

    /* << Insert implementation >> */

/* $end */
}

corto_void _test_anyApi_tc_createChild(test_anyApi this) {
/* $begin(test/anyApi/tc_createChild) */

    /* << Insert implementation >> */

/* $end */
}

corto_void _test_anyApi_tc_createChildAuto(test_anyApi this) {
/* $begin(test/anyApi/tc_createChildAuto) */

    /* << Insert implementation >> */

/* $end */
}

corto_void _test_anyApi_tc_declare(test_anyApi this) {
/* $begin(test/anyApi/tc_declare) */

    /* << Insert implementation >> */

/* $end */
}

corto_void _test_anyApi_tc_declareAuto(test_anyApi this) {
/* $begin(test/anyApi/tc_declareAuto) */

    /* << Insert implementation >> */

/* $end */
}

corto_void _test_anyApi_tc_declareChild(test_anyApi this) {
/* $begin(test/anyApi/tc_declareChild) */

    /* << Insert implementation >> */

/* $end */
}

corto_void _test_anyApi_tc_declareChildAuto(test_anyApi this) {
/* $begin(test/anyApi/tc_declareChildAuto) */

    /* << Insert implementation >> */

/* $end */
}

corto_void _test_anyApi_tc_define(test_anyApi this) {
/* $begin(test/anyApi/tc_define) */

    /* << Insert implementation >> */

/* $end */
}

corto_void _test_anyApi_tc_fromStr(test_anyApi this) {
/* $begin(test/anyApi/tc_fromStr) */

    /* << Insert implementation >> */

/* $end */
}

corto_void _test_anyApi_tc_set(test_anyApi this) {
/* $begin(test/anyApi/tc_set) */

    /* << Insert implementation >> */

/* $end */
}

corto_void _test_anyApi_tc_str(test_anyApi this) {
/* $begin(test/anyApi/tc_str) */

    /* << Insert implementation >> */

/* $end */
}

corto_void _test_anyApi_tc_update(test_anyApi this) {
/* $begin(test/anyApi/tc_update) */

    /* << Insert implementation >> */

/* $end */
}
