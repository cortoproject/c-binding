/* $CORTO_GENERATED
 *
 * compositeApi.c
 *
 * Only code written between the begin and end tags will be preserved
 * when the file is regenerated.
 */

#include "test.h"

corto_void _test_compositeApi_onUpdate(test_compositeApi this, corto_object observable) {
/* $begin(test/compositeApi/onUpdate) */
    this->updated = TRUE;
/* $end */
}

corto_void _test_compositeApi_tc_compare(test_compositeApi this) {
/* $begin(test/compositeApi/tc_compare) */
    corto_equalityKind eq;
    type_compositeType a = {10, 20}, b = {20, 30};

    eq = type_compositeTypeCompare(&a, &b);
    test_assert(eq == CORTO_LT);

    eq = type_compositeTypeCompare(&b, &a);
    test_assert(eq == CORTO_GT);

    eq = type_compositeTypeCompare(&a, &a);
    test_assert(eq == CORTO_EQ);

/* $end */
}

corto_void _test_compositeApi_tc_create(test_compositeApi this) {
/* $begin(test/compositeApi/tc_create) */

    type_compositeType *o = type_compositeTypeCreate(10, 20);
    test_assert(o != NULL);
    test_assert(corto_typeof(o) == corto_type(type_compositeType_o));
    test_assert(corto_checkState(o, CORTO_DEFINED));
    test_assert(o->x == 10);
    test_assert(o->y == 20);
    test_assert(!corto_delete(o));

/* $end */
}

corto_void _test_compositeApi_tc_createAuto(test_compositeApi this) {
/* $begin(test/compositeApi/tc_createAuto) */

    type_compositeTypeCreate_auto(o, 10, 20);
    test_assert(o != NULL);
    test_assert(corto_typeof(o) == corto_type(type_compositeType_o));
    test_assert(corto_checkState(o, CORTO_DEFINED));
    test_assert(o->x == 10);
    test_assert(o->y == 20);
    test_assert(!corto_delete(o));

/* $end */
}

corto_void _test_compositeApi_tc_createChild(test_compositeApi this) {
/* $begin(test/compositeApi/tc_createChild) */

    type_compositeType *o = type_compositeTypeCreateChild(root_o, "o", 10, 20);
    test_assert(o != NULL);
    test_assert(corto_typeof(o) == corto_type(type_compositeType_o));
    test_assert(corto_checkState(o, CORTO_DEFINED));
    test_assert(corto_checkAttr(o, CORTO_ATTR_SCOPED));
    test_assert(corto_parentof(o) == root_o);
    test_assert(!strcmp(corto_nameof(o), "o"));
    test_assert(o->x == 10);
    test_assert(o->y == 20);
    test_assert(!corto_delete(o));

/* $end */
}

corto_void _test_compositeApi_tc_createChildAuto(test_compositeApi this) {
/* $begin(test/compositeApi/tc_createChildAuto) */

    type_compositeTypeCreateChild_auto(root_o, o, 10, 20);
    test_assert(o != NULL);
    test_assert(corto_typeof(o) == corto_type(type_compositeType_o));
    test_assert(corto_checkState(o, CORTO_DEFINED));
    test_assert(corto_checkAttr(o, CORTO_ATTR_SCOPED));
    test_assert(corto_parentof(o) == root_o);
    test_assert(!strcmp(corto_nameof(o), "o"));
    test_assert(o->x == 10);
    test_assert(o->y == 20);
    test_assert(!corto_delete(o));

/* $end */
}

corto_void _test_compositeApi_tc_declare(test_compositeApi this) {
/* $begin(test/compositeApi/tc_declare) */

    type_compositeType *o = type_compositeTypeDeclare();
    test_assert(o != NULL);
    test_assert(corto_typeof(o) == corto_type(type_compositeType_o));
    test_assert(!corto_checkState(o, CORTO_DEFINED));
    test_assert(o->x == 1);
    test_assert(o->y == 2);
    test_assert(!corto_delete(o));

/* $end */
}

corto_void _test_compositeApi_tc_declareAuto(test_compositeApi this) {
/* $begin(test/compositeApi/tc_declareAuto) */

    type_compositeTypeDeclare_auto(o);
    test_assert(o != NULL);
    test_assert(corto_typeof(o) == corto_type(type_compositeType_o));
    test_assert(!corto_checkState(o, CORTO_DEFINED));
    test_assert(o->x == 1);
    test_assert(o->y == 2);
    test_assert(!corto_delete(o));

/* $end */
}

