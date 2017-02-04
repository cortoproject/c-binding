/* $CORTO_GENERATED
 *
 * OptionalCompositeApi.c
 *
 * Only code written between the begin and end tags will be preserved
 * when the file is regenerated.
 */

#include <include/test.h>

corto_void _test_OptionalCompositeApi_onUpdate(
    test_OptionalCompositeApi this,
    corto_eventMask event,
    corto_object object,
    corto_observer observer)
{
/* $begin(test/OptionalCompositeApi/onUpdate) */

    this->updated = TRUE;

/* $end */
}

corto_void _test_OptionalCompositeApi_tc_create(
    test_OptionalCompositeApi this)
{
/* $begin(test/OptionalCompositeApi/tc_create) */
    test_Point p = {30, 40};
    test_OptionalComposite *o = test_OptionalCompositeCreate(
      Set(10, 20), &p);

    test_assert(o != NULL);
    test_assert(o->a != NULL);
    test_assertint(o->a->x, 10);
    test_assertint(o->a->y, 20);
    test_assertint(o->b.x, 30);
    test_assertint(o->b.y, 40);

/* $end */
}

corto_void _test_OptionalCompositeApi_tc_createChild(
    test_OptionalCompositeApi this)
{
/* $begin(test/OptionalCompositeApi/tc_createChild) */
    test_Point p = {30, 40};
    test_OptionalComposite *o = test_OptionalCompositeCreateChild(
      root_o, "a", Set(10, 20), &p);

    test_assert(o != NULL);
    test_assert(corto_checkAttr(o, CORTO_ATTR_SCOPED));
    test_assertstr(corto_idof(o), "a");
    test_assert(corto_parentof(o) == root_o);
    test_assert(o->a != NULL);
    test_assertint(o->a->x, 10);
    test_assertint(o->a->y, 20);
    test_assertint(o->b.x, 30);
    test_assertint(o->b.y, 40);

/* $end */
}

corto_void _test_OptionalCompositeApi_tc_createChildNotSet(
    test_OptionalCompositeApi this)
{
/* $begin(test/OptionalCompositeApi/tc_createChildNotSet) */
    test_Point p = {30, 40};
    test_OptionalComposite *o = test_OptionalCompositeCreateChild(
      root_o, "a", NotSet, &p);

    test_assert(o != NULL);
    test_assert(corto_checkAttr(o, CORTO_ATTR_SCOPED));
    test_assertstr(corto_idof(o), "a");
    test_assert(corto_parentof(o) == root_o);
    test_assert(o->a == NULL);
    test_assertint(o->b.x, 30);
    test_assertint(o->b.y, 40);

/* $end */
}

corto_void _test_OptionalCompositeApi_tc_createNotSet(
    test_OptionalCompositeApi this)
{
/* $begin(test/OptionalCompositeApi/tc_createNotSet) */
    test_Point p = {30, 40};
    test_OptionalComposite *o = test_OptionalCompositeCreate(
      NotSet, &p);

    test_assert(o != NULL);
    test_assert(o->a == NULL);
    test_assertint(o->b.x, 30);
    test_assertint(o->b.y, 40);

/* $end */
}

corto_void _test_OptionalCompositeApi_tc_declare(
    test_OptionalCompositeApi this)
{
/* $begin(test/OptionalCompositeApi/tc_declare) */
    test_OptionalComposite *o = test_OptionalCompositeDeclare();

    test_assert(o != NULL);
    test_assert(!corto_checkState(o, CORTO_DEFINED));
    test_assert(o->a == NULL);
    test_assertint(o->b.x, 0);
    test_assertint(o->b.y, 0);

/* $end */
}

corto_void _test_OptionalCompositeApi_tc_declareChild(
    test_OptionalCompositeApi this)
{
/* $begin(test/OptionalCompositeApi/tc_declareChild) */
    test_OptionalComposite *o = test_OptionalCompositeDeclareChild(root_o, "a");

    test_assert(o != NULL);
    test_assert(!corto_checkState(o, CORTO_DEFINED));
    test_assert(corto_checkAttr(o, CORTO_ATTR_SCOPED));
    test_assertstr(corto_idof(o), "a");
    test_assert(corto_parentof(o) == root_o);
    test_assert(o->a == NULL);
    test_assertint(o->b.x, 0);
    test_assertint(o->b.y, 0);

/* $end */
}

corto_void _test_OptionalCompositeApi_tc_define(
    test_OptionalCompositeApi this)
{
/* $begin(test/OptionalCompositeApi/tc_define) */
    test_OptionalComposite *o = test_OptionalCompositeDeclare();

    test_assert(o != NULL);
    test_assert(!corto_checkState(o, CORTO_DEFINED));
    test_assert(o->a == NULL);
    test_assertint(o->b.x, 0);
    test_assertint(o->b.y, 0);

    test_OptionalCompositeDefine(o, Set(10, 20), NULL);
    test_assert(o->a != NULL);
    test_assertint(o->a->x, 10);
    test_assertint(o->a->y, 20);
    test_assertint(o->b.x, 0);
    test_assertint(o->b.y, 0);

/* $end */
}

