/* $CORTO_GENERATED
 *
 * anyApi.cpp
 *
 * Only code written between the begin and end tags will be preserved
 * when the file is regenerated.
 */

#include <test.h>

corto_void _test_anyApi_onUpdate(
    test_anyApi _this,
    corto_object observable)
{
/* $begin(test/anyApi/onUpdate) */

    _this->updated = TRUE;

/* $end */
}

corto_void _test_anyApi_tc_compare(
    test_anyApi _this)
{
/* $begin(test/anyApi/tc_compare) */
    corto_int32 i1 = 10, i2 = 20;
    test_anyType a = {corto_type(corto_int32_o), &i1, FALSE};
    test_anyType b = {corto_type(corto_int32_o), &i2, FALSE};
    corto_equalityKind eq;

    eq = test_anyTypeCompare(a, b);
    test_assert(eq == CORTO_LT);

    eq = test_anyTypeCompare(b, a);
    test_assert(eq == CORTO_GT);

    eq = test_anyTypeCompare(a, a);
    test_assert(eq == CORTO_EQ);

/* $end */
}

corto_void _test_anyApi_tc_create(
    test_anyApi _this)
{
/* $begin(test/anyApi/tc_create) */
    corto_string v = "Foo";
    test_anyType *o = test_anyTypeCreate(corto_string_o, &v);
    test_assert(o != NULL);
    test_assert(corto_typeof(o) == corto_type(test_anyType_o));
    test_assert(corto_checkState(o, CORTO_DEFINED));
    test_assert(o->type == corto_type(corto_string_o));
    test_assert(o->value != NULL);
    test_assert(!strcmp(*(corto_string*)o->value, "Foo"));
    v = "Bar";
    test_assert(!strcmp(*(corto_string*)o->value, "Foo"));
    test_assert(o->owner = TRUE);
    test_assert(!corto_delete(o));

/* $end */
}

corto_void _test_anyApi_tc_createAuto(
    test_anyApi _this)
{
/* $begin(test/anyApi/tc_createAuto) */
    corto_string v = "Foo";
    test_anyTypeCreate_auto(o, corto_string_o, &v);
    test_assert(o != NULL);
    test_assert(corto_typeof(o) == corto_type(test_anyType_o));
    test_assert(corto_checkState(o, CORTO_DEFINED));
    test_assert(o->type == corto_type(corto_string_o));
    test_assert(o->value != NULL);
    test_assert(!strcmp(*(corto_string*)o->value, "Foo"));
    v = "Bar";
    test_assert(!strcmp(*(corto_string*)o->value, "Foo"));
    test_assert(o->owner = TRUE);
    test_assert(!corto_delete(o));

/* $end */
}

corto_void _test_anyApi_tc_createChild(
    test_anyApi _this)
{
/* $begin(test/anyApi/tc_createChild) */
    corto_string v = "Foo";
    test_anyType *o = test_anyTypeCreateChild(root_o, "o", corto_string_o, &v);
    test_assert(o != NULL);
    test_assert(corto_typeof(o) == corto_type(test_anyType_o));
    test_assert(corto_checkState(o, CORTO_DEFINED));
    test_assert(o->type == corto_type(corto_string_o));
    test_assert(o->value != NULL);
    test_assert(!strcmp(*(corto_string*)o->value, "Foo"));
    v = "Bar";
    test_assert(!strcmp(*(corto_string*)o->value, "Foo"));
    test_assert(corto_checkAttr(o, CORTO_ATTR_SCOPED));
    test_assert(corto_parentof(o) == root_o);
    test_assert(!strcmp(corto_idof(o), "o"));
    test_assert(o->owner = TRUE);
    test_assert(!corto_delete(o));

/* $end */
}

corto_void _test_anyApi_tc_createChildAuto(
    test_anyApi _this)
{
/* $begin(test/anyApi/tc_createChildAuto) */
    corto_string v = "Foo";
    test_anyTypeCreateChild_auto(root_o, o, corto_string_o, &v);
    test_assert(o != NULL);
    test_assert(corto_typeof(o) == corto_type(test_anyType_o));
    test_assert(corto_checkState(o, CORTO_DEFINED));
    test_assert(o->type == corto_type(corto_string_o));
    test_assert(o->value != NULL);
    test_assert(!strcmp(*(corto_string*)o->value, "Foo"));
    v = "Bar";
    test_assert(!strcmp(*(corto_string*)o->value, "Foo"));
    test_assert(corto_checkAttr(o, CORTO_ATTR_SCOPED));
    test_assert(corto_parentof(o) == root_o);
    test_assert(!strcmp(corto_idof(o), "o"));
    test_assert(o->owner = TRUE);
    test_assert(!corto_delete(o));

/* $end */
}

corto_void _test_anyApi_tc_declare(
    test_anyApi _this)
{
/* $begin(test/anyApi/tc_declare) */
    test_anyType *o = test_anyTypeDeclare();
    test_assert(o != NULL);
    test_assert(corto_typeof(o) == corto_type(test_anyType_o));
    test_assert(!corto_checkState(o, CORTO_DEFINED));
    test_assert(o->type == NULL);
    test_assert(o->value == NULL);
    test_assert(o->owner = TRUE);
    test_assert(!corto_delete(o));

/* $end */
}