corto_void _test_compositeApi_tc_declareChild(test_compositeApi this) {
/* $begin(test/compositeApi/tc_declareChild) */

    type_compositeType *o = type_compositeTypeDeclareChild(root_o, "o");
    test_assert(o != NULL);
    test_assert(corto_typeof(o) == corto_type(type_compositeType_o));
    test_assert(!corto_checkState(o, CORTO_DEFINED));
    test_assert(corto_checkAttr(o, CORTO_ATTR_SCOPED));
    test_assert(corto_parentof(o) == root_o);
    test_assert(!strcmp(corto_nameof(o), "o"));
    test_assert(o->x == 1);
    test_assert(o->y == 2);
    test_assert(!corto_delete(o));

/* $end */
}

corto_void _test_compositeApi_tc_declareChildAuto(test_compositeApi this) {
/* $begin(test/compositeApi/tc_declareChildAuto) */

    type_compositeTypeDeclareChild_auto(root_o, o);
    test_assert(o != NULL);
    test_assert(corto_typeof(o) == corto_type(type_compositeType_o));
    test_assert(!corto_checkState(o, CORTO_DEFINED));
    test_assert(corto_checkAttr(o, CORTO_ATTR_SCOPED));
    test_assert(corto_parentof(o) == root_o);
    test_assert(!strcmp(corto_nameof(o), "o"));
    test_assert(o->x == 1);
    test_assert(o->y == 2);
    test_assert(!corto_delete(o));

/* $end */
}

corto_void _test_compositeApi_tc_define(test_compositeApi this) {
/* $begin(test/compositeApi/tc_define) */
    corto_int16 result;

    type_compositeType *o = type_compositeTypeDeclareChild(root_o, "o");
    test_assert(o != NULL);
    test_assert(corto_typeof(o) == corto_type(type_compositeType_o));
    test_assert(!corto_checkState(o, CORTO_DEFINED));
    test_assert(corto_checkAttr(o, CORTO_ATTR_SCOPED));
    test_assert(corto_parentof(o) == root_o);
    test_assert(!strcmp(corto_nameof(o), "o"));
    test_assert(o->x == 1);
    test_assert(o->y == 2);

    result = type_compositeTypeDefine(o, 10, 20);
    test_assert(result == 0);
    test_assert(corto_checkState(o, CORTO_DEFINED));
    test_assert(o->x == 10);
    test_assert(o->y == 20);

    test_assert(!corto_delete(o));

/* $end */
}

corto_void _test_compositeApi_tc_fromStr(test_compositeApi this) {
/* $begin(test/compositeApi/tc_fromStr) */
    type_compositeType o = {10, 20};

    type_compositeTypeFromStr(&o, "{20, 30}");
    test_assert(o.x == 20);
    test_assert(o.y == 30);

/* $end */
}

corto_void _test_compositeApi_tc_set(test_compositeApi this) {
/* $begin(test/compositeApi/tc_set) */
    type_compositeType o = {10, 20};

    type_compositeTypeSet(&o, 20, 30);
    test_assert(o.x == 20);
    test_assert(o.y == 30);

/* $end */
}

corto_void _test_compositeApi_tc_str(test_compositeApi this) {
/* $begin(test/compositeApi/tc_str) */
    type_compositeType o = {10, 20};

    corto_string str = type_compositeTypeStr(&o);
    test_assert(!strcmp(str, "{10,20}"));

/* $end */
}

corto_void _test_compositeApi_tc_update(test_compositeApi this) {
/* $begin(test/compositeApi/tc_update) */
    corto_int16 result;

    type_compositeType *o = type_compositeTypeCreate(10, 20);
    test_assert(o != NULL);

    result = corto_listen(this, test_compositeApi_onUpdate_o, CORTO_ON_UPDATE, o, NULL);
    test_assert(result == 0);

    result = type_compositeTypeUpdate(o, 20, 30);
    test_assert(result == 0);
    test_assert(this->updated == TRUE);
    test_assert(o->x == 20);
    test_assert(o->y == 30);

    result = corto_silence(this, test_compositeApi_onUpdate_o, CORTO_ON_UPDATE, o);
    test_assert(result == 0);

    corto_delete(o);

/* $end */
}
