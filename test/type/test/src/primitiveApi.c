/* $CORTO_GENERATED
 *
 * primitiveApi.c
 *
 * Only code written between the begin and end tags will be preserved
 * when the file is regenerated.
 */

#include "test.h"

corto_void _test_primitiveApi_onUpdate(test_primitiveApi this, corto_object observable) {
/* $begin(test/primitiveApi/onUpdate) */
    this->updated = TRUE;
/* $end */
}

corto_void _test_primitiveApi_tc_compare(test_primitiveApi this) {
/* $begin(test/primitiveApi/tc_compare) */
    corto_equalityKind eq;

    eq = type_primitiveTypeCompare(10, 20);
    test_assert(eq == CORTO_LT);

    eq = type_primitiveTypeCompare(20, 10);
    test_assert(eq == CORTO_GT);

    eq = type_primitiveTypeCompare(10, 10);
    test_assert(eq == CORTO_EQ);

/* $end */
}

corto_void _test_primitiveApi_tc_create(test_primitiveApi this) {
/* $begin(test/primitiveApi/tc_create) */

    type_primitiveType *o = type_primitiveTypeCreate(10);
    test_assert(o != NULL);
    test_assert(corto_typeof(o) == corto_type(type_primitiveType_o));
    test_assert(corto_checkState(o, CORTO_DEFINED));
    test_assert(*o == 10);
    test_assert(!corto_delete(o));

/* $end */
}

corto_void _test_primitiveApi_tc_createAuto(test_primitiveApi this) {
/* $begin(test/primitiveApi/tc_createAuto) */

    type_primitiveTypeCreate_auto(o, 10);
    test_assert(o != NULL);
    test_assert(corto_typeof(o) == corto_type(type_primitiveType_o));
    test_assert(corto_checkState(o, CORTO_DEFINED));
    test_assert(*o == 10);
    test_assert(!corto_delete(o));

/* $end */
}

corto_void _test_primitiveApi_tc_createChild(test_primitiveApi this) {
/* $begin(test/primitiveApi/tc_createChild) */

    type_primitiveType *o = type_primitiveTypeCreateChild(root_o, "o", 10);
    test_assert(o != NULL);
    test_assert(corto_typeof(o) == corto_type(type_primitiveType_o));
    test_assert(corto_checkState(o, CORTO_DEFINED));
    test_assert(corto_checkAttr(o, CORTO_ATTR_SCOPED));
    test_assert(corto_parentof(o) == root_o);
    test_assert(!strcmp(corto_nameof(o), "o"));
    test_assert(*o == 10);
    test_assert(!corto_delete(o));

/* $end */
}

corto_void _test_primitiveApi_tc_createChildAuto(test_primitiveApi this) {
/* $begin(test/primitiveApi/tc_createChildAuto) */

    type_primitiveTypeCreateChild_auto(root_o, o, 10);
    test_assert(o != NULL);
    test_assert(corto_typeof(o) == corto_type(type_primitiveType_o));
    test_assert(corto_checkState(o, CORTO_DEFINED));
    test_assert(corto_checkAttr(o, CORTO_ATTR_SCOPED));
    test_assert(corto_parentof(o) == root_o);
    test_assert(!strcmp(corto_nameof(o), "o"));
    test_assert(*o == 10);
    test_assert(!corto_delete(o));

/* $end */
}

corto_void _test_primitiveApi_tc_declare(test_primitiveApi this) {
/* $begin(test/primitiveApi/tc_declare) */

    type_primitiveType *o = type_primitiveTypeDeclare();
    test_assert(o != NULL);
    test_assert(corto_typeof(o) == corto_type(type_primitiveType_o));
    test_assert(!corto_checkState(o, CORTO_DEFINED));
    test_assert(*o == 5);
    test_assert(!corto_delete(o));

/* $end */
}

corto_void _test_primitiveApi_tc_declareAuto(test_primitiveApi this) {
/* $begin(test/primitiveApi/tc_declareAuto) */

    type_primitiveTypeDeclare_auto(o);
    test_assert(o != NULL);
    test_assert(corto_typeof(o) == corto_type(type_primitiveType_o));
    test_assert(!corto_checkState(o, CORTO_DEFINED));
    test_assert(*o == 5);
    test_assert(!corto_delete(o));

/* $end */
}

