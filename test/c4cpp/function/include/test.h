/* test.h
 *
 * This file contains generated code. Do not modify!
 */

#ifndef TEST_H
#define TEST_H

#include <corto/corto.h>
#include <corto/corto.h>
#include <include/_project.h>
#include <corto/test/test.h>
#include <corto/core/c/c.h>
#include <corto/lang/c/c.h>

/* $header() */
class myNativeType {
public:
    myNativeType(int foo) : m_foo(foo) {}
    int m_foo;
};
/* $end */

#include <include/_type.h>
#include <include/_load.h>
#include <include/_api.h>

/* $body() */
/* Enter code that requires types here */
/* $end */

#ifdef __cplusplus
extern "C" {
#endif

#define test_f_collectionReferenceArg(pts) _test_f_collectionReferenceArg(pts)
TEST_EXPORT
corto_void _test_f_collectionReferenceArg(
    test_PointArray3 *pts);

#define test_f_compositeArg(p) _test_f_compositeArg(p)
TEST_EXPORT
corto_void _test_f_compositeArg(
    test_Point *p);

#define test_f_compositeReferenceArg(p) _test_f_compositeReferenceArg(test_Point(p))
TEST_EXPORT
corto_void _test_f_compositeReferenceArg(
    test_Point *p);

#define test_f_noArgs() _test_f_noArgs()
TEST_EXPORT
corto_void _test_f_noArgs(void);

#define test_f_primitiveArg(a) _test_f_primitiveArg(a)
TEST_EXPORT
corto_void _test_f_primitiveArg(
    corto_uint32 a);

#define test_f_primitiveReferenceArg(a) _test_f_primitiveReferenceArg(corto_uint32(a))
TEST_EXPORT
corto_void _test_f_primitiveReferenceArg(
    corto_uint32 *a);

#define test_f_referenceArg(f) _test_f_referenceArg(test_Foo(f))
TEST_EXPORT
corto_void _test_f_referenceArg(
    test_Foo f);

#include <include/Foo.h>
#include <include/Point.h>

#ifdef __cplusplus
}
#endif

#endif

