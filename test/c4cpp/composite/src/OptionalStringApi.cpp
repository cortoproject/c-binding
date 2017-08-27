/* $CORTO_GENERATED
 *
 * OptionalStringApi.cpp
 *
 * Only code written between the begin and end tags will be preserved
 * when the file is regenerated.
 */

#include <include/test.h>

corto_void _test_OptionalStringApi_onUpdate(
    test_OptionalStringApi _this,
    corto_eventMask event,
    corto_object object,
    corto_observer observer)
{
/* $begin(test/OptionalStringApi/onUpdate) */

    _this->updated = TRUE;

/* $end */
}

corto_void _test_OptionalStringApi_tc_create(
    test_OptionalStringApi _this)
{
/* $begin(test/OptionalStringApi/tc_create) */
    test_OptionalString *o = test_OptionalStringCreate(Set("Foo"), "Bar");

    test_assert(o != NULL);
    test_assert(o->a != NULL);
    test_assertstr(*o->a, "Foo");
    test_assertstr(o->b, "Bar");

/* $end */
}

corto_void _test_OptionalStringApi_tc_createChild(
    test_OptionalStringApi _this)
{
/* $begin(test/OptionalStringApi/tc_createChild) */
    test_OptionalString *o = test_OptionalStringCreateChild(
        root_o, "a", Set("Foo"), "Bar");

    test_assert(o != NULL);
    test_assert(corto_checkAttr(o, CORTO_ATTR_NAMED));
    test_assertstr(corto_idof(o), "a");
    test_assert(corto_parentof(o) == root_o);
    test_assert(o->a != NULL);
    test_assertstr(*o->a, "Foo");
    test_assertstr(o->b, "Bar");

/* $end */
}

corto_void _test_OptionalStringApi_tc_createChildNotSet(
    test_OptionalStringApi _this)
{
/* $begin(test/OptionalStringApi/tc_createChildNotSet) */
    test_OptionalString *o = test_OptionalStringCreateChild(
        root_o, "a", NotSet, "Bar");

    test_assert(o != NULL);
    test_assert(corto_checkAttr(o, CORTO_ATTR_NAMED));
    test_assertstr(corto_idof(o), "a");
    test_assert(corto_parentof(o) == root_o);
    test_assert(o->a == NULL);
    test_assertstr(o->b, "Bar");

/* $end */
}

corto_void _test_OptionalStringApi_tc_createNotSet(
    test_OptionalStringApi _this)
{
/* $begin(test/OptionalStringApi/tc_createNotSet) */
    test_OptionalString *o = test_OptionalStringCreate(NotSet, "Bar");

    test_assert(o != NULL);
    test_assert(o->a == NULL);
    test_assertstr(o->b, "Bar");

/* $end */
}

corto_void _test_OptionalStringApi_tc_declare(
    test_OptionalStringApi _this)
{
/* $begin(test/OptionalStringApi/tc_declare) */
    test_OptionalString *o = test_OptionalStringDeclare();

    test_assert(o != NULL);
    test_assert(!corto_checkState(o, CORTO_VALID));
    test_assert(o->a == NULL);
    test_assert(o->b == NULL);

/* $end */
}

corto_void _test_OptionalStringApi_tc_declareChild(
    test_OptionalStringApi _this)
{
/* $begin(test/OptionalStringApi/tc_declareChild) */
    test_OptionalString *o = test_OptionalStringDeclareChild(root_o, "a");

    test_assert(o != NULL);
    test_assert(!corto_checkState(o, CORTO_VALID));
    test_assert(corto_checkAttr(o, CORTO_ATTR_NAMED));
    test_assertstr(corto_idof(o), "a");
    test_assert(corto_parentof(o) == root_o);
    test_assert(o->a == NULL);
    test_assert(o->b == NULL);

/* $end */
}

corto_void _test_OptionalStringApi_tc_define(
    test_OptionalStringApi _this)
{
/* $begin(test/OptionalStringApi/tc_define) */
    test_OptionalString *o = test_OptionalStringDeclare();

    test_assert(o != NULL);
    test_assert(!corto_checkState(o, CORTO_VALID));
    test_assert(o->a == NULL);
    test_assert(o->b == NULL);

    test_OptionalStringDefine(o, Set("Foo"), "Bar");
    test_assert(o->a != NULL);
    test_assertstr(*o->a, "Foo");
    test_assertstr(o->b, "Bar");

/* $end */
}

