/* $CORTO_GENERATED
 *
 * compositeReferenceApi.c
 *
 * Only code written between the begin and end tags will be preserved
 * when the file is regenerated.
 */

#include "test.h"

corto_void _test_compositeReferenceApi_onUpdate(test_compositeReferenceApi this, corto_object observable) {
/* $begin(test/compositeReferenceApi/onUpdate) */
    this->updated = TRUE;
/* $end */
}

corto_void _test_compositeReferenceApi_tc_compare(test_compositeReferenceApi this) {
/* $begin(test/compositeReferenceApi/tc_compare) */
    corto_equalityKind eq;
    type_compositeReferenceType a = type_compositeReferenceTypeCreate(10, 20);
    type_compositeReferenceType b = type_compositeReferenceTypeCreate(20, 30);

    eq = type_compositeReferenceTypeCompare(a, b);
    test_assert(eq == CORTO_LT);

    eq = type_compositeReferenceTypeCompare(b, a);
    test_assert(eq == CORTO_GT);

    eq = type_compositeReferenceTypeCompare(a, a);
    test_assert(eq == CORTO_EQ);
/* $end */
}

corto_void _test_compositeReferenceApi_tc_create(test_compositeReferenceApi this) {
/* $begin(test/compositeReferenceApi/tc_create) */

    type_compositeReferenceType o = type_compositeReferenceTypeCreate(10, 20);
    test_assert(o != NULL);
    test_assert(corto_typeof(o) == corto_type(type_compositeReferenceType_o));
    test_assert(corto_checkState(o, CORTO_DEFINED));
    test_assert(o->x == 10);
    test_assert(o->y == 20);
    test_assert(!corto_delete(o));

/* $end */
}

corto_void _test_compositeReferenceApi_tc_createAuto(test_compositeReferenceApi this) {
/* $begin(test/compositeReferenceApi/tc_createAuto) */

    type_compositeReferenceTypeCreate_auto(o, 10, 20);
    test_assert(o != NULL);
    test_assert(corto_typeof(o) == corto_type(type_compositeReferenceType_o));
    test_assert(corto_checkState(o, CORTO_DEFINED));
    test_assert(o->x == 10);
    test_assert(o->y == 20);
    test_assert(!corto_delete(o));

/* $end */
}

corto_void _test_compositeReferenceApi_tc_createChild(test_compositeReferenceApi this) {
/* $begin(test/compositeReferenceApi/tc_createChild) */

    type_compositeReferenceType o =
      type_compositeReferenceTypeCreateChild(root_o, "o", 10, 20);
    test_assert(o != NULL);
    test_assert(corto_typeof(o) == corto_type(type_compositeReferenceType_o));
    test_assert(corto_checkState(o, CORTO_DEFINED));
    test_assert(corto_checkAttr(o, CORTO_ATTR_SCOPED));
    test_assert(corto_parentof(o) == root_o);
    test_assert(!strcmp(corto_nameof(o), "o"));
    test_assert(o->x == 10);
    test_assert(o->y == 20);
    test_assert(!corto_delete(o));

/* $end */
}

corto_void _test_compositeReferenceApi_tc_createChildAuto(test_compositeReferenceApi this) {
/* $begin(test/compositeReferenceApi/tc_createChildAuto) */

    type_compositeReferenceType o =
      type_compositeReferenceTypeCreateChild(root_o, "o", 10, 20);
    test_assert(o != NULL);
    test_assert(corto_typeof(o) == corto_type(type_compositeReferenceType_o));
    test_assert(corto_checkState(o, CORTO_DEFINED));
    test_assert(corto_checkAttr(o, CORTO_ATTR_SCOPED));
    test_assert(corto_parentof(o) == root_o);
    test_assert(!strcmp(corto_nameof(o), "o"));
    test_assert(o->x == 10);
    test_assert(o->y == 20);
    test_assert(!corto_delete(o));

/* $end */
}

corto_void _test_compositeReferenceApi_tc_declare(test_compositeReferenceApi this) {
/* $begin(test/compositeReferenceApi/tc_declare) */

    type_compositeReferenceType o = type_compositeReferenceTypeDeclare();
    test_assert(o != NULL);
    test_assert(corto_typeof(o) == corto_type(type_compositeReferenceType_o));
    test_assert(!corto_checkState(o, CORTO_DEFINED));
    test_assert(o->x == 1);
    test_assert(o->y == 2);
    test_assert(!corto_delete(o));

/* $end */
}

corto_void _test_compositeReferenceApi_tc_declareAuto(test_compositeReferenceApi this) {
/* $begin(test/compositeReferenceApi/tc_declareAuto) */

    type_compositeReferenceTypeDeclare_auto(o);
    test_assert(o != NULL);
    test_assert(corto_typeof(o) == corto_type(type_compositeReferenceType_o));
    test_assert(!corto_checkState(o, CORTO_DEFINED));
    test_assert(o->x == 1);
    test_assert(o->y == 2);
    test_assert(!corto_delete(o));

/* $end */
}

