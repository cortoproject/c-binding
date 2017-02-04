/* $CORTO_GENERATED
 *
 * OptionalApi.cpp
 *
 * Only code written between the begin and end tags will be preserved
 * when the file is regenerated.
 */

#include <include/test.h>

corto_void _test_OptionalApi_onUpdate(
    test_OptionalApi _this,
    corto_eventMask event,
    corto_object object,
    corto_observer observer)
{
/* $begin(test/OptionalApi/onUpdate) */

    _this->updated = TRUE;

/* $end */
}

corto_void _test_OptionalApi_tc_create(
    test_OptionalApi _this)
{
/* $begin(test/OptionalApi/tc_create) */
    test_Optional *o = test_OptionalCreate(Set(10), 20);

    test_assert(o != NULL);
    test_assert(o->a != NULL);
    test_assertint(*o->a, 10);
    test_assertint(o->b, 20);

/* $end */
}

corto_void _test_OptionalApi_tc_createChild(
    test_OptionalApi _this)
{
/* $begin(test/OptionalApi/tc_createChild) */

    test_Optional *o = test_OptionalCreateChild(root_o, "a", Set(10), 20);

    test_assert(o != NULL);
    test_assert(corto_checkAttr(o, CORTO_ATTR_SCOPED));
    test_assertstr(corto_idof(o), "a");
    test_assert(corto_parentof(o) == root_o);
    test_assert(o->a != NULL);
    test_assert(*o->a == 10);
    test_assertint(o->b, 20);

/* $end */
}

corto_void _test_OptionalApi_tc_createChildNotSet(
    test_OptionalApi _this)
{
/* $begin(test/OptionalApi/tc_createChildNotSet) */

    test_Optional *o = test_OptionalCreateChild(root_o, "a", NotSet, 20);

    test_assert(o != NULL);
    test_assert(corto_checkAttr(o, CORTO_ATTR_SCOPED));
    test_assertstr(corto_idof(o), "a");
    test_assert(corto_parentof(o) == root_o);
    test_assert(o->a == NULL);
    test_assertint(o->b, 20);

/* $end */
}

corto_void _test_OptionalApi_tc_createNotSet(
    test_OptionalApi _this)
{
/* $begin(test/OptionalApi/tc_createNotSet) */
    test_Optional *o = test_OptionalCreate(NotSet, 20);

    test_assert(o != NULL);
    test_assert(o->a == NULL);
    test_assertint(o->b, 20);

/* $end */
}

corto_void _test_OptionalApi_tc_declare(
    test_OptionalApi _this)
{
/* $begin(test/OptionalApi/tc_declare) */
    test_Optional *o = test_OptionalDeclare();

    test_assert(o != NULL);
    test_assert(!corto_checkState(o, CORTO_DEFINED));
    test_assert(o->a == NULL);
    test_assertint(o->b, 0);

/* $end */
}

corto_void _test_OptionalApi_tc_declareChild(
    test_OptionalApi _this)
{
/* $begin(test/OptionalApi/tc_declareChild) */
    test_Optional *o = test_OptionalDeclareChild(root_o, "a");

    test_assert(o != NULL);
    test_assert(!corto_checkState(o, CORTO_DEFINED));
    test_assert(corto_checkAttr(o, CORTO_ATTR_SCOPED));
    test_assertstr(corto_idof(o), "a");
    test_assert(corto_parentof(o) == root_o);
    test_assert(o->a == NULL);
    test_assertint(o->b, 0);

/* $end */
}

corto_void _test_OptionalApi_tc_define(
    test_OptionalApi _this)
{
/* $begin(test/OptionalApi/tc_define) */
    test_Optional *o = test_OptionalDeclare();

    test_assert(o != NULL);
    test_assert(!corto_checkState(o, CORTO_DEFINED));
    test_assert(o->a == NULL);
    test_assertint(o->b, 0);

    test_OptionalDefine(o, Set(10), 20);
    test_assert(o->a != NULL);
    test_assert(*o->a == 10);
    test_assertint(o->b, 20);

/* $end */
}

