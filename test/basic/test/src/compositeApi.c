/* $CORTO_GENERATED
 *
 * compositeApi.c
 *
 * Only code written between the begin and end tags will be preserved
 * when the file is regenerated.
 */

#include "test.h"

corto_void _test_compositeApi_onUpdate(
    test_compositeApi this,
    corto_object observable)
{
/* $begin(test/compositeApi/onUpdate) */
    this->updated = TRUE;
/* $end */
}

corto_void _test_compositeApi_tc_compare(
    test_compositeApi this)
{
/* $begin(test/compositeApi/tc_compare) */
    corto_equalityKind eq;
    basic_compositeType a = {10, 20}, b = {20, 30};

    eq = basic_compositeTypeCompare(&a, &b);
    test_assert(eq == CORTO_LT);

    eq = basic_compositeTypeCompare(&b, &a);
    test_assert(eq == CORTO_GT);

    eq = basic_compositeTypeCompare(&a, &a);
    test_assert(eq == CORTO_EQ);

/* $end */
}

corto_void _test_compositeApi_tc_create(
    test_compositeApi this)
{
/* $begin(test/compositeApi/tc_create) */

    basic_compositeType *o = basic_compositeTypeCreate(10, 20);
    test_assert(o != NULL);
    test_assert(corto_typeof(o) == corto_type(basic_compositeType_o));
    test_assert(corto_checkState(o, CORTO_DEFINED));
    test_assert(o->x == 10);
    test_assert(o->y == 20);
    test_assert(!corto_delete(o));

/* $end */
}

corto_void _test_compositeApi_tc_createAuto(
    test_compositeApi this)
{
/* $begin(test/compositeApi/tc_createAuto) */

    basic_compositeTypeCreate_auto(o, 10, 20);
    test_assert(o != NULL);
    test_assert(corto_typeof(o) == corto_type(basic_compositeType_o));
    test_assert(corto_checkState(o, CORTO_DEFINED));
    test_assert(o->x == 10);
    test_assert(o->y == 20);
    test_assert(!corto_delete(o));

/* $end */
}

corto_void _test_compositeApi_tc_createChild(
    test_compositeApi this)
{
/* $begin(test/compositeApi/tc_createChild) */

    basic_compositeType *o = basic_compositeTypeCreateChild(root_o, "o", 10, 20);
    test_assert(o != NULL);
    test_assert(corto_typeof(o) == corto_type(basic_compositeType_o));
    test_assert(corto_checkState(o, CORTO_DEFINED));
    test_assert(corto_checkAttr(o, CORTO_ATTR_SCOPED));
    test_assert(corto_parentof(o) == root_o);
    test_assert(!strcmp(corto_nameof(o), "o"));
    test_assert(o->x == 10);
    test_assert(o->y == 20);
    test_assert(!corto_delete(o));

/* $end */
}

corto_void _test_compositeApi_tc_createChildAuto(
    test_compositeApi this)
{
/* $begin(test/compositeApi/tc_createChildAuto) */

    basic_compositeTypeCreateChild_auto(root_o, o, 10, 20);
    test_assert(o != NULL);
    test_assert(corto_typeof(o) == corto_type(basic_compositeType_o));
    test_assert(corto_checkState(o, CORTO_DEFINED));
    test_assert(corto_checkAttr(o, CORTO_ATTR_SCOPED));
    test_assert(corto_parentof(o) == root_o);
    test_assert(!strcmp(corto_nameof(o), "o"));
    test_assert(o->x == 10);
    test_assert(o->y == 20);
    test_assert(!corto_delete(o));

/* $end */
}

corto_void _test_compositeApi_tc_declare(
    test_compositeApi this)
{
/* $begin(test/compositeApi/tc_declare) */

    basic_compositeType *o = basic_compositeTypeDeclare();
    test_assert(o != NULL);
    test_assert(corto_typeof(o) == corto_type(basic_compositeType_o));
    test_assert(!corto_checkState(o, CORTO_DEFINED));
    test_assert(o->x == 1);
    test_assert(o->y == 2);
    test_assert(!corto_delete(o));

/* $end */
}

corto_void _test_compositeApi_tc_declareAuto(
    test_compositeApi this)
{
/* $begin(test/compositeApi/tc_declareAuto) */

    basic_compositeTypeDeclare_auto(o);
    test_assert(o != NULL);
    test_assert(corto_typeof(o) == corto_type(basic_compositeType_o));
    test_assert(!corto_checkState(o, CORTO_DEFINED));
    test_assert(o->x == 1);
    test_assert(o->y == 2);
    test_assert(!corto_delete(o));

/* $end */
}

