/* $CORTO_GENERATED
 *
 * voidApi.c
 *
 * Only code written between the begin and end tags will be preserved
 * when the file is regenerated.
 */

#include <include/test.h>

corto_void _test_voidApi_onUpdate(
    test_voidApi this,
    corto_eventMask event,
    corto_object object,
    corto_observer observer)
{
/* $begin(test/voidApi/onUpdate) */
    this->updated = TRUE;
/* $end */
}

corto_void _test_voidApi_tc_create(
    test_voidApi this)
{
/* $begin(test/voidApi/tc_create) */

    test_voidType *o = test_voidTypeCreate();
    test_assert(o != NULL);
    test_assert(corto_typeof(o) == test_voidType_o);
    test_assert(corto_checkState(o, CORTO_DEFINED));
    test_assert(!corto_delete(o));

/* $end */
}

corto_void _test_voidApi_tc_createAuto(
    test_voidApi this)
{
/* $begin(test/voidApi/tc_createAuto) */

    test_voidTypeCreate_auto(o);
    test_assert(o != NULL);
    test_assert(corto_typeof(o) == test_voidType_o);
    test_assert(corto_checkState(o, CORTO_DEFINED));
    test_assert(!corto_delete(o));

/* $end */
}

corto_void _test_voidApi_tc_createChild(
    test_voidApi this)
{
/* $begin(test/voidApi/tc_createChild) */

    test_voidType *o = test_voidTypeCreateChild(root_o, "o");
    test_assert(o != NULL);
    test_assert(corto_typeof(o) == test_voidType_o);
    test_assert(corto_checkState(o, CORTO_DEFINED));
    test_assert(corto_checkAttr(o, CORTO_ATTR_NAMED));
    test_assert(corto_parentof(o) == root_o);
    test_assert(!strcmp(corto_idof(o), "o"));
    test_assert(!corto_delete(o));

/* $end */
}

corto_void _test_voidApi_tc_createChildAuto(
    test_voidApi this)
{
/* $begin(test/voidApi/tc_createChildAuto) */

    test_voidTypeCreateChild_auto(root_o, o);
    test_assert(o != NULL);
    test_assert(corto_typeof(o) == test_voidType_o);
    test_assert(corto_checkState(o, CORTO_DEFINED));
    test_assert(corto_checkAttr(o, CORTO_ATTR_NAMED));
    test_assert(corto_parentof(o) == root_o);
    test_assert(!strcmp(corto_idof(o), "o"));
    test_assert(!corto_delete(o));

/* $end */
}

corto_void _test_voidApi_tc_update(
    test_voidApi this)
{
/* $begin(test/voidApi/tc_update) */
    corto_int16 result;

    test_voidType *o = test_voidTypeCreate();
    test_assert(o != NULL);

    result = corto_observer_observe(test_voidApi_onUpdate_o, this, o);
    test_assert(result == 0);

    result = test_voidTypeUpdate(o);
    test_assert(result == 0);
    test_assert(this->updated == TRUE);

    result = corto_observer_unobserve(test_voidApi_onUpdate_o, this, o);
    test_assert(result == 0);

    corto_delete(o);

/* $end */
}