corto_void _test_compositeReferenceApi_tc_declareChild(test_compositeReferenceApi this) {
/* $begin(test/compositeReferenceApi/tc_declareChild) */

    type_compositeReferenceType o =
        type_compositeReferenceTypeDeclareChild(root_o, "o");
    test_assert(o != NULL);
    test_assert(corto_typeof(o) == corto_type(type_compositeReferenceType_o));
    test_assert(!corto_checkState(o, CORTO_DEFINED));
    test_assert(corto_checkAttr(o, CORTO_ATTR_SCOPED));
    test_assert(corto_parentof(o) == root_o);
    test_assert(!strcmp(corto_nameof(o), "o"));
    test_assert(o->x == 1);
    test_assert(o->y == 2);
    test_assert(!corto_delete(o));

/* $end */
}

corto_void _test_compositeReferenceApi_tc_declareChildAuto(test_compositeReferenceApi this) {
/* $begin(test/compositeReferenceApi/tc_declareChildAuto) */

    type_compositeReferenceTypeDeclareChild_auto(root_o, o);
    test_assert(o != NULL);
    test_assert(corto_typeof(o) == corto_type(type_compositeReferenceType_o));
    test_assert(!corto_checkState(o, CORTO_DEFINED));
    test_assert(corto_checkAttr(o, CORTO_ATTR_SCOPED));
    test_assert(corto_parentof(o) == root_o);
    test_assert(!strcmp(corto_nameof(o), "o"));
    test_assert(o->x == 1);
    test_assert(o->y == 2);
    test_assert(!corto_delete(o));

/* $end */
}

corto_void _test_compositeReferenceApi_tc_define(test_compositeReferenceApi this) {
/* $begin(test/compositeReferenceApi/tc_define) */
    corto_int16 result;

    type_compositeReferenceType o =
        type_compositeReferenceTypeDeclareChild(root_o, "o");
    test_assert(o != NULL);
    test_assert(corto_typeof(o) == corto_type(type_compositeReferenceType_o));
    test_assert(!corto_checkState(o, CORTO_DEFINED));
    test_assert(corto_checkAttr(o, CORTO_ATTR_SCOPED));
    test_assert(corto_parentof(o) == root_o);
    test_assert(!strcmp(corto_nameof(o), "o"));
    test_assert(o->x == 1);
    test_assert(o->y == 2);

    result = type_compositeReferenceTypeDefine(o, 10, 20);
    test_assert(result == 0);
    test_assert(corto_checkState(o, CORTO_DEFINED));
    test_assert(o->x == 10);
    test_assert(o->y == 20);

    test_assert(!corto_delete(o));

/* $end */
}

corto_void _test_compositeReferenceApi_tc_fromStr(test_compositeReferenceApi this) {
/* $begin(test/compositeReferenceApi/tc_fromStr) */
    type_compositeReferenceType o = type_compositeReferenceTypeCreate(10, 20);

    type_compositeReferenceTypeFromStr(o, "{20, 30}");
    test_assert(o->x == 20);
    test_assert(o->y == 30);

    test_assert(!corto_delete(o));

/* $end */
}

corto_void _test_compositeReferenceApi_tc_set(test_compositeReferenceApi this) {
/* $begin(test/compositeReferenceApi/tc_set) */
    type_compositeReferenceType o = type_compositeReferenceTypeCreate(10, 20);

    type_compositeReferenceTypeSet(o, 20, 30);
    test_assert(o->x == 20);
    test_assert(o->y == 30);

    test_assert(!corto_delete(o));

/* $end */
}

corto_void _test_compositeReferenceApi_tc_str(test_compositeReferenceApi this) {
/* $begin(test/compositeReferenceApi/tc_str) */
    type_compositeReferenceType o = type_compositeReferenceTypeCreate(10, 20);

    corto_string str = type_compositeReferenceTypeStr(o);
    test_assert(!strcmp(str, "{10,20}"));
    corto_dealloc(str);
    test_assert(!corto_delete(o));

/* $end */
}

corto_void _test_compositeReferenceApi_tc_update(test_compositeReferenceApi this) {
/* $begin(test/compositeReferenceApi/tc_update) */
    corto_int16 result;

    type_compositeReferenceType o = type_compositeReferenceTypeCreate(10, 20);
    test_assert(o != NULL);

    result = corto_listen(
        this, test_compositeReferenceApi_onUpdate_o, CORTO_ON_UPDATE, o, NULL);
    test_assert(result == 0);

    result = type_compositeReferenceTypeUpdate(o, 20, 30);
    test_assert(result == 0);
    test_assert(this->updated == TRUE);
    test_assert(o->x == 20);
    test_assert(o->y == 30);

    result = corto_silence(
        this, test_compositeReferenceApi_onUpdate_o, CORTO_ON_UPDATE, o);
    test_assert(result == 0);

    corto_delete(o);

/* $end */
}
