/* $CORTO_GENERATED
 *
 * voidApi.c
 *
 * Only code written between the begin and end tags will be preserved
 * when the file is regenerated.
 */

#include "test.h"

corto_void _test_voidApi_onUpdate(test_voidApi this, corto_object observable) {
/* $begin(test/voidApi/onUpdate) */
    this->updated = TRUE;
/* $end */
}

corto_void _test_voidApi_tc_create(test_voidApi this) {
/* $begin(test/voidApi/tc_create) */

    type_voidType *o = type_voidTypeCreate();
    test_assert(o != NULL);
    test_assert(corto_typeof(o) == type_voidType_o);
    test_assert(corto_checkState(o, CORTO_DEFINED));
    test_assert(!corto_delete(o));

/* $end */
}

corto_void _test_voidApi_tc_createAuto(test_voidApi this) {
/* $begin(test/voidApi/tc_createAuto) */

    type_voidTypeCreate_auto(o);
    test_assert(o != NULL);
    test_assert(corto_typeof(o) == type_voidType_o);
    test_assert(corto_checkState(o, CORTO_DEFINED));
    test_assert(!corto_delete(o));

/* $end */
}

corto_void _test_voidApi_tc_createChild(test_voidApi this) {
/* $begin(test/voidApi/tc_createChild) */

    type_voidType *o = type_voidTypeCreateChild(root_o, "o");
    test_assert(o != NULL);
    test_assert(corto_typeof(o) == type_voidType_o);
    test_assert(corto_checkState(o, CORTO_DEFINED));
    test_assert(corto_checkAttr(o, CORTO_ATTR_SCOPED));
    test_assert(corto_parentof(o) == root_o);
    test_assert(!strcmp(corto_nameof(o), "o"));
    test_assert(!corto_delete(o));

/* $end */
}

corto_void _test_voidApi_tc_createChildAuto(test_voidApi this) {
/* $begin(test/voidApi/tc_createChildAuto) */

    type_voidTypeCreateChild_auto(root_o, o);
    test_assert(o != NULL);
    test_assert(corto_typeof(o) == type_voidType_o);
    test_assert(corto_checkState(o, CORTO_DEFINED));
    test_assert(corto_checkAttr(o, CORTO_ATTR_SCOPED));
    test_assert(corto_parentof(o) == root_o);
    test_assert(!strcmp(corto_nameof(o), "o"));
    test_assert(!corto_delete(o));

/* $end */
}

corto_void _test_voidApi_tc_update(test_voidApi this) {
/* $begin(test/voidApi/tc_update) */
    corto_int16 result;

    type_voidType *o = type_voidTypeCreate();
    test_assert(o != NULL);

    result = corto_listen(this, test_voidApi_onUpdate_o, CORTO_ON_UPDATE, o, NULL);
    test_assert(result == 0);

    result = type_voidTypeUpdate(o);
    test_assert(result == 0);
    test_assert(this->updated == TRUE);

    result = corto_silence(this, test_voidApi_onUpdate_o, CORTO_ON_UPDATE, o);
    test_assert(result == 0);

    corto_delete(o);

/* $end */
}
