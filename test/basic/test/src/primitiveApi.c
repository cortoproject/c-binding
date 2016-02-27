/* $CORTO_GENERATED
 *
 * primitiveApi.c
 *
 * Only code written between the begin and end tags will be preserved
 * when the file is regenerated.
 */

#include "test.h"

corto_void _test_primitiveApi_onUpdate(
    test_primitiveApi this,
    corto_object observable)
{
/* $begin(test/primitiveApi/onUpdate) */
    this->updated = TRUE;
/* $end */
}

corto_void _test_primitiveApi_tc_compare(
    test_primitiveApi this)
{
/* $begin(test/primitiveApi/tc_compare) */
    corto_equalityKind eq;

    eq = basic_primitiveTypeCompare(10, 20);
    test_assert(eq == CORTO_LT);

    eq = basic_primitiveTypeCompare(20, 10);
    test_assert(eq == CORTO_GT);

    eq = basic_primitiveTypeCompare(10, 10);
    test_assert(eq == CORTO_EQ);

/* $end */
}

corto_void _test_primitiveApi_tc_create(
    test_primitiveApi this)
{
/* $begin(test/primitiveApi/tc_create) */

    basic_primitiveType *o = basic_primitiveTypeCreate(10);
    test_assert(o != NULL);
    test_assert(corto_typeof(o) == corto_type(basic_primitiveType_o));
    test_assert(corto_checkState(o, CORTO_DEFINED));
    test_assert(*o == 10);
    test_assert(!corto_delete(o));

/* $end */
}

corto_void _test_primitiveApi_tc_createAuto(
    test_primitiveApi this)
{
/* $begin(test/primitiveApi/tc_createAuto) */

    basic_primitiveTypeCreate_auto(o, 10);
    test_assert(o != NULL);
    test_assert(corto_typeof(o) == corto_type(basic_primitiveType_o));
    test_assert(corto_checkState(o, CORTO_DEFINED));
    test_assert(*o == 10);
    test_assert(!corto_delete(o));

/* $end */
}

corto_void _test_primitiveApi_tc_createChild(
    test_primitiveApi this)
{
/* $begin(test/primitiveApi/tc_createChild) */

    basic_primitiveType *o = basic_primitiveTypeCreateChild(root_o, "o", 10);
    test_assert(o != NULL);
    test_assert(corto_typeof(o) == corto_type(basic_primitiveType_o));
    test_assert(corto_checkState(o, CORTO_DEFINED));
    test_assert(corto_checkAttr(o, CORTO_ATTR_SCOPED));
    test_assert(corto_parentof(o) == root_o);
    test_assert(!strcmp(corto_nameof(o), "o"));
    test_assert(*o == 10);
    test_assert(!corto_delete(o));

/* $end */
}

corto_void _test_primitiveApi_tc_createChildAuto(
    test_primitiveApi this)
{
/* $begin(test/primitiveApi/tc_createChildAuto) */

    basic_primitiveTypeCreateChild_auto(root_o, o, 10);
    test_assert(o != NULL);
    test_assert(corto_typeof(o) == corto_type(basic_primitiveType_o));
    test_assert(corto_checkState(o, CORTO_DEFINED));
    test_assert(corto_checkAttr(o, CORTO_ATTR_SCOPED));
    test_assert(corto_parentof(o) == root_o);
    test_assert(!strcmp(corto_nameof(o), "o"));
    test_assert(*o == 10);
    test_assert(!corto_delete(o));

/* $end */
}

corto_void _test_primitiveApi_tc_declare(
    test_primitiveApi this)
{
/* $begin(test/primitiveApi/tc_declare) */

    basic_primitiveType *o = basic_primitiveTypeDeclare();
    test_assert(o != NULL);
    test_assert(corto_typeof(o) == corto_type(basic_primitiveType_o));
    test_assert(!corto_checkState(o, CORTO_DEFINED));
    test_assert(*o == 5);
    test_assert(!corto_delete(o));

/* $end */
}

corto_void _test_primitiveApi_tc_declareAuto(
    test_primitiveApi this)
{
/* $begin(test/primitiveApi/tc_declareAuto) */

    basic_primitiveTypeDeclare_auto(o);
    test_assert(o != NULL);
    test_assert(corto_typeof(o) == corto_type(basic_primitiveType_o));
    test_assert(!corto_checkState(o, CORTO_DEFINED));
    test_assert(*o == 5);
    test_assert(!corto_delete(o));

/* $end */
}

