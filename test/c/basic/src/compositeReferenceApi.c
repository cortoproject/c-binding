/* $CORTO_GENERATED
 *
 * compositeReferenceApi.c
 *
 * Only code written between the begin and end tags will be preserved
 * when the file is regenerated.
 */

#include "test.h"

corto_void _test_compositeReferenceApi_onUpdate(
    test_compositeReferenceApi this,
    corto_object observable)
{
/* $begin(test/compositeReferenceApi/onUpdate) */
    this->updated = TRUE;
/* $end */
}

corto_void _test_compositeReferenceApi_tc_compare(
    test_compositeReferenceApi this)
{
/* $begin(test/compositeReferenceApi/tc_compare) */
    corto_equalityKind eq;
    test_compositeReferenceType a = test_compositeReferenceTypeCreate(10, 20);
    test_compositeReferenceType b = test_compositeReferenceTypeCreate(20, 30);

    eq = test_compositeReferenceTypeCompare(a, b);
    test_assert(eq == CORTO_LT);

    eq = test_compositeReferenceTypeCompare(b, a);
    test_assert(eq == CORTO_GT);

    eq = test_compositeReferenceTypeCompare(a, a);
    test_assert(eq == CORTO_EQ);
/* $end */
}

corto_void _test_compositeReferenceApi_tc_create(
    test_compositeReferenceApi this)
{
/* $begin(test/compositeReferenceApi/tc_create) */

    test_compositeReferenceType o = test_compositeReferenceTypeCreate(10, 20);
    test_assert(o != NULL);
    test_assert(corto_typeof(o) == corto_type(test_compositeReferenceType_o));
    test_assert(corto_checkState(o, CORTO_DEFINED));
    test_assert(o->x == 10);
    test_assert(o->y == 20);
    test_assert(!corto_delete(o));

/* $end */
}

corto_void _test_compositeReferenceApi_tc_createAuto(
    test_compositeReferenceApi this)
{
/* $begin(test/compositeReferenceApi/tc_createAuto) */

    test_compositeReferenceTypeCreate_auto(o, 10, 20);
    test_assert(o != NULL);
    test_assert(corto_typeof(o) == corto_type(test_compositeReferenceType_o));
    test_assert(corto_checkState(o, CORTO_DEFINED));
    test_assert(o->x == 10);
    test_assert(o->y == 20);
    test_assert(!corto_delete(o));

/* $end */
}

corto_void _test_compositeReferenceApi_tc_createChild(
    test_compositeReferenceApi this)
{
/* $begin(test/compositeReferenceApi/tc_createChild) */

    test_compositeReferenceType o =
      test_compositeReferenceTypeCreateChild(root_o, "o", 10, 20);
    test_assert(o != NULL);
    test_assert(corto_typeof(o) == corto_type(test_compositeReferenceType_o));
    test_assert(corto_checkState(o, CORTO_DEFINED));
    test_assert(corto_checkAttr(o, CORTO_ATTR_SCOPED));
    test_assert(corto_parentof(o) == root_o);
    test_assert(!strcmp(corto_idof(o), "o"));
    test_assert(o->x == 10);
    test_assert(o->y == 20);
    test_assert(!corto_delete(o));

/* $end */
}

corto_void _test_compositeReferenceApi_tc_createChildAuto(
    test_compositeReferenceApi this)
{
/* $begin(test/compositeReferenceApi/tc_createChildAuto) */

    test_compositeReferenceType o =
      test_compositeReferenceTypeCreateChild(root_o, "o", 10, 20);
    test_assert(o != NULL);
    test_assert(corto_typeof(o) == corto_type(test_compositeReferenceType_o));
    test_assert(corto_checkState(o, CORTO_DEFINED));
    test_assert(corto_checkAttr(o, CORTO_ATTR_SCOPED));
    test_assert(corto_parentof(o) == root_o);
    test_assert(!strcmp(corto_idof(o), "o"));
    test_assert(o->x == 10);
    test_assert(o->y == 20);
    test_assert(!corto_delete(o));

/* $end */
}

corto_void _test_compositeReferenceApi_tc_declare(
    test_compositeReferenceApi this)
{
/* $begin(test/compositeReferenceApi/tc_declare) */

    test_compositeReferenceType o = test_compositeReferenceTypeDeclare();
    test_assert(o != NULL);
    test_assert(corto_typeof(o) == corto_type(test_compositeReferenceType_o));
    test_assert(!corto_checkState(o, CORTO_DEFINED));
    test_assert(o->x == 1);
    test_assert(o->y == 2);
    test_assert(!corto_delete(o));

/* $end */
}