corto_void _test_compositeApi_tc_declareChild(
    test_compositeApi this)
{
/* $begin(test/compositeApi/tc_declareChild) */

    basic_compositeType *o = basic_compositeTypeDeclareChild(root_o, "o");
    test_assert(o != NULL);
    test_assert(corto_typeof(o) == corto_type(basic_compositeType_o));
    test_assert(!corto_checkState(o, CORTO_DEFINED));
    test_assert(corto_checkAttr(o, CORTO_ATTR_SCOPED));
    test_assert(corto_parentof(o) == root_o);
    test_assert(!strcmp(corto_nameof(o), "o"));
    test_assert(o->x == 1);
    test_assert(o->y == 2);
    test_assert(!corto_delete(o));

/* $end */
}

corto_void _test_compositeApi_tc_declareChildAuto(
    test_compositeApi this)
{
/* $begin(test/compositeApi/tc_declareChildAuto) */

    basic_compositeTypeDeclareChild_auto(root_o, o);
    test_assert(o != NULL);
    test_assert(corto_typeof(o) == corto_type(basic_compositeType_o));
    test_assert(!corto_checkState(o, CORTO_DEFINED));
    test_assert(corto_checkAttr(o, CORTO_ATTR_SCOPED));
    test_assert(corto_parentof(o) == root_o);
    test_assert(!strcmp(corto_nameof(o), "o"));
    test_assert(o->x == 1);
    test_assert(o->y == 2);
    test_assert(!corto_delete(o));

/* $end */
}

corto_void _test_compositeApi_tc_define(
    test_compositeApi this)
{
/* $begin(test/compositeApi/tc_define) */
    corto_int16 result;

    basic_compositeType *o = basic_compositeTypeDeclareChild(root_o, "o");
    test_assert(o != NULL);
    test_assert(corto_typeof(o) == corto_type(basic_compositeType_o));
    test_assert(!corto_checkState(o, CORTO_DEFINED));
    test_assert(corto_checkAttr(o, CORTO_ATTR_SCOPED));
    test_assert(corto_parentof(o) == root_o);
    test_assert(!strcmp(corto_nameof(o), "o"));
    test_assert(o->x == 1);
    test_assert(o->y == 2);

    result = basic_compositeTypeDefine(o, 10, 20);
    test_assert(result == 0);
    test_assert(corto_checkState(o, CORTO_DEFINED));
    test_assert(o->x == 10);
    test_assert(o->y == 20);

    test_assert(!corto_delete(o));

/* $end */
}

corto_void _test_compositeApi_tc_fromStr(
    test_compositeApi this)
{
/* $begin(test/compositeApi/tc_fromStr) */
    basic_compositeType o = {10, 20};

    basic_compositeTypeFromStr(&o, "{20, 30}");
    test_assert(o.x == 20);
    test_assert(o.y == 30);

/* $end */
}

corto_void _test_compositeApi_tc_set(
    test_compositeApi this)
{
/* $begin(test/compositeApi/tc_set) */
    basic_compositeType o = {10, 20};

    basic_compositeTypeSet(&o, 20, 30);
    test_assert(o.x == 20);
    test_assert(o.y == 30);

/* $end */
}

corto_void _test_compositeApi_tc_str(
    test_compositeApi this)
{
/* $begin(test/compositeApi/tc_str) */
    basic_compositeType o = {10, 20};

    corto_string str = basic_compositeTypeStr(&o);
    test_assert(!strcmp(str, "{10,20}"));
    corto_dealloc(str);

/* $end */
}

corto_void _test_compositeApi_tc_update(
    test_compositeApi this)
{
/* $begin(test/compositeApi/tc_update) */
    corto_int16 result;

    basic_compositeType *o = basic_compositeTypeCreate(10, 20);
    test_assert(o != NULL);

    result = corto_listen(this, test_compositeApi_onUpdate_o, CORTO_ON_UPDATE, o, NULL);
    test_assert(result == 0);

    result = basic_compositeTypeUpdate(o, 20, 30);
    test_assert(result == 0);
    test_assert(this->updated == TRUE);
    test_assert(o->x == 20);
    test_assert(o->y == 30);

    result = corto_silence(this, test_compositeApi_onUpdate_o, CORTO_ON_UPDATE, o);
    test_assert(result == 0);

    corto_delete(o);

/* $end */
}
