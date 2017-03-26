/* corto_array.h
 *
 * This file contains generated code. Do not modify!
 */

#ifndef corto_api_H
#define corto_api_H

#include "corto/corto.h"
#include "corto/gen/c/common/common.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct c_apiWalk_t {
    g_generator g;
    g_file header;
    g_file source;
    corto_string _this;
    corto_bool owned;
    corto_object current;
    corto_uint32 parameterCount;
    corto_ll memberCache;
    corto_ll collections;
    corto_ll iterators;
    corto_ll args;
} c_apiWalk_t;

corto_int16 c_apiTypeCreate(corto_type t, c_apiWalk_t *data);
corto_int16 c_apiTypeCreateChild(corto_type t, c_apiWalk_t *data);
corto_int16 c_apiTypeStr(corto_type t, c_apiWalk_t *data);

corto_int16 c_apiTypeDeclare(corto_type o, c_apiWalk_t *data);
corto_int16 c_apiTypeDeclareChild(corto_type o, c_apiWalk_t *data);
corto_int16 c_apiTypeDefine(corto_type o, c_apiWalk_t *data);
corto_int16 c_apiTypeUpdate(corto_type o, c_apiWalk_t *data);
corto_int16 c_apiTypeSet(corto_type t, c_apiWalk_t *data);
corto_int16 c_apiTypeStr(corto_type o, c_apiWalk_t *data);
corto_int16 c_apiTypeFromStr(corto_type o, c_apiWalk_t *data);
corto_int16 c_apiTypeCopy(corto_type o, c_apiWalk_t *data);
corto_int16 c_apiTypeCompare(corto_type o, c_apiWalk_t *data);

corto_int16 c_apiTypeInit(corto_type o, c_apiWalk_t *data);
corto_int16 c_apiTypeDeinit(corto_type o, c_apiWalk_t *data);

corto_int16 c_apiDelegateCall(corto_delegate t, c_apiWalk_t *data);
corto_int16 c_apiDelegateInitCallback(
    corto_delegate t,
    corto_bool instance,
    c_apiWalk_t *data);

int c_apiCollectionWalk(void* o, void* userData);
int c_apiIteratorWalk(void* o, void* userData);

#ifdef __cplusplus
}
#endif

#endif
