/* $CORTO_GENERATED
 *
 * collectionApi.c
 *
 * Only code written between the begin and end tags will be preserved
 * when the file is regenerated.
 */

#include "test.h"

corto_void _test_collectionApi_onUpdate(test_collectionApi this, corto_object observable) {
/* $begin(test/collectionApi/onUpdate) */
    this->updated = TRUE;
/* $end */
}

corto_void _test_collectionApi_tc_compare(test_collectionApi this) {
/* $begin(test/collectionApi/tc_compare) */
    corto_equalityKind eq;
    basic_collectionType a = {10, 20}, b = {20, 30};

    eq = basic_collectionTypeCompare(a, b);
    test_assert(eq == CORTO_LT);

    eq = basic_collectionTypeCompare(b, a);
    test_assert(eq == CORTO_GT);

    eq = basic_collectionTypeCompare(a, a);
    test_assert(eq == CORTO_EQ);

/* $end */
}

corto_void _test_collectionApi_tc_create(test_collectionApi this) {
/* $begin(test/collectionApi/tc_create) */

    corto_int32* o =
        basic_collectionTypeCreate(2, ((corto_int32[]){10, 20}));
    test_assert(o != NULL);
    test_assert(corto_typeof(o) == corto_type(basic_collectionType_o));
    test_assert(corto_checkState(o, CORTO_DEFINED));
    test_assert(o[0] == 10);
    test_assert(o[1] == 20);
    test_assert(!corto_delete(o));

/* $end */
}

corto_void _test_collectionApi_tc_createAuto(test_collectionApi this) {
/* $begin(test/collectionApi/tc_createAuto) */

    basic_collectionTypeCreate_auto(o, 2, ((corto_int32[]){10, 20}));
    test_assert(o != NULL);
    test_assert(corto_typeof(o) == corto_type(basic_collectionType_o));
    test_assert(corto_checkState(o, CORTO_DEFINED));
    test_assert(o[0] == 10);
    test_assert(o[1] == 20);
    test_assert(!corto_delete(o));

/* $end */
}

corto_void _test_collectionApi_tc_createChild(test_collectionApi this) {
/* $begin(test/collectionApi/tc_createChild) */

    corto_int32 *o =
      basic_collectionTypeCreateChild(root_o, "o", 2, ((corto_int32[]){10, 20}));
    test_assert(o != NULL);
    test_assert(corto_typeof(o) == corto_type(basic_collectionType_o));
    test_assert(corto_checkState(o, CORTO_DEFINED));
    test_assert(corto_checkAttr(o, CORTO_ATTR_SCOPED));
    test_assert(corto_parentof(o) == root_o);
    test_assert(!strcmp(corto_nameof(o), "o"));
    test_assert(o[0] == 10);
    test_assert(o[1] == 20);
    test_assert(!corto_delete(o));

/* $end */
}

corto_void _test_collectionApi_tc_createChildAuto(test_collectionApi this) {
/* $begin(test/collectionApi/tc_createChildAuto) */

    basic_collectionTypeCreateChild_auto(root_o, o, 2, ((corto_int32[]){10, 20}));
    test_assert(o != NULL);
    test_assert(corto_typeof(o) == corto_type(basic_collectionType_o));
    test_assert(corto_checkState(o, CORTO_DEFINED));
    test_assert(corto_checkAttr(o, CORTO_ATTR_SCOPED));
    test_assert(corto_parentof(o) == root_o);
    test_assert(!strcmp(corto_nameof(o), "o"));
    test_assert(o[0] == 10);
    test_assert(o[1] == 20);
    test_assert(!corto_delete(o));

/* $end */
}

corto_void _test_collectionApi_tc_declare(test_collectionApi this) {
/* $begin(test/collectionApi/tc_declare) */

    corto_int32 *o = basic_collectionTypeDeclare();
    test_assert(o != NULL);
    test_assert(corto_typeof(o) == corto_type(basic_collectionType_o));
    test_assert(!corto_checkState(o, CORTO_DEFINED));
    test_assert(o[0] == 1);
    test_assert(o[1] == 2);
    test_assert(!corto_delete(o));

/* $end */
}

corto_void _test_collectionApi_tc_declareAuto(test_collectionApi this) {
/* $begin(test/collectionApi/tc_declareAuto) */

    basic_collectionTypeDeclare_auto(o);
    test_assert(o != NULL);
    test_assert(corto_typeof(o) == corto_type(basic_collectionType_o));
    test_assert(!corto_checkState(o, CORTO_DEFINED));
    test_assert(o[0] == 1);
    test_assert(o[1] == 2);
    test_assert(!corto_delete(o));

/* $end */
}