corto_void _test_primitiveApi_tc_declareChild(test_primitiveApi this) {
/* $begin(test/primitiveApi/tc_declareChild) */

    type_primitiveType *o = type_primitiveTypeDeclareChild(root_o, "o");
    test_assert(o != NULL);
    test_assert(corto_typeof(o) == corto_type(type_primitiveType_o));
    test_assert(!corto_checkState(o, CORTO_DEFINED));
    test_assert(corto_checkAttr(o, CORTO_ATTR_SCOPED));
    test_assert(corto_parentof(o) == root_o);
    test_assert(!strcmp(corto_nameof(o), "o"));
    test_assert(*o == 5);
    test_assert(!corto_delete(o));

/* $end */
}

corto_void _test_primitiveApi_tc_declareChildAuto(test_primitiveApi this) {
/* $begin(test/primitiveApi/tc_declareChildAuto) */

    type_primitiveTypeDeclareChild_auto(root_o, o);
    test_assert(o != NULL);
    test_assert(corto_typeof(o) == corto_type(type_primitiveType_o));
    test_assert(!corto_checkState(o, CORTO_DEFINED));
    test_assert(corto_checkAttr(o, CORTO_ATTR_SCOPED));
    test_assert(corto_parentof(o) == root_o);
    test_assert(!strcmp(corto_nameof(o), "o"));
    test_assert(*o == 5);
    test_assert(!corto_delete(o));

/* $end */
}

corto_void _test_primitiveApi_tc_define(test_primitiveApi this) {
/* $begin(test/primitiveApi/tc_define) */
    corto_int16 result;

    type_primitiveType *o = type_primitiveTypeDeclareChild(root_o, "o");
    test_assert(o != NULL);
    test_assert(corto_typeof(o) == corto_type(type_primitiveType_o));
    test_assert(!corto_checkState(o, CORTO_DEFINED));
    test_assert(corto_checkAttr(o, CORTO_ATTR_SCOPED));
    test_assert(corto_parentof(o) == root_o);
    test_assert(!strcmp(corto_nameof(o), "o"));
    test_assert(*o == 5);

    result = type_primitiveTypeDefine(o, 10);
    test_assert(result == 0);
    test_assert(corto_checkState(o, CORTO_DEFINED));
    test_assert(*o == 10);

    test_assert(!corto_delete(o));

/* $end */
}

corto_void _test_primitiveApi_tc_fromStr(test_primitiveApi this) {
/* $begin(test/primitiveApi/tc_fromStr) */

    type_primitiveType *o = type_primitiveTypeCreate(10);
    test_assert(o != NULL);
    type_primitiveTypeFromStr(o, "20");
    test_assert(*o == 20);
    test_assert(!corto_delete(o));

/* $end */
}

corto_void _test_primitiveApi_tc_set(test_primitiveApi this) {
/* $begin(test/primitiveApi/tc_set) */

    type_primitiveType *o = type_primitiveTypeCreate(10);
    test_assert(o != NULL);
    type_primitiveTypeSet(o, 20);
    test_assert(*o == 20);
    test_assert(!corto_delete(o));

/* $end */
}

corto_void _test_primitiveApi_tc_str(test_primitiveApi this) {
/* $begin(test/primitiveApi/tc_str) */

    type_primitiveType *o = type_primitiveTypeCreate(10);
    test_assert(o != NULL);
    corto_string str = type_primitiveTypeStr(*o);
    test_assert(!strcmp(str, "10"));
    corto_dealloc(str);
    test_assert(!corto_delete(o));

/* $end */
}

corto_void _test_primitiveApi_tc_update(test_primitiveApi this) {
/* $begin(test/primitiveApi/tc_update) */
    corto_int16 result;

    type_primitiveType *o = type_primitiveTypeCreate(10);
    test_assert(o != NULL);

    result = corto_listen(this, test_primitiveApi_onUpdate_o, CORTO_ON_UPDATE, o, NULL);
    test_assert(result == 0);

    result = type_primitiveTypeUpdate(o, 20);
    test_assert(result == 0);
    test_assert(this->updated == TRUE);
    test_assert(*o == 20);

    result = corto_silence(this, test_primitiveApi_onUpdate_o, CORTO_ON_UPDATE, o);
    test_assert(result == 0);

    corto_delete(o);

/* $end */
}