corto_void _test_OptionalCompositeApi_tc_defineNotSet(
    test_OptionalCompositeApi this)
{
/* $begin(test/OptionalCompositeApi/tc_defineNotSet) */
    test_OptionalComposite *o = test_OptionalCompositeDeclare();

    test_assert(o != NULL);
    test_assert(!corto_checkState(o, CORTO_DEFINED));
    test_assert(o->a == NULL);
    test_assertint(o->b.x, 0);
    test_assertint(o->b.y, 0);

    test_OptionalCompositeDefine(o, NotSet, NULL);
    test_assert(o->a == NULL);
    test_assertint(o->b.x, 0);
    test_assertint(o->b.y, 0);

/* $end */
}

corto_void _test_OptionalCompositeApi_tc_set(
    test_OptionalCompositeApi this)
{
/* $begin(test/OptionalCompositeApi/tc_set) */
    test_OptionalComposite *o = test_OptionalCompositeCreate(NotSet, NULL);

    test_assert(o != NULL);
    test_assert(o->a == NULL);
    test_assertint(o->b.x, 0);
    test_assertint(o->b.y, 0);

    test_PointSet(o->a, 10, 20);
    test_assert(o->a != NULL);
    test_assertint(o->a->x, 10);
    test_assertint(o->a->y, 20);

/* $end */
}

corto_void _test_OptionalCompositeApi_tc_str(
    test_OptionalCompositeApi this)
{
/* $begin(test/OptionalCompositeApi/tc_str) */
    test_OptionalComposite *o = test_OptionalCompositeCreate(Set(10, 20), NULL);

    corto_string str = test_OptionalCompositeStr(o);
    test_assertstr(str, "{{10,20},{0,0}}");
    corto_dealloc(str);

/* $end */
}

corto_void _test_OptionalCompositeApi_tc_strNotSet(
    test_OptionalCompositeApi this)
{
/* $begin(test/OptionalCompositeApi/tc_strNotSet) */
    test_OptionalComposite *o = test_OptionalCompositeCreate(NotSet, NULL);

    corto_string str = test_OptionalCompositeStr(o);
    test_assertstr(str, "{{0,0}}");
    corto_dealloc(str);

/* $end */
}

corto_void _test_OptionalCompositeApi_tc_unset(
    test_OptionalCompositeApi this)
{
/* $begin(test/OptionalCompositeApi/tc_unset) */
    test_OptionalComposite *o = test_OptionalCompositeCreate(Set(10, 20), NULL);

    test_assert(o != NULL);
    test_assert(o->a != NULL);
    test_assertint(o->a->x, 10);
    test_assertint(o->a->y, 20);
    test_assertint(o->b.x, 0);
    test_assertint(o->b.y, 0);

    test_PointUnset(o->a);
    test_assert(o->a == NULL);

/* $end */
}

corto_void _test_OptionalCompositeApi_tc_update(
    test_OptionalCompositeApi this)
{
/* $begin(test/OptionalCompositeApi/tc_update) */
    corto_int16 result;

    test_OptionalComposite *o = test_OptionalCompositeCreate(NotSet, NULL);
    test_assert(o != NULL);

    result = corto_observer_observe(test_OptionalCompositeApi_onUpdate_o, this, o);
    test_assert(result == 0);

    result = test_OptionalCompositeUpdate(o, Set(10, 20), NULL);
    test_assert(result == 0);
    test_assert(this->updated == TRUE);
    test_assert(o->a != NULL);
    test_assert(o->a->x == 10);
    test_assert(o->a->y == 20);
    test_assert(o->b.x == 0);
    test_assert(o->b.y == 0);

    result = corto_observer_unobserve(test_OptionalCompositeApi_onUpdate_o, this, o);
    test_assert(result == 0);

    corto_delete(o);

/* $end */
}

corto_void _test_OptionalCompositeApi_tc_updateNotSet(
    test_OptionalCompositeApi this)
{
/* $begin(test/OptionalCompositeApi/tc_updateNotSet) */
    corto_int16 result;

    test_OptionalComposite *o = test_OptionalCompositeCreate(Set(10, 20), NULL);
    test_assert(o != NULL);

    result = corto_observer_observe(test_OptionalCompositeApi_onUpdate_o, this, o);
    test_assert(result == 0);

    result = test_OptionalCompositeUpdate(o, NotSet, NULL);
    test_assert(result == 0);
    test_assert(this->updated == TRUE);
    test_assert(o->a == NULL);
    test_assert(o->b.x == 0);
    test_assert(o->b.y == 0);

    result = corto_observer_unobserve(test_OptionalCompositeApi_onUpdate_o, this, o);
    test_assert(result == 0);

    corto_delete(o);

/* $end */
}