corto_void _test_collectionApi_tc_declareChild(test_collectionApi this) {
/* $begin(test/collectionApi/tc_declareChild) */

    corto_int32 *o = basic_collectionTypeDeclareChild(root_o, "o");
    test_assert(o != NULL);
    test_assert(corto_typeof(o) == corto_type(basic_collectionType_o));
    test_assert(!corto_checkState(o, CORTO_DEFINED));
    test_assert(corto_checkAttr(o, CORTO_ATTR_SCOPED));
    test_assert(corto_parentof(o) == root_o);
    test_assert(!strcmp(corto_nameof(o), "o"));
    test_assert(o[0] == 1);
    test_assert(o[1] == 2);
    test_assert(!corto_delete(o));

/* $end */
}

corto_void _test_collectionApi_tc_declareChildAuto(test_collectionApi this) {
/* $begin(test/collectionApi/tc_declareChildAuto) */

    basic_collectionTypeDeclareChild_auto(root_o, o);
    test_assert(o != NULL);
    test_assert(corto_typeof(o) == corto_type(basic_collectionType_o));
    test_assert(!corto_checkState(o, CORTO_DEFINED));
    test_assert(corto_checkAttr(o, CORTO_ATTR_SCOPED));
    test_assert(corto_parentof(o) == root_o);
    test_assert(!strcmp(corto_nameof(o), "o"));
    test_assert(o[0] == 1);
    test_assert(o[1] == 2);
    test_assert(!corto_delete(o));

/* $end */
}

corto_void _test_collectionApi_tc_define(test_collectionApi this) {
/* $begin(test/collectionApi/tc_define) */
    corto_int16 result;

    corto_int32 *o = basic_collectionTypeDeclareChild(root_o, "o");
    test_assert(o != NULL);
    test_assert(corto_typeof(o) == corto_type(basic_collectionType_o));
    test_assert(!corto_checkState(o, CORTO_DEFINED));
    test_assert(corto_checkAttr(o, CORTO_ATTR_SCOPED));
    test_assert(corto_parentof(o) == root_o);
    test_assert(!strcmp(corto_nameof(o), "o"));
    test_assert(o[0] == 1);
    test_assert(o[1] == 2);

    result = basic_collectionTypeDefine(o, 2, ((corto_int32[]){10, 20}));
    test_assert(result == 0);
    test_assert(corto_checkState(o, CORTO_DEFINED));
    test_assert(o[0] == 10);
    test_assert(o[1] == 20);

    test_assert(!corto_delete(o));

/* $end */
}

corto_void _test_collectionApi_tc_fromStr(test_collectionApi this) {
/* $begin(test/collectionApi/tc_fromStr) */
    basic_collectionType o = {10, 20};

    basic_collectionTypeFromStr(o, "{20, 30}");
    test_assert(o[0] == 20);
    test_assert(o[1] == 30);

/* $end */
}

corto_void _test_collectionApi_tc_set(test_collectionApi this) {
/* $begin(test/collectionApi/tc_set) */
    basic_collectionType o = {10, 20};

    basic_collectionTypeSet(o, 2, ((corto_int32[]){20, 30}));
    test_assert(o[0] == 20);
    test_assert(o[1] == 30);

/* $end */
}

corto_void _test_collectionApi_tc_str(test_collectionApi this) {
/* $begin(test/collectionApi/tc_str) */
    basic_collectionType o = {10, 20};

    corto_string str = basic_collectionTypeStr(o);
    test_assert(!strcmp(str, "{10,20}"));

/* $end */
}

corto_void _test_collectionApi_tc_update(test_collectionApi this) {
/* $begin(test/collectionApi/tc_update) */
    corto_int16 result;

    corto_int32 *o =
        basic_collectionTypeCreate(2, ((corto_int32[]){10, 20}));
    test_assert(o != NULL);

    result = corto_listen(this, test_collectionApi_onUpdate_o, CORTO_ON_UPDATE, o, NULL);
    test_assert(result == 0);

    result = basic_collectionTypeUpdate(o, 2, ((corto_int32[]){20, 30}));
    test_assert(result == 0);
    test_assert(this->updated == TRUE);
    test_assert(o[0] == 20);
    test_assert(o[1] == 30);

    result = corto_silence(this, test_collectionApi_onUpdate_o, CORTO_ON_UPDATE, o);
    test_assert(result == 0);

    corto_delete(o);

/* $end */
}
