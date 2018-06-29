
#include "api.h"
#include "driver/gen/c/common/common.h"

corto_int16 c_apiIteratorForeach(corto_iterator o, c_apiWalk_t *data) {
    corto_id id, elementId;
    corto_bool prefix;
    corto_bool requires_alloc = corto_collection_requires_alloc(o->element_type);

    c_specifierId(data->g, corto_type(o), id, NULL, NULL);
    c_specifierId(data->g, corto_type(o->element_type), elementId, &prefix, NULL);

    g_fileWrite(data->header, "#define %s__foreach(iter, elem) \\\n", id);
    g_fileIndent(data->header);
    g_fileWrite(data->header, "%s elem;\\\n", elementId);
    g_fileWrite(data->header, "while(corto_iter_hasNext(&iter) ? ");
    if (!o->element_type->reference) {
        g_fileWrite(data->header,
            "elem = %s(%s%s)(corto_word)corto_iter_next(&iter), TRUE",
            requires_alloc ? "*" : "",
            elementId,
            requires_alloc ? "*" : "");
    } else {
        g_fileWrite(
          data->header, "elem = (%s)corto_iter_next(&iter), TRUE", elementId);
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
