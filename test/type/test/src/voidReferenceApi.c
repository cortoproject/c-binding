/* $CORTO_GENERATED
 *
 * voidReferenceApi.c
 *
 * Only code written between the begin and end tags will be preserved
 * when the file is regenerated.
 */

#include "test.h"

corto_void _test_voidReferenceApi_onUpdate(test_voidReferenceApi this, corto_object observable) {
/* $begin(test/voidReferenceApi/onUpdate) */
    this->updated = TRUE;
/* $end */
}

corto_void _test_voidReferenceApi_tc_create(test_voidReferenceApi this) {
/* $begin(test/voidReferenceApi/tc_create) */

    type_voidReferenceType o = type_voidReferenceTypeCreate();
    test_assert(o != NULL);
    test_assert(corto_typeof(o) == type_voidReferenceType_o);
    test_assert(corto_checkState(o, CORTO_DEFINED));
    test_assert(!corto_delete(o));

/* $end */
}

corto_void _test_voidReferenceApi_tc_createAuto(test_voidReferenceApi this) {
/* $begin(test/voidReferenceApi/tc_createAuto) */

    type_voidReferenceTypeCreate_auto(o);
    test_assert(o != NULL);
    test_assert(corto_typeof(o) == type_voidReferenceType_o);
    test_assert(corto_checkState(o, CORTO_DEFINED));
    test_assert(!corto_delete(o));

/* $end */
}

corto_void _test_voidReferenceApi_tc_createChild(test_voidReferenceApi this) {
/* $begin(test/voidReferenceApi/tc_createChild) */

    type_voidReferenceType o = type_voidReferenceTypeCreateChild(root_o, "o");
    test_assert(o != NULL);
    test_assert(corto_typeof(o) == type_voidReferenceType_o);
    test_assert(corto_checkState(o, CORTO_DEFINED));
    test_assert(corto_checkAttr(o, CORTO_ATTR_SCOPED));
    test_assert(corto_parentof(o) == root_o);
    test_assert(!strcmp(corto_nameof(o), "o"));
    test_assert(!corto_delete(o));

/* $end */
}

corto_void _test_voidReferenceApi_tc_createChildAuto(test_voidReferenceApi this) {
/* $begin(test/voidReferenceApi/tc_createChildAuto) */

    type_voidReferenceTypeCreateChild_auto(root_o, o);
    test_assert(o != NULL);
    test_assert(corto_typeof(o) == type_voidReferenceType_o);
    test_assert(corto_checkState(o, CORTO_DEFINED));
    test_assert(corto_checkAttr(o, CORTO_ATTR_SCOPED));
    test_assert(corto_parentof(o) == root_o);
    test_assert(!strcmp(corto_nameof(o), "o"));
    test_assert(!corto_delete(o));

/* $end */
}

corto_void _test_voidReferenceApi_tc_update(test_voidReferenceApi this) {
/* $begin(test/voidReferenceApi/tc_update) */
    corto_int16 result;

    type_voidReferenceType o = type_voidReferenceTypeCreate();
    test_assert(o != NULL);

    result = corto_listen(this, test_voidReferenceApi_onUpdate_o, CORTO_ON_UPDATE, o, NULL);
    test_assert(result == 0);

    result = type_voidReferenceTypeUpdate(o);
    test_assert(result == 0);
    test_assert(this->updated == TRUE);

    result = corto_silence(this, test_voidReferenceApi_onUpdate_o, CORTO_ON_UPDATE, o);
    test_assert(result == 0);

    corto_delete(o);

/* $end */
}
