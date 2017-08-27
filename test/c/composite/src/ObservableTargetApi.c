/* $CORTO_GENERATED
 *
 * ObservableTargetApi.c
 *
 * Only code written between the begin and end tags will be preserved
 * when the file is regenerated.
 */

#include <include/test.h>

corto_void _test_ObservableTargetApi_onUpdate(
    test_ObservableTargetApi this,
    corto_eventMask event,
    corto_object object,
    corto_observer observer)
{
/* $begin(test/ObservableTargetApi/onUpdate) */

    this->updated = TRUE;

/* $end */
}

corto_void _test_ObservableTargetApi_tc_create(
    test_ObservableTargetApi this)
{
/* $begin(test/ObservableTargetApi/tc_create) */
    test_ObservableTarget *o = test_ObservableTargetCreate(10, 20);
    test_assert(o != NULL);
    test_assert(corto_checkState(o, CORTO_VALID));
    test_assert(o->a->actual == 10);
    test_assert(o->a->target == 0);
    test_assert(o->b == 20);
    test_assert(!corto_delete(o));

/* $end */
}

corto_void _test_ObservableTargetApi_tc_createChild(
    test_ObservableTargetApi this)
{
/* $begin(test/ObservableTargetApi/tc_createChild) */
    test_ObservableTarget *o = test_ObservableTargetCreateChild(root_o, "o", 10, 20);
    test_assert(o != NULL);
    test_assert(corto_checkState(o, CORTO_VALID));
    test_assert(corto_checkAttr(o, CORTO_ATTR_NAMED));
    test_assertstr(corto_idof(o), "o");
    test_assert(corto_parentof(o) == root_o);
    test_assert(o->a->actual == 10);
    test_assert(o->a->target == 0);
    test_assert(o->b == 20);
    test_assert(!corto_delete(o));

/* $end */
}

corto_void _test_ObservableTargetApi_tc_declare(
    test_ObservableTargetApi this)
{
/* $begin(test/ObservableTargetApi/tc_declare) */
    test_ObservableTarget *o = test_ObservableTargetDeclare();
    test_assert(o != NULL);
    test_assert(!corto_checkState(o, CORTO_VALID));
    test_assert(o->a != NULL);
    test_assert(o->a->actual == 0);
    test_assert(o->a->target == 0);
    test_assert(o->b == 0);
    test_assert(!corto_delete(o));

/* $end */
}

corto_void _test_ObservableTargetApi_tc_declareChild(
    test_ObservableTargetApi this)
{
/* $begin(test/ObservableTargetApi/tc_declareChild) */
    test_ObservableTarget *o = test_ObservableTargetDeclareChild(root_o, "o");
    test_assert(o != NULL);
    test_assert(!corto_checkState(o, CORTO_VALID));
    test_assert(corto_checkAttr(o, CORTO_ATTR_NAMED));
    test_assertstr(corto_idof(o), "o");
    test_assert(corto_parentof(o) == root_o);
    test_assert(o->a != NULL);
    test_assert(o->a->actual == 0);
    test_assert(o->a->target == 0);
    test_assert(o->b == 0);
    test_assert(!corto_delete(o));

/* $end */
}

corto_void _test_ObservableTargetApi_tc_define(
    test_ObservableTargetApi this)
{
/* $begin(test/ObservableTargetApi/tc_define) */
    test_ObservableTarget *o = test_ObservableTargetDeclareChild(root_o, "o");
    test_assert(o != NULL);
    test_assert(!corto_checkState(o, CORTO_VALID));
    test_assert(corto_checkAttr(o, CORTO_ATTR_NAMED));
    test_assertstr(corto_idof(o), "o");
    test_assert(corto_parentof(o) == root_o);
    test_assert(o->a != NULL);
    test_assert(o->a->actual == 0);
    test_assert(o->a->target == 0);
    test_assert(o->b == 0);

    corto_int16 result = test_ObservableTargetDefine(o, 10, 20);
    test_assert(result == 0);
    test_assert(o->a != NULL);
    test_assert(o->a->actual == 10);
    test_assert(o->a->target == 0);
    test_assert(o->b == 20);

    test_assert(!corto_delete(o));

/* $end */
}