corto_void _test_anyApi_tc_declareAuto(
    test_anyApi _this)
{
/* $begin(test/anyApi/tc_declareAuto) */
    test_anyTypeDeclare_auto(o);
    test_assert(o != NULL);
    test_assert(corto_typeof(o) == corto_type(test_anyType_o));
    test_assert(!corto_checkState(o, CORTO_DEFINED));
    test_assert(o->type == NULL);
    test_assert(o->value == NULL);
    test_assert(o->owner = TRUE);
    test_assert(!corto_delete(o));


/* $end */
}

corto_void _test_anyApi_tc_declareChild(
    test_anyApi _this)
{
/* $begin(test/anyApi/tc_declareChild) */
    test_anyType *o = test_anyTypeDeclareChild(root_o, "o");
    test_assert(o != NULL);
    test_assert(corto_typeof(o) == corto_type(test_anyType_o));
    test_assert(!corto_checkState(o, CORTO_DEFINED));
    test_assert(o->type == NULL);
    test_assert(o->value == NULL);
    test_assert(corto_checkAttr(o, CORTO_ATTR_SCOPED));
    test_assert(corto_parentof(o) == root_o);
    test_assert(!strcmp(corto_idof(o), "o"));
    test_assert(o->owner = TRUE);
    test_assert(!corto_delete(o));

/* $end */
}

corto_void _test_anyApi_tc_declareChildAuto(
    test_anyApi _this)
{
/* $begin(test/anyApi/tc_declareChildAuto) */
    test_anyTypeDeclareChild_auto(root_o, o);
    test_assert(o != NULL);
    test_assert(corto_typeof(o) == corto_type(test_anyType_o));
    test_assert(!corto_checkState(o, CORTO_DEFINED));
    test_assert(o->type == NULL);
    test_assert(o->value == NULL);
    test_assert(corto_checkAttr(o, CORTO_ATTR_SCOPED));
    test_assert(corto_parentof(o) == root_o);
    test_assert(!strcmp(corto_idof(o), "o"));
    test_assert(o->owner = TRUE);
    test_assert(!corto_delete(o));

/* $end */
}

corto_void _test_anyApi_tc_define(
    test_anyApi _this)
{
/* $begin(test/anyApi/tc_define) */
    corto_int16 result;
    corto_int32 v = 10;

    test_anyType *o = test_anyTypeDeclareChild(root_o, "o");
    test_assert(o != NULL);
    test_assert(corto_typeof(o) == corto_type(test_anyType_o));
    test_assert(!corto_checkState(o, CORTO_DEFINED));
    test_assert(corto_checkAttr(o, CORTO_ATTR_SCOPED));
    test_assert(corto_parentof(o) == root_o);
    test_assert(!strcmp(corto_idof(o), "o"));
    test_assert(o->type == NULL);
    test_assert(o->value == NULL);
    test_assert(o->owner = TRUE);

    result = test_anyTypeDefine(o, corto_int32_o, &v);
    test_assert(result == 0);
    test_assert(corto_checkState(o, CORTO_DEFINED));
    test_assert(o->type == corto_type(corto_int32_o));
    test_assert(o->value != NULL);
    test_assert(*(corto_int32*)o->value == 10);

    test_assert(!corto_delete(o));

/* $end */
}

corto_void _test_anyApi_tc_fromStr(
    test_anyApi _this)
{
/* $begin(test/anyApi/tc_fromStr) */
    corto_string v = "Foo";
    test_anyType o = {corto_type(corto_string_o), &v, FALSE};

    test_anyTypeFromStr(&o, "{/corto/lang/int32,10}");
    test_assert(o.type == corto_type(corto_int32_o));
    test_assert(o.value != NULL);
    test_assert(*(corto_int32*)o.value == 10);
    test_assert(o.owner = TRUE);

/* $end */
}

corto_void _test_anyApi_tc_set(
    test_anyApi _this)
{
/* $begin(test/anyApi/tc_set) */
    corto_int32 v1 = 10;
    corto_string v2 = "Foo";
    test_anyType o = {corto_type(corto_int32_o), &v1, FALSE};

    test_anyTypeSet(&o, corto_string_o, &v2);
    test_assert(o.type == corto_type(corto_string_o));
    test_assert(!strcmp(*(corto_string*)o.value, "Foo"));
    v2 = "Bar";
    test_assert(!strcmp(*(corto_string*)o.value, "Foo"));
    test_assert(o.owner == TRUE);

/* $end */
}

corto_void _test_anyApi_tc_str(
    test_anyApi _this)
{
/* $begin(test/anyApi/tc_str) */
    corto_int32 v1 = 10;
    test_anyType o = {corto_type(corto_int32_o), &v1, FALSE};

    corto_string str = test_anyTypeStr(o);
    test_assert(!strcmp(str, "{int32,10}"));
    corto_dealloc(str);

/* $end */
}

corto_void _test_anyApi_tc_update(
    test_anyApi _this)
{
/* $begin(test/anyApi/tc_update) */
    corto_int16 result;

    corto_string v = "Foo";
    test_anyType *o = test_anyTypeCreate(corto_string_o, &v);
    test_assert(o != NULL);

    result = corto_listen(_this, test_anyApi_onUpdate_o, CORTO_ON_UPDATE, o, NULL);
    test_assert(result == 0);

    v = "Bar";
    result = test_anyTypeUpdate(o, corto_string_o, &v);
    test_assert(result == 0);
    test_assert(_this->updated == TRUE);
    test_assert(o->type == corto_type(corto_string_o));
    test_assert(!strcmp(*(corto_string*)o->value, "Bar"));

    result = corto_silence(_this, test_anyApi_onUpdate_o, CORTO_ON_UPDATE, o);
    test_assert(result == 0);

    corto_delete(o);

/* $end */
}