corto_void _test_primitiveApi_tc_declareChild(
    test_primitiveApi this)
{
/* $begin(test/primitiveApi/tc_declareChild) */

    basic_primitiveType *o = basic_primitiveTypeDeclareChild(root_o, "o");
    test_assert(o != NULL);
    test_assert(corto_typeof(o) == corto_type(basic_primitiveType_o));
    test_assert(!corto_checkState(o, CORTO_DEFINED));
    test_assert(corto_checkAttr(o, CORTO_ATTR_SCOPED));
    test_assert(corto_parentof(o) == root_o);
    test_assert(!strcmp(corto_nameof(o), "o"));
    test_assert(*o == 5);
    test_assert(!corto_delete(o));

/* $end */
}

corto_void _test_primitiveApi_tc_declareChildAuto(
    test_primitiveApi this)
{
/* $begin(test/primitiveApi/tc_declareChildAuto) */

    basic_primitiveTypeDeclareChild_auto(root_o, o);
    test_assert(o != NULL);
    test_assert(corto_typeof(o) == corto_type(basic_primitiveType_o));
    test_assert(!corto_checkState(o, CORTO_DEFINED));
    test_assert(corto_checkAttr(o, CORTO_ATTR_SCOPED));
    test_assert(corto_parentof(o) == root_o);
    test_assert(!strcmp(corto_nameof(o), "o"));
    test_assert(*o == 5);
    test_assert(!corto_delete(o));

/* $end */
}

corto_void _test_primitiveApi_tc_define(
    test_primitiveApi this)
{
/* $begin(test/primitiveApi/tc_define) */
    corto_int16 result;

    basic_primitiveType *o = basic_primitiveTypeDeclareChild(root_o, "o");
    test_assert(o != NULL);
    test_assert(corto_typeof(o) == corto_type(basic_primitiveType_o));
    test_assert(!corto_checkState(o, CORTO_DEFINED));
    test_assert(corto_checkAttr(o, CORTO_ATTR_SCOPED));
    test_assert(corto_parentof(o) == root_o);
    test_assert(!strcmp(corto_nameof(o), "o"));
    test_assert(*o == 5);

    result = basic_primitiveTypeDefine(o, 10);
    test_assert(result == 0);
    test_assert(corto_checkState(o, CORTO_DEFINED));
    test_assert(*o == 10);

    test_assert(!corto_delete(o));

/* $end */
}

corto_void _test_primitiveApi_tc_fromStr(
    test_primitiveApi this)
{
/* $begin(test/primitiveApi/tc_fromStr) */

    basic_primitiveType *o = basic_primitiveTypeCreate(10);
    test_assert(o != NULL);
    basic_primitiveTypeFromStr(o, "20");
    test_assert(*o == 20);
    test_assert(!corto_delete(o));

/* $end */
}

corto_void _test_primitiveApi_tc_set(
    test_primitiveApi this)
{
/* $begin(test/primitiveApi/tc_set) */

    basic_primitiveType *o = basic_primitiveTypeCreate(10);
    test_assert(o != NULL);
    basic_primitiveTypeSet(o, 20);
    test_assert(*o == 20);
    test_assert(!corto_delete(o));

/* $end */
}

corto_void _test_primitiveApi_tc_str(
    test_primitiveApi this)
{
/* $begin(test/primitiveApi/tc_str) */

    basic_primitiveType *o = basic_primitiveTypeCreate(10);
    test_assert(o != NULL);
    corto_string str = basic_primitiveTypeStr(*o);
    test_assert(!strcmp(str, "10"));
    corto_dealloc(str);
    test_assert(!corto_delete(o));

/* $end */
}

corto_void _test_primitiveApi_tc_update(
    test_primitiveApi this)
{
/* $begin(test/primitiveApi/tc_update) */
    corto_int16 result;

    basic_primitiveType *o = basic_primitiveTypeCreate(10);
    test_assert(o != NULL);

    result = corto_listen(this, test_primitiveApi_onUpdate_o, CORTO_ON_UPDATE, o, NULL);
    test_assert(result == 0);

    result = basic_primitiveTypeUpdate(o, 20);
    test_assert(result == 0);
    test_assert(this->updated == TRUE);
    test_assert(*o == 20);

    result = corto_silence(this, test_primitiveApi_onUpdate_o, CORTO_ON_UPDATE, o);
    test_assert(result == 0);

    corto_delete(o);

/* $end */
}