corto_void _test_OptionalApi_tc_defineNotSet(
    test_OptionalApi _this)
{
/* $begin(test/OptionalApi/tc_defineNotSet) */
    test_Optional *o = test_OptionalDeclare();

    test_assert(o != NULL);
    test_assert(!corto_checkState(o, CORTO_DEFINED));
    test_assert(o->a == NULL);
    test_assertint(o->b, 0);

    test_OptionalDefine(o, NotSet, 20);
    test_assert(o->a == NULL);
    test_assertint(o->b, 20);

/* $end */
}

corto_void _test_OptionalApi_tc_set(
    test_OptionalApi _this)
{
/* $begin(test/OptionalApi/tc_set) */
    test_Optional *o = test_OptionalCreate(NotSet, 20);

    test_assert(o != NULL);
    test_assert(o->a == NULL);
    test_assertint(o->b, 20);

    corto_int32Set(o->a, 10);
    test_assert(o->a != NULL);
    test_assertint(*o->a, 10);

/* $end */
}

corto_void _test_OptionalApi_tc_str(
    test_OptionalApi _this)
{
/* $begin(test/OptionalApi/tc_str) */
    test_Optional *o = test_OptionalCreate(Set(10), 20);

    corto_string str = test_OptionalStr(o);
    test_assertstr(str, "{10,20}");
    corto_dealloc(str);

/* $end */
}

corto_void _test_OptionalApi_tc_strNotSet(
    test_OptionalApi _this)
{
/* $begin(test/OptionalApi/tc_strNotSet) */
    test_Optional *o = test_OptionalCreate(NotSet, 20);

    corto_string str = test_OptionalStr(o);
    test_assertstr(str, "{20}");
    corto_dealloc(str);

/* $end */
}

corto_void _test_OptionalApi_tc_unset(
    test_OptionalApi _this)
{
/* $begin(test/OptionalApi/tc_unset) */
    test_Optional *o = test_OptionalCreate(Set(10), 20);

    test_assert(o != NULL);
    test_assert(o->a != NULL);
    test_assertint(*o->a, 10);
    test_assertint(o->b, 20);

    corto_int32Unset(o->a);
    test_assert(o->a == NULL);

/* $end */
}

corto_void _test_OptionalApi_tc_update(
    test_OptionalApi _this)
{
/* $begin(test/OptionalApi/tc_update) */
    corto_int16 result;

    test_Optional *o = test_OptionalCreate(NotSet, 20);
    test_assert(o != NULL);

    result = corto_observer_observe(test_OptionalApi_onUpdate_o, _this, o);
    test_assert(result == 0);

    result = test_OptionalUpdate(o, Set(20), 30);
    test_assert(result == 0);
    test_assert(_this->updated == TRUE);
    test_assert(o->a != NULL);
    test_assert(*o->a == 20);
    test_assert(o->b == 30);

    result = corto_observer_unobserve(test_OptionalApi_onUpdate_o, _this, o);
    test_assert(result == 0);

    corto_delete(o);

/* $end */
}

corto_void _test_OptionalApi_tc_updateNotSet(
    test_OptionalApi _this)
{
/* $begin(test/OptionalApi/tc_updateNotSet) */
    corto_int16 result;

    test_Optional *o = test_OptionalCreate(Set(10), 20);
    test_assert(o != NULL);

    result = corto_observer_observe(test_OptionalApi_onUpdate_o, _this, o);
    test_assert(result == 0);

    result = test_OptionalUpdate(o, NotSet, 30);
    test_assert(result == 0);
    test_assert(_this->updated == TRUE);
    test_assert(o->a == NULL);
    test_assert(o->b == 30);

    result = corto_observer_unobserve(test_OptionalApi_onUpdate_o, _this, o);
    test_assert(result == 0);

    corto_delete(o);

/* $end */
}