corto_void _test_ObservableTargetApi_tc_fromStr(
    test_ObservableTargetApi this)
{
/* $begin(test/ObservableTargetApi/tc_fromStr) */
    test_ObservableTarget *o = test_ObservableTargetCreate(10, 20);
    test_assert(o != NULL);

    test_ObservableTargetFromStr(o, "{{30, 40}, 20}");
    test_assertint(o->a->actual, 30);
    test_assertint(o->a->target, 0); /* object is owned by this thread */
    test_assertint(o->b, 20);

    corto_mount m = corto_create(corto_mount_o);
    corto_setOwner(m);
    test_ObservableTargetFromStr(o, "{{50, 60}, 20}");
    test_assertint(o->a->actual, 30); /* object is not owned by this thread */
    test_assertint(o->a->target, 60);
    test_assertint(o->b, 20);

    corto_setOwner(NULL);

    test_assert(!corto_delete(m));
    test_assert(!corto_delete(o));

/* $end */
}

corto_void _test_ObservableTargetApi_tc_set(
    test_ObservableTargetApi this)
{
/* $begin(test/ObservableTargetApi/tc_set) */
    test_ObservableTarget *o = test_ObservableTargetCreate(10, 20);
    test_assert(o != NULL);
    test_assert(corto_checkState(o, CORTO_VALID));
    test_assert(o->a->actual == 10);
    test_assert(o->a->target == 0);
    test_assert(o->b == 20);

    test_ObservableTargetSet(o, 20, 30);
    test_assert(o->a->actual == 20);
    test_assert(o->a->target == 0);
    test_assert(o->b == 30);

    test_assert(!corto_delete(o));

/* $end */
}

corto_void _test_ObservableTargetApi_tc_str(
    test_ObservableTargetApi this)
{
/* $begin(test/ObservableTargetApi/tc_str) */
    test_ObservableTarget *o = test_ObservableTargetCreate(10, 20);
    test_assert(o != NULL);

    corto_string str = test_ObservableTargetStr(o);
    test_assertstr(str, "{{10,0,0},20}");
    corto_dealloc(str);

    test_assert(!corto_delete(o));

/* $end */
}

corto_void _test_ObservableTargetApi_tc_update(
    test_ObservableTargetApi this)
{
/* $begin(test/ObservableTargetApi/tc_update) */
    corto_int16 result;

    test_ObservableTarget *o = test_ObservableTargetCreate(10, 20);
    test_assert(o != NULL);

    result = corto_observer_observe(test_ObservableTargetApi_onUpdate_o, this, o);
    test_assert(result == 0);

    result = test_ObservableTargetUpdate(o, 20, 30);
    test_assert(result == 0);
    test_assert(this->updated == TRUE);
    test_assert(o->a->actual == 20);
    test_assert(o->a->target == 0);
    test_assert(o->b == 30);

    result = corto_observer_unobserve(test_ObservableTargetApi_onUpdate_o, this, o);
    test_assert(result == 0);

    corto_delete(o);

/* $end */
}

corto_void _test_ObservableTargetApi_tc_updateTarget(
    test_ObservableTargetApi this)
{
/* $begin(test/ObservableTargetApi/tc_updateTarget) */
    corto_int16 result;

    test_ObservableTarget *o = test_ObservableTargetCreate(10, 20);
    test_assert(o != NULL);

    result = corto_observer_observe(test_ObservableTargetApi_onUpdate_o, this, o);
    test_assert(result == 0);

    result = corto_int32Update(o->a, 30);
    test_assert(result == 0);
    test_assert(this->updated == TRUE);
    test_assert(o->a->actual == 30);
    test_assert(o->a->target == 0);
    test_assert(o->b == 20);

    result = corto_observer_unobserve(test_ObservableTargetApi_onUpdate_o, this, o);
    test_assert(result == 0);

    corto_delete(o);

/* $end */
}
