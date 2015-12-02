
#include "corto.h"
#include "c_api.h"
#include "c_common.h"

corto_int16 c_apiIteratorForeach(corto_iterator o, c_apiWalk_t *data) {
    corto_id id, elementId;
    corto_bool prefix;

    c_specifierId(data->g, corto_type(o), id, NULL, NULL);
    c_specifierId(data->g, corto_type(o->elementType), elementId, &prefix, NULL);

    g_fileWrite(data->header, "#define %sForeach(iter, elem) \\\n", id);
    g_fileWrite(data->header, "while(corto_iterHasNext(&elem##_iter) ? ");
    if (!o->elementType->reference) {
        g_fileWrite(data->header, "%s *elem;\\\n", elementId);
        g_fileWrite(data->header, "elem = *(%s*)(corto_word)corto_iterNext(&iter), TRUE", elementId);
    } else {
        g_fileWrite(data->header, "%s *elem;\\\n", elementId);
        g_fileWrite(data->header, "elem = corto_iterNext(&iter), TRUE");
    }
    g_fileWrite(data->header, " : FALSE)\n");
    g_fileDedent(data->header);
    g_fileWrite(data->header, "\n");

    return 0;
}

/* Forward collection objects for which code will be generated */
int c_apiIteratorWalk(void* o, void* userData) {

    if (c_apiIteratorForeach(o, userData)) {
        goto error;
    }

    return 1;
error:
    return 0;
}
