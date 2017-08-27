/* This is a managed file. Do not delete this comment. */

#include <include/test.h>

void test_primitiveApi_onUpdate(
    corto_observerEvent *e)
{
    this->updated = TRUE;
}

void test_primitiveApi_tc_compare(
    test_primitiveApi this)
{
    corto_equalityKind eq;

    eq = test_primitiveTypeCompare(10, 20);
    test_assert(eq == CORTO_LT);

    eq = test_primitiveTypeCompare(20, 10);
    test_assert(eq == CORTO_GT);

    eq = test_primitiveTypeCompare(10, 10);
    test_assert(eq == CORTO_EQ);

}

void test_primitiveApi_tc_create(
    test_primitiveApi this)
{

    test_primitiveType *o = test_primitiveTypeCreate(10);
    test_assert(o != NULL);
    test_assert(corto_typeof(o) == corto_type(test_primitiveType_o));
    test_assert(corto_checkState(o, CORTO_VALID));
    test_assert(*o == 10);
    test_assert(!corto_delete(o));

}

void test_primitiveApi_tc_createAuto(
    test_primitiveApi this)
{

    test_primitiveTypeCreate_auto(o, 10);
    test_assert(o != NULL);
    test_assert(corto_typeof(o) == corto_type(test_primitiveType_o));
    test_assert(corto_checkState(o, CORTO_VALID));
    test_assert(*o == 10);
    test_assert(!corto_delete(o));

}

void test_primitiveApi_tc_createChild(
    test_primitiveApi this)
{

    test_primitiveType *o = test_primitiveTypeCreateChild(root_o, "o", 10);
    test_assert(o != NULL);
    test_assert(corto_typeof(o) == corto_type(test_primitiveType_o));
    test_assert(corto_checkState(o, CORTO_VALID));
    test_assert(corto_checkAttr(o, CORTO_ATTR_NAMED));
    test_assert(corto_parentof(o) == root_o);
    test_assert(!strcmp(corto_idof(o), "o"));
    test_assert(*o == 10);
    test_assert(!corto_delete(o));

}

void test_primitiveApi_tc_createChildAuto(
    test_primitiveApi this)
{

    test_primitiveTypeCreateChild_auto(root_o, o, 10);
    test_assert(o != NULL);
    test_assert(corto_typeof(o) == corto_type(test_primitiveType_o));
    test_assert(corto_checkState(o, CORTO_VALID));
    test_assert(corto_checkAttr(o, CORTO_ATTR_NAMED));
    test_assert(corto_parentof(o) == root_o);
    test_assert(!strcmp(corto_idof(o), "o"));
    test_assert(*o == 10);
    test_assert(!corto_delete(o));

}

void test_primitiveApi_tc_declare(
    test_primitiveApi this)
{

    test_primitiveType *o = test_primitiveTypeDeclare();
    test_assert(o != NULL);
    test_assert(corto_typeof(o) == corto_type(test_primitiveType_o));
    test_assert(!corto_checkState(o, CORTO_VALID));
    test_assert(*o == 5);
    test_assert(!corto_delete(o));

}

void test_primitiveApi_tc_declareAuto(
    test_primitiveApi this)
{

    test_primitiveTypeDeclare_auto(o);
    test_assert(o != NULL);
    test_assert(corto_typeof(o) == corto_type(test_primitiveType_o));
    test_assert(!corto_checkState(o, CORTO_VALID));
    test_assert(*o == 5);
    test_assert(!corto_delete(o));

}

void test_primitiveApi_tc_declareChild(
    test_primitiveApi this)
{

    test_primitiveType *o = test_primitiveTypeDeclareChild(root_o, "o");
    test_assert(o != NULL);
    test_assert(corto_typeof(o) == corto_type(test_primitiveType_o));
    test_assert(!corto_checkState(o, CORTO_VALID));
    test_assert(corto_checkAttr(o, CORTO_ATTR_NAMED));
    test_assert(corto_parentof(o) == root_o);
    test_assert(!strcmp(corto_idof(o), "o"));
    test_assert(*o == 5);
    test_assert(!corto_delete(o));

}

void test_primitiveApi_tc_declareChildAuto(
    test_primitiveApi this)
{

    test_primitiveTypeDeclareChild_auto(root_o, o);
    test_assert(o != NULL);
    test_assert(corto_typeof(o) == corto_type(test_primitiveType_o));
    test_assert(!corto_checkState(o, CORTO_VALID));
    test_assert(corto_checkAttr(o, CORTO_ATTR_NAMED));
    test_assert(corto_parentof(o) == root_o);
    test_assert(!strcmp(corto_idof(o), "o"));
    test_assert(*o == 5);
    test_assert(!corto_delete(o));

}

void test_primitiveApi_tc_define(
    test_primitiveApi this)
{
    corto_int16 result;

    test_primitiveType *o = test_primitiveTypeDeclareChild(root_o, "o");
    test_assert(o != NULL);
    test_assert(corto_typeof(o) == corto_type(test_primitiveType_o));
    test_assert(!corto_checkState(o, CORTO_VALID));
    test_assert(corto_checkAttr(o, CORTO_ATTR_NAMED));
    test_assert(corto_parentof(o) == root_o);
    test_assert(!strcmp(corto_idof(o), "o"));
    test_assert(*o == 5);

    result = test_primitiveTypeDefine(o, 10);
    test_assert(result == 0);
    test_assert(corto_checkState(o, CORTO_VALID));
    test_assert(*o == 10);

    test_assert(!corto_delete(o));

}

void test_primitiveApi_tc_fromStr(
    test_primitiveApi this)
{

    test_primitiveType *o = test_primitiveTypeCreate(10);
    test_assert(o != NULL);
    test_primitiveTypeFromStr(o, "20");
    test_assert(*o == 20);
    test_assert(!corto_delete(o));

}

void test_primitiveApi_tc_set(
    test_primitiveApi this)
{

    test_primitiveType *o = test_primitiveTypeCreate(10);
    test_assert(o != NULL);
    test_primitiveTypeAssign(o, 20);
    test_assert(*o == 20);
    test_assert(!corto_delete(o));

}

void test_primitiveApi_tc_str(
    test_primitiveApi this)
{

    test_primitiveType *o = test_primitiveTypeCreate(10);
    test_assert(o != NULL);
    corto_string str = test_primitiveTypeStr(*o);
    test_assert(!strcmp(str, "10"));
    corto_dealloc(str);
    test_assert(!corto_delete(o));

}

void test_primitiveApi_tc_update(
    test_primitiveApi this)
{
    corto_int16 result;

    test_primitiveType *o = test_primitiveTypeCreate(10);
    test_assert(o != NULL);

    result = corto_observer_observe(test_primitiveApi_onUpdate_o, this, o);
    test_assert(result == 0);

    result = test_primitiveTypeUpdate(o, 20);
    test_assert(result == 0);
    test_assert(this->updated == TRUE);
    test_assert(*o == 20);

    result = corto_observer_unobserve(test_primitiveApi_onUpdate_o, this, o);
    test_assert(result == 0);

    corto_delete(o);

}

