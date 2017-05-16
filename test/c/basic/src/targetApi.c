/* $CORTO_GENERATED
 *
 * targetApi.c
 *
 * Only code written between the begin and end tags will be preserved
 * when the file is regenerated.
 */

#include <include/test.h>

corto_void _test_targetApi_setup(
    test_targetApi this)
{
/* $begin(test/targetApi/setup) */

    /* << Insert implementation >> */

/* $end */
}

corto_void _test_targetApi_tc_compare(
    test_targetApi this)
{
/* $begin(test/targetApi/tc_compare) */
    corto_equalityKind eq;
    test_intTarget a = test_intTargetCreate(10, 20, 20);
    test_intTarget b = test_intTargetCreate(20, 30, 30);

    eq = test_intTargetCompare(a, b);
    test_assert(eq == CORTO_LT);

    eq = test_intTargetCompare(b, a);
    test_assert(eq == CORTO_GT);

    eq = test_intTargetCompare(a, a);
    test_assert(eq == CORTO_EQ);

/* $end */
}

corto_void _test_targetApi_tc_create(
    test_targetApi this)
{
/* $begin(test/targetApi/tc_create) */
    test_intTarget o = test_intTargetCreate(10, 20, 20);
    test_assert(o != NULL);
    test_assert(corto_checkState(o, CORTO_DEFINED));
    test_assert(o->actual == 10);
    test_assert(o->target == 20);
    test_assert(!corto_delete(o));

/* $end */
}

corto_void _test_targetApi_tc_createAuto(
    test_targetApi this)
{
/* $begin(test/targetApi/tc_createAuto) */
    test_intTargetCreate_auto(o, 10, 20, 20);
    test_assert(o != NULL);
    test_assert(corto_checkState(o, CORTO_DEFINED));
    test_assert(o->actual == 10);
    test_assert(o->target == 20);
    test_assert(!corto_delete(o));

/* $end */
}

corto_void _test_targetApi_tc_createChild(
    test_targetApi this)
{
/* $begin(test/targetApi/tc_createChild) */
    test_intTarget o = test_intTargetCreateChild(root_o, "o", 10, 20, 20);
    test_assert(o != NULL);
    test_assert(corto_checkState(o, CORTO_DEFINED));
    test_assert(corto_checkAttr(o, CORTO_ATTR_NAMED));
    test_assert(corto_parentof(o) == root_o);
    test_assert(!strcmp(corto_idof(o), "o"));
    test_assert(o->actual == 10);
    test_assert(o->target == 20);
    test_assert(!corto_delete(o));

/* $end */
}

corto_void _test_targetApi_tc_createChildAuto(
    test_targetApi this)
{
/* $begin(test/targetApi/tc_createChildAuto) */
    test_intTargetCreateChild_auto(root_o, o, 10, 20, 20);
    test_assert(o != NULL);
    test_assert(corto_checkState(o, CORTO_DEFINED));
    test_assert(corto_checkAttr(o, CORTO_ATTR_NAMED));
    test_assert(corto_parentof(o) == root_o);
    test_assert(!strcmp(corto_idof(o), "o"));
    test_assert(o->actual == 10);
    test_assert(o->target == 20);
    test_assert(!corto_delete(o));

/* $end */
}

corto_void _test_targetApi_tc_fromStr(
    test_targetApi this)
{
/* $begin(test/targetApi/tc_fromStr) */
    test_intTarget o = test_intTargetCreate(10, 20, 20);
    test_assert(o != NULL);

    test_intTargetFromStr(o, "{actual=30,target=40}");
    test_assertint(o->actual, 30);
    test_assertint(o->target, 20); /* object is owned by this thread */

    /* Test with owner set to mount */
    corto_mount m = corto_create(corto_mount_o);
    corto_setOwner(m);
    test_intTargetFromStr(o, "{actual=40,target=50}");
    test_assertint(o->actual, 30); /* object is not owned by this thread */
    test_assertint(o->target, 50);

    corto_setOwner(NULL);
    test_assert(!corto_delete(m));
    test_assert(!corto_delete(o));

/* $end */
}

corto_void _test_targetApi_tc_fromStrOwnedByMount(
    test_targetApi this)
{
/* $begin(test/targetApi/tc_fromStrOwnedByMount) */
    corto_mount m = corto_create(corto_mount_o);
    corto_setOwner(m);
    corto_setAttr(CORTO_ATTR_PERSISTENT);
    test_intTarget o = test_intTargetCreateChild(root_o, "a", 10, 20, 20);
    test_assert(o != NULL);
    test_assert(corto_ownerof(o) == m);

    test_intTargetFromStr(o, "{actual=30,target=40}");
    test_assertint(o->actual, 30);
    test_assertint(o->target, 20); /* object is owned by this thread */

    corto_setOwner(NULL);
    test_intTargetFromStr(o, "{actual=40,target=50}");
    test_assertint(o->actual, 30); /* object is not owned by this thread */
    test_assertint(o->target, 50);

    corto_setOwner(m);
    test_assert(!corto_delete(o));

    corto_setOwner(NULL);
    test_assert(!corto_delete(m));

/* $end */
}

corto_void _test_targetApi_tc_set(
    test_targetApi this)
{
/* $begin(test/targetApi/tc_set) */
    test_intTarget o = test_intTargetCreate(10, 20, 20);
    test_assert(o != NULL);

    test_intTargetAssign(o, 20, 30, 30);
    test_assert(o->actual == 20);
    test_assert(o->target == 30);
    test_assert(!corto_delete(o));

/* $end */
}

corto_void _test_targetApi_tc_str(
    test_targetApi this)
{
/* $begin(test/targetApi/tc_str) */
    test_intTarget o = test_intTargetCreate(10, 20, 20);
    test_assert(o != NULL);

    corto_string str = test_intTargetStr(o);
    test_assertstr(str, "{10,20,20}");
    corto_dealloc(str);

/* $end */
}