corto_void _test_OptionalStringApi_tc_defineNotSet(
    test_OptionalStringApi _this)
{
/* $begin(test/OptionalStringApi/tc_defineNotSet) */
    test_OptionalString *o = test_OptionalStringDeclare();

    test_assert(o != NULL);
    test_assert(!corto_checkState(o, CORTO_VALID));
    test_assert(o->a == NULL);
    test_assert(o->b == NULL);

    test_OptionalStringDefine(o, NotSet, "Bar");
    test_assert(o->a == NULL);
    test_assertstr(o->b, "Bar");

/* $end */
}

corto_void _test_OptionalStringApi_tc_set(
    test_OptionalStringApi _this)
{
/* $begin(test/OptionalStringApi/tc_set) */
    test_OptionalString *o = test_OptionalStringCreate(NotSet, "Bar");

    test_assert(o != NULL);
    test_assert(o->a == NULL);
    test_assertstr(o->b, "Bar");

    corto_stringSet(o->a, "Foo");
    test_assert(o->a != NULL);
    test_assertstr(*o->a, "Foo");

/* $end */
}

corto_void _test_OptionalStringApi_tc_str(
    test_OptionalStringApi _this)
{
/* $begin(test/OptionalStringApi/tc_str) */
    test_OptionalString *o = test_OptionalStringCreate(Set("Foo"), "Bar");

    corto_string str = test_OptionalStringStr(o);
    test_assertstr(str, "{\"Foo\",\"Bar\"}");
    corto_dealloc(str);

/* $end */
}

corto_void _test_OptionalStringApi_tc_strNotSet(
    test_OptionalStringApi _this)
{
/* $begin(test/OptionalStringApi/tc_strNotSet) */
    test_OptionalString *o = test_OptionalStringCreate(NotSet, "Bar");

    corto_string str = test_OptionalStringStr(o);
    test_assertstr(str, "{\"Bar\"}");
    corto_dealloc(str);

/* $end */
}

corto_void _test_OptionalStringApi_tc_unset(
    test_OptionalStringApi _this)
{
/* $begin(test/OptionalStringApi/tc_unset) */
    test_OptionalString *o = test_OptionalStringCreate(Set("Foo"), "Bar");

    test_assert(o != NULL);
    test_assert(o->a != NULL);
    test_assertstr(*o->a, "Foo");
    test_assertstr(o->b, "Bar");

    corto_stringUnset(o->a);
    test_assert(o->a == NULL);

/* $end */
}

corto_void _test_OptionalStringApi_tc_update(
    test_OptionalStringApi _this)
{
/* $begin(test/OptionalStringApi/tc_update) */
    corto_int16 result;

    test_OptionalString *o = test_OptionalStringCreate(NotSet, "Bar");
    test_assert(o != NULL);

    result = corto_observer_observe(test_OptionalStringApi_onUpdate_o, _this, o);
    test_assert(result == 0);

    result = test_OptionalStringUpdate(o, Set("Foo"), "Bar");
    test_assert(result == 0);
    test_assert(_this->updated == TRUE);
    test_assert(o->a != NULL);
    test_assertstr(*o->a, "Foo");
    test_assertstr(o->b, "Bar");

    result = corto_observer_unobserve(test_OptionalStringApi_onUpdate_o, _this, o);
    test_assert(result == 0);

    corto_delete(o);

/* $end */
}

corto_void _test_OptionalStringApi_tc_updateNotSet(
    test_OptionalStringApi _this)
{
/* $begin(test/OptionalStringApi/tc_updateNotSet) */
    corto_int16 result;

    test_OptionalString *o = test_OptionalStringCreate(Set("Foo"), "Bar");
    test_assert(o != NULL);

    result = corto_observer_observe(test_OptionalStringApi_onUpdate_o, _this, o);
    test_assert(result == 0);

    result = test_OptionalStringUpdate(o, NotSet, "Bar");
    test_assert(result == 0);
    test_assert(_this->updated == TRUE);
    test_assert(o->a == NULL);
    test_assertstr(o->b, "Bar");

    result = corto_observer_unobserve(test_OptionalStringApi_onUpdate_o, _this, o);
    test_assert(result == 0);

    corto_delete(o);

/* $end */
}
