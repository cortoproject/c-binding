/* This is a managed file. Do not delete this comment. */

#include <include/test.h>

void test_voidReferenceApi_onUpdate(
    corto_observerEvent *e)
{
    this->updated = TRUE;
}

void test_voidReferenceApi_tc_create(
    test_voidReferenceApi this)
{

    test_voidReferenceType o = test_voidReferenceTypeCreate();
    test_assert(o != NULL);
    test_assert(corto_typeof(o) == test_voidReferenceType_o);
    test_assert(corto_checkState(o, CORTO_VALID));
    test_assert(!corto_delete(o));

}

void test_voidReferenceApi_tc_createAuto(
    test_voidReferenceApi this)
{

    test_voidReferenceTypeCreate_auto(o);
    test_assert(o != NULL);
    test_assert(corto_typeof(o) == test_voidReferenceType_o);
    test_assert(corto_checkState(o, CORTO_VALID));
    test_assert(!corto_delete(o));

}

void test_voidReferenceApi_tc_createChild(
    test_voidReferenceApi this)
{

    test_voidReferenceType o = test_voidReferenceTypeCreateChild(root_o, "o");
    test_assert(o != NULL);
    test_assert(corto_typeof(o) == test_voidReferenceType_o);
    test_assert(corto_checkState(o, CORTO_VALID));
    test_assert(corto_checkAttr(o, CORTO_ATTR_NAMED));
    test_assert(corto_parentof(o) == root_o);
    test_assert(!strcmp(corto_idof(o), "o"));
    test_assert(!corto_delete(o));

}

void test_voidReferenceApi_tc_createChildAuto(
    test_voidReferenceApi this)
{

    test_voidReferenceTypeCreateChild_auto(root_o, o);
    test_assert(o != NULL);
    test_assert(corto_typeof(o) == test_voidReferenceType_o);
    test_assert(corto_checkState(o, CORTO_VALID));
    test_assert(corto_checkAttr(o, CORTO_ATTR_NAMED));
    test_assert(corto_parentof(o) == root_o);
    test_assert(!strcmp(corto_idof(o), "o"));
    test_assert(!corto_delete(o));

}

void test_voidReferenceApi_tc_update(
    test_voidReferenceApi this)
{
    corto_int16 result;

    test_voidReferenceType o = test_voidReferenceTypeCreate();
    test_assert(o != NULL);

    result = corto_observer_observe(test_voidReferenceApi_onUpdate_o, this, o);
    test_assert(result == 0);

    result = test_voidReferenceTypeUpdate(o);
    test_assert(result == 0);
    test_assert(this->updated == TRUE);

    result = corto_observer_unobserve(test_voidReferenceApi_onUpdate_o, this, o);
    test_assert(result == 0);

    corto_delete(o);

}