corto_void _test_compositeReferenceApi_tc_declareAuto(
    test_compositeReferenceApi this)
{
/* $begin(test/compositeReferenceApi/tc_declareAuto) */

    test_compositeReferenceTypeDeclare_auto(o);
    test_assert(o != NULL);
    test_assert(corto_typeof(o) == corto_type(test_compositeReferenceType_o));
    test_assert(!corto_checkState(o, CORTO_DEFINED));
    test_assert(o->x == 1);
    test_assert(o->y == 2);
    test_assert(!corto_delete(o));

/* $end */
}

corto_void _test_compositeReferenceApi_tc_declareChild(
    test_compositeReferenceApi this)
{
/* $begin(test/compositeReferenceApi/tc_declareChild) */

    test_compositeReferenceType o =
        test_compositeReferenceTypeDeclareChild(root_o, "o");
    test_assert(o != NULL);
    test_assert(corto_typeof(o) == corto_type(test_compositeReferenceType_o));
    test_assert(!corto_checkState(o, CORTO_DEFINED));
    test_assert(corto_checkAttr(o, CORTO_ATTR_SCOPED));
    test_assert(corto_parentof(o) == root_o);
    test_assert(!strcmp(corto_idof(o), "o"));
    test_assert(o->x == 1);
    test_assert(o->y == 2);
    test_assert(!corto_delete(o));

/* $end */
}

corto_void _test_compositeReferenceApi_tc_declareChildAuto(
    test_compositeReferenceApi this)
{
/* $begin(test/compositeReferenceApi/tc_declareChildAuto) */

    test_compositeReferenceTypeDeclareChild_auto(root_o, o);
    test_assert(o != NULL);
    test_assert(corto_typeof(o) == corto_type(test_compositeReferenceType_o));
    test_assert(!corto_checkState(o, CORTO_DEFINED));
    test_assert(corto_checkAttr(o, CORTO_ATTR_SCOPED));
    test_assert(corto_parentof(o) == root_o);
    test_assert(!strcmp(corto_idof(o), "o"));
    test_assert(o->x == 1);
    test_assert(o->y == 2);
    test_assert(!corto_delete(o));

/* $end */
}

corto_void _test_compositeReferenceApi_tc_define(
    test_compositeReferenceApi this)
{
/* $begin(test/compositeReferenceApi/tc_define) */
    corto_int16 result;

    test_compositeReferenceType o =
        test_compositeReferenceTypeDeclareChild(root_o, "o");
    test_assert(o != NULL);
    test_assert(corto_typeof(o) == corto_type(test_compositeReferenceType_o));
    test_assert(!corto_checkState(o, CORTO_DEFINED));
    test_assert(corto_checkAttr(o, CORTO_ATTR_SCOPED));
    test_assert(corto_parentof(o) == root_o);
    test_assert(!strcmp(corto_idof(o), "o"));
    test_assert(o->x == 1);
    test_assert(o->y == 2);

    result = test_compositeReferenceTypeDefine(o, 10, 20);
    test_assert(result == 0);
    test_assert(corto_checkState(o, CORTO_DEFINED));
    test_assert(o->x == 10);
    test_assert(o->y == 20);

    test_assert(!corto_delete(o));

/* $end */
}

corto_void _test_compositeReferenceApi_tc_fromStr(
    test_compositeReferenceApi this)
{
/* $begin(test/compositeReferenceApi/tc_fromStr) */
    test_compositeReferenceType o = test_compositeReferenceTypeCreate(10, 20);

    test_compositeReferenceTypeFromStr(o, "{20, 30}");
    test_assert(o->x == 20);
    test_assert(o->y == 30);

    test_assert(!corto_delete(o));

/* $end */
}

corto_void _test_compositeReferenceApi_tc_set(
    test_compositeReferenceApi this)
{
/* $begin(test/compositeReferenceApi/tc_set) */
    test_compositeReferenceType o = test_compositeReferenceTypeCreate(10, 20);

    test_compositeReferenceTypeSet(o, 20, 30);
    test_assert(o->x == 20);
    test_assert(o->y == 30);

    test_assert(!corto_delete(o));

/* $end */
}

corto_void _test_compositeReferenceApi_tc_str(
    test_compositeReferenceApi this)
{
/* $begin(test/compositeReferenceApi/tc_str) */
    test_compositeReferenceType o = test_compositeReferenceTypeCreate(10, 20);

    corto_string str = test_compositeReferenceTypeStr(o);
    test_assert(!strcmp(str, "{10,20}"));
    corto_dealloc(str);
    test_assert(!corto_delete(o));

/* $end */
}

corto_void _test_compositeReferenceApi_tc_update(
    test_compositeReferenceApi this)
{
/* $begin(test/compositeReferenceApi/tc_update) */
    corto_int16 result;

    test_compositeReferenceType o = test_compositeReferenceTypeCreate(10, 20);
    test_assert(o != NULL);

    result = corto_listen(
        this, test_compositeReferenceApi_onUpdate_o, CORTO_ON_UPDATE, o, NULL);
    test_assert(result == 0);

    result = test_compositeReferenceTypeUpdate(o, 20, 30);
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
