
#include "api.h"

static corto_bool c_apiElementRequiresInit(corto_type element_type) {
    corto_bool result = FALSE;

    if ((element_type->kind != CORTO_PRIMITIVE) && !element_type->reference) {
        result = TRUE;
    }

    return result;
}

/* Initialize or deinitialize element */
static corto_int16 c_apiElementInit(corto_type element_type, corto_string element, corto_bool isInit, c_apiWalk_t* data) {
    corto_id varId;
    c_varId(data->g, element_type, varId);

    g_fileWrite(data->source, "{\n");
    g_fileIndent(data->source);
    if (isInit) {
        g_fileWrite(data->source, "corto_ptr_init(%s, %s);\n", element, varId);
    } else {
        g_fileWrite(data->source, "corto_ptr_deinit(%s, %s);\n", element, varId);
    }
    g_fileDedent(data->source);
    g_fileWrite(data->source, "}\n");

    return 0;
}

/* Create append-function */
static corto_int16 c_apiSequenceTypeAppendAlloc(corto_sequence o, c_apiWalk_t* data) {
    corto_id id, elementId, varId;
    corto_bool prefix;
    corto_type element_type = corto_collection(o)->element_type;

    c_specifierId(data->g, corto_type(o), id, NULL, NULL);
    c_specifierId(data->g, corto_type(element_type), elementId, &prefix, NULL);
    c_varId(data->g, element_type, varId);

    /* Function declaration */
    c_writeExport(data->g, data->header);
    g_fileWrite(data->header, " %s* %s__append_alloc(%s *seq);\n", elementId, id, id);

    /* Function implementation */
    g_fileWrite(data->source, "%s* %s__append_alloc(%s *seq) {\n", elementId, id, id);

    g_fileIndent(data->source);
    g_fileWrite(data->source, "corto_uint32 size;\n");
    g_fileWrite(data->source, "seq->length++;\n");
    g_fileWrite(data->source, "seq->buffer = (%s*)corto_realloc(seq->buffer, seq->length * (size=corto_type_sizeof(corto_type(%s))));\n", elementId, varId);
    g_fileWrite(data->source, "memset(%sseq->buffer[seq->length-1], 0, size);\n", element_type->reference?"":"&");
    if (c_apiElementRequiresInit(element_type)) {
        if (!element_type->reference) {
            c_apiElementInit(element_type, "&seq->buffer[seq->length-1]", TRUE, data);
        }
    }
    g_fileWrite(data->source, "return &seq->buffer[seq->length-1];\n");
    g_fileDedent(data->source);
    g_fileWrite(data->source, "}\n\n");

    return 0;
}

/* Create append-function */
static corto_int16 c_apiSequenceTypeAppend(corto_sequence o, c_apiWalk_t* data) {
    corto_id id, elementId, varId;
    corto_bool prefix;
    corto_type element_type = corto_collection(o)->element_type;

    c_specifierId(data->g, corto_type(o), id, NULL, NULL);
    c_specifierId(data->g, corto_type(element_type), elementId, &prefix, NULL);
    c_varId(data->g, element_type, varId);

    /* Function declaration */
    c_writeExport(data->g, data->header);
    g_fileWrite(data->header, " %s* %s__append(%s *seq, %s element);\n", elementId, id, id, elementId);

    /* Function implementation */
    g_fileWrite(data->source, "%s* %s__append(%s *seq, %s element) {\n", elementId, id, id, elementId);

    g_fileIndent(data->source);
    g_fileWrite(data->source, "corto_uint32 size;\n");
    g_fileWrite(data->source, "seq->length++;\n");
    g_fileWrite(data->source, "seq->buffer = (%s*)corto_realloc(seq->buffer, seq->length * (size=corto_type_sizeof(corto_type(%s))));\n", elementId, varId);
    g_fileWrite(data->source, "memset(%sseq->buffer[seq->length-1], 0, size);\n", element_type->reference?"":"&");
    if (c_apiElementRequiresInit(element_type)) {
        if (!element_type->reference) {
            c_apiElementInit(element_type, "&seq->buffer[seq->length-1]", TRUE, data);
        }
    }
    if (element_type->reference) {
        g_fileWrite(data->source, "corto_set_ref(&seq->buffer[seq->length-1], element);\n");
    } else {
        g_fileWrite(data->source, "corto_ptr_copy(&seq->buffer[seq->length-1], %s, &element);\n", varId);
    }

    g_fileWrite(data->source, "return &seq->buffer[seq->length-1];\n");
    g_fileDedent(data->source);
    g_fileWrite(data->source, "}\n\n");

    return 0;
}

/* Create size-function */
static corto_int16 c_apiSequenceTypeResize(corto_sequence o, c_apiWalk_t* data) {
    corto_id id, elementId, varId;
    corto_bool prefix;
    corto_type element_type = corto_collection(o)->element_type;

    c_specifierId(data->g, corto_type(o), id, NULL, NULL);
    c_specifierId(data->g, corto_type(element_type), elementId, &prefix, NULL);
    c_varId(data->g, element_type, varId);

    /* Function declaration */
    c_writeExport(data->g, data->header);
    g_fileWrite(data->header, " void %s__resize(%s *seq, corto_uint32 length);\n", id, id);

    /* Function implementation */
    g_fileWrite(data->source, "void %s__resize(%s *seq, corto_uint32 length) {\n", id, id);
    g_fileIndent(data->source);
    g_fileWrite(data->source, "corto_uint32 size;\n");

    /* Deinitialize old elements if new size is smaller than old size - only valid for non-primitive types */
    if (c_apiElementRequiresInit(element_type)) {
        g_fileWrite(data->source, "if (length < seq->length) {\n");
        g_fileIndent(data->source);
        g_fileWrite(data->source, "corto_uint32 i;\n");
        g_fileWrite(data->source, "for(i=length; i<seq->length; i++) {\n");
        g_fileIndent(data->source);
        c_apiElementInit(element_type, "&seq->buffer[i]", FALSE, data);
        g_fileDedent(data->source);
        g_fileWrite(data->source, "}\n");
        g_fileDedent(data->source);
        g_fileWrite(data->source, "}\n");
    }

    /* Resize buffer */
    g_fileWrite(data->source, "seq->buffer = (%s*)corto_realloc(seq->buffer, length * (size=corto_type_sizeof(corto_type(%s))));\n", elementId, varId);

    /* Initialize new elements */
    g_fileWrite(data->source, "if (length > seq->length) {\n");
    g_fileIndent(data->source);
    if (element_type->kind != CORTO_PRIMITIVE) {
        g_fileWrite(data->source, "corto_uint32 i;\n");
    }
    g_fileWrite(data->source, "memset(&seq->buffer[seq->length], 0, size * (length - seq->length));\n");
    if (element_type->kind != CORTO_PRIMITIVE) {
        g_fileWrite(data->source, "for(i=seq->length; i<length; i++) {\n");
        g_fileIndent(data->source);
        c_apiElementInit(element_type, "&seq->buffer[i]", FALSE, data);
        g_fileDedent(data->source);
        g_fileWrite(data->source, "}\n");
    }
    g_fileDedent(data->source);
    g_fileWrite(data->source, "}\n");

    /* Set new length */
    g_fileWrite(data->source, "seq->length = length;\n");
    g_fileDedent(data->source);
    g_fileWrite(data->source, "}\n\n");

    return 0;
}

/* Create sequence clear-function */
static corto_int16 c_apiSequenceTypeClear(corto_sequence o, c_apiWalk_t* data) {
    corto_id id, elementId;
    corto_bool prefix;
    corto_type element_type = corto_collection(o)->element_type;

    c_specifierId(data->g, corto_type(o), id, NULL, NULL);
    c_specifierId(data->g, corto_type(element_type), elementId, &prefix, NULL);

    /* Function declaration */
    c_writeExport(data->g, data->header);
    g_fileWrite(data->header, " void %s__clear(%s *seq);\n", id, id);

    /* Function implementation */
    g_fileWrite(data->source, "void %s__clear(%s *seq) {\n", id, id);
    g_fileIndent(data->source);
    g_fileWrite(data->source, "%s__resize(seq, 0);\n", id);
    g_fileDedent(data->source);
    g_fileWrite(data->source, "}\n\n");

    return 0;
}

/* Walk sequence */
static corto_int16 c_apiWalkSequence(corto_sequence o, c_apiWalk_t* data) {

    g_fileWrite(data->header, "\n/* %s */\n", corto_fullpath(NULL, o));

    data->current = o;

    /* Generate append */
    if (c_apiSequenceTypeAppend(o, data)) {
        goto error;
    }

    /* Generate append that just returns allocated sequence */
    if (c_apiSequenceTypeAppendAlloc(o, data)) {
        goto error;
    }

    /* Generate size */
    if (c_apiSequenceTypeResize(o, data)) {
        goto error;
    }

    /* Generate clear */
    if (c_apiSequenceTypeClear(o, data)) {
        goto error;
    }

    if (strcmp(g_getAttribute(data->g, "bootstrap"), "true") && corto_parentof(o) != root_o) {
        corto_id id;
        corto_id localId;
        c_short_id(data->g, localId, o);
        c_id(data->g, id, o);
        if (strcmp(localId, id)) {
            g_fileWrite(data->header, "\n");
            g_fileWrite(data->header, "#define %s__append %s__append\n", localId, id);
            g_fileWrite(data->header, "#define %s__append_alloc %s__append_alloc\n", localId, id);
            g_fileWrite(data->header, "#define %s__resize %s__resize\n", localId, id);
            g_fileWrite(data->header, "#define %s__clear %s__clear\n", localId, id);
        }
    }

    return 0;
error:
    return -1;
}

static corto_string corto_operationToApi(corto_string operation, corto_id id) {
    sprintf(id, "corto_ll%s", &operation[1]);
    return id;
}

/* Create insert function for types that require allocation */
static corto_int16 c_apiListTypeInsertAlloc(corto_list o, corto_string operation, c_apiWalk_t* data) {
    corto_id id, elementId, api, varId;
    corto_bool prefix;
    corto_type element_type = corto_collection(o)->element_type;

    c_specifierId(data->g, corto_type(o), id, NULL, NULL);
    c_specifierId(data->g, corto_type(element_type), elementId, &prefix, NULL);
    c_varId(data->g, element_type, varId);

    /* Function declaration */
    c_writeExport(data->g, data->header);
    g_fileWrite(data->header, " %s* %s%s_alloc(%s list);\n", elementId, id, operation, id);

    /* Function implementation */
    g_fileWrite(data->source, "%s* %s%s_alloc(%s list) {\n", elementId, id, operation, id);

    g_fileIndent(data->source);
    g_fileWrite(data->source, "%s* result;\n", elementId);
    g_fileWrite(data->source, "result = (%s*)corto_calloc(corto_type_sizeof(corto_type(%s)));\n",
        elementId,
        varId);

    /* Initialize element */
    c_apiElementInit(element_type, "result", TRUE, data);

    /* Insert element to list */
    g_fileWrite(data->source, "%s(list, result);\n", corto_operationToApi(operation, api));

    /* Return new element */
    g_fileWrite(data->source, "return result;\n");
    g_fileDedent(data->source);
    g_fileWrite(data->source, "}\n\n");

    return 0;
}

/* Create insert function for types that require no allocation */
static corto_int16 c_apiListTypeInsertNoAlloc(corto_list o, corto_string operation, c_apiWalk_t* data) {
    corto_id id, elementId, api, varId;
    corto_bool prefix;
    corto_type element_type = corto_collection(o)->element_type;
    corto_bool requires_alloc = corto_collection_requires_alloc(element_type);
    corto_bool ptr = c_typeRequiresPtr(element_type);

    c_specifierId(data->g, corto_type(o), id, NULL, NULL);
    c_specifierId(data->g, corto_type(element_type), elementId, &prefix, NULL);
    c_varId(data->g, element_type, varId);

    c_writeExport(data->g, data->header);

    if (requires_alloc) {
        g_fileWrite(data->header, " %s* ", elementId);
        g_fileWrite(data->source, "%s* ", elementId);
    } else {
        g_fileWrite(data->header, " void ");
        g_fileWrite(data->source, "void ");
    }

    /* Function declaration */
    g_fileWrite(data->header, "%s%s(%s list, %s%s element);\n",
        id, operation, id, elementId, ptr ? "*" : "");

    /* Function implementation */
    g_fileWrite(data->source, "%s%s(%s list, %s%s element) {\n",
        id, operation, id, elementId, ptr ? "*" : "");

    g_fileIndent(data->source);

    /* Insert element to list */
    if (requires_alloc) {
        g_fileWrite(data->source, "%s *result = %s%s_alloc(list);\n", elementId, id, operation);
        g_fileWrite(data->source, "corto_ptr_copy(result, %s, %selement);\n", varId, ptr ? "" : "&");
    } else {
        if ((element_type->kind == CORTO_PRIMITIVE) && (corto_primitive(element_type)->kind == CORTO_TEXT)) {
            g_fileWrite(data->source, "if (element) {\n");
            g_fileIndent(data->source);
            g_fileWrite(data->source, "%s(list, (void*)corto_strdup(element));\n", corto_operationToApi(operation, api));
            g_fileDedent(data->source);
            g_fileWrite(data->source, "} else {\n");
            g_fileWrite(data->source, "%s(list, (void*)element);\n", corto_operationToApi(operation, api));
            g_fileWrite(data->source, "}\n");
        } else {
            if (element_type->kind == CORTO_PRIMITIVE || element_type->reference) {
                g_fileWrite(data->source, "%s(list, (void*)*(uintptr_t*)&element);\n", corto_operationToApi(operation, api));
            } else {
                g_fileWrite(data->source, "%s%s result;\n", elementId, ptr ? "*" : "");
                g_fileWrite(data->source, "corto_ptr_init(&result, %s);\n", varId);
                g_fileWrite(data->source, "corto_ptr_copy(&result, %s, %selement);\n", varId, ptr ? "" : "&");
                g_fileWrite(data->source, "%s(list, result);\n", corto_operationToApi(operation, api));
            }
        }
        if (element_type->reference) {
            g_fileWrite(data->source, "if (element) {\n");
            g_fileIndent(data->source);
            g_fileWrite(data->source, "corto_claim(element);\n");
            g_fileDedent(data->source);
            g_fileWrite(data->source, "}\n");
        }
    }

    if (requires_alloc) {
        g_fileWrite(data->source, "return result;\n");
    }

    /* Return new element */
    g_fileDedent(data->source);
    g_fileWrite(data->source, "}\n\n");

    return 0;
}

/* Create insert-function */
static corto_int16 c_apiListTypeInsert(corto_list o, corto_string operation, c_apiWalk_t* data) {
    corto_int16 result;

    if (corto_collection_requires_alloc(corto_collection(o)->element_type)) {
        result = c_apiListTypeInsertAlloc(o, operation, data);
        result = c_apiListTypeInsertNoAlloc(o, operation, data);
    } else {
        result = c_apiListTypeInsertNoAlloc(o, operation, data);
    }

    return result;
}

/* Create insert function for types that require no allocation */
static corto_int16 c_apiListTypeRemove(corto_list o, c_apiWalk_t* data) {
    corto_id id, elementId;
    corto_bool prefix;
    corto_type element_type = corto_collection(o)->element_type;

    c_specifierId(data->g, corto_type(o), id, NULL, NULL);
    c_specifierId(data->g, corto_type(element_type), elementId, &prefix, NULL);

    c_writeExport(data->g, data->header);

    g_fileWrite(data->header, " void ");
    g_fileWrite(data->source, "void ");

    /* Function declaration */
    g_fileWrite(data->header, "%s__remove(%s list, %s element);\n",
        id, id, elementId);

    /* Function implementation */
    g_fileWrite(data->source, "%s__remove(%s list, %s element) {\n",
        id, id, elementId);

    g_fileIndent(data->source);

    g_fileWrite(data->source, "corto_ll_remove(list, element);\n");
    g_fileWrite(data->source, "corto_release(element);\n");

    g_fileDedent(data->source);
    g_fileWrite(data->source, "}\n\n");

    return 0;
}


/* Create take function for types that require allocation */
static corto_int16 c_apiListTypeTake(corto_list o, corto_string operation, c_apiWalk_t* data) {
    corto_id id, elementId, api;
    corto_bool prefix;
    corto_type element_type = corto_collection(o)->element_type;
    corto_bool allocRequired = corto_collection_requires_alloc(element_type);

    c_specifierId(data->g, corto_type(o), id, NULL, NULL);
    c_specifierId(data->g, corto_type(element_type), elementId, &prefix, NULL);

    /* Function declaration */
    c_writeExport(data->g, data->header);
    g_fileWrite(data->header, " %s%s %s%s(%s list);\n", elementId, allocRequired?"*":"", id, operation, id);

    /* Function implementation */
    g_fileWrite(data->source, "%s%s %s%s(%s list) {\n", elementId, allocRequired?"*":"", id, operation, id);
    g_fileIndent(data->source);

    /* Insert element to list */
    g_fileWrite(data->source, "return (%s%s)(corto_word)%s(list);\n", elementId, allocRequired?"*":"", corto_operationToApi(operation, api));

    g_fileDedent(data->source);
    g_fileWrite(data->source, "}\n\n");

    return 0;
}

/* Create get function for types that require allocation */
static corto_int16 c_apiListTypeGet(corto_list o, c_apiWalk_t* data) {
    corto_id id, elementId;
    corto_bool prefix;
    corto_type element_type = corto_collection(o)->element_type;
    corto_bool allocRequired = corto_collection_requires_alloc(element_type);
    corto_bool deref = (!element_type->reference && (element_type->kind == CORTO_COMPOSITE));

    c_specifierId(data->g, corto_type(o), id, NULL, NULL);
    c_specifierId(data->g, corto_type(element_type), elementId, &prefix, NULL);

    /* Function declaration */
    c_writeExport(data->g, data->header);
    g_fileWrite(
      data->header,
      " %s%s %s__get(%s list, corto_uint32 index);\n",
      elementId,
      deref ? "*" : "", id, id);

    /* Function implementation */
    g_fileWrite(
      data->source,
      "%s%s %s__get(%s list, corto_uint32 index) {\n",
      elementId,
      deref ? "*" : "", id, id);

    g_fileIndent(data->source);

    /* Insert element to list */
    if (allocRequired) {
        g_fileWrite(
          data->source,
          "return %s(%s%s)corto_ll_get(list, index);\n",
          (element_type->kind == CORTO_PRIMITIVE) ? "*" : "",
          elementId,
          allocRequired ? "*" : "");
    } else {
        g_fileWrite(
          data->source,
          "void *_get_temp = corto_ll_get(list, index);\nreturn *(%s*%s)&_get_temp;",
          elementId,
          allocRequired ? "*" : "");
    }

    g_fileDedent(data->source);
    g_fileWrite(data->source, "}\n\n");

    return 0;
}

static corto_int16 c_apiListTypeClear(corto_list o, c_apiWalk_t* data) {
    corto_id id, elementId, varId;
    corto_bool prefix;
    corto_type element_type = corto_collection(o)->element_type;

    c_specifierId(data->g, corto_type(o), id, NULL, NULL);
    c_specifierId(data->g, corto_type(element_type), elementId, &prefix, NULL);
    c_varId(data->g, element_type, varId);

    /* Function declaration */
    c_writeExport(data->g, data->header);
    g_fileWrite(data->header, " void %s__clear(%s list);\n", id, id);

    /* Function implementation */
    g_fileWrite(data->source, "void %s__clear(%s list) {\n", id, id);
    g_fileIndent(data->source);

    g_fileWrite(data->source, "corto_iter iter = corto_ll_iter(list);\n");
    g_fileWrite(data->source, "while(corto_iter_hasNext(&iter)) {\n");
    g_fileIndent(data->source);
    g_fileWrite(data->source, "void *ptr = corto_iter_next(&iter);\n");

    if (element_type->reference) {
        g_fileWrite(data->source, "corto_release(ptr);\n");
    } else {
        g_fileWrite(data->source, "corto_ptr_deinit(ptr, %s);\n", varId);
        if (corto_collection_requires_alloc(element_type)) {
            g_fileWrite(data->source, "corto_dealloc(ptr);\n");
        }
    }
    g_fileDedent(data->source);
    g_fileWrite(data->source, "}\n");
    g_fileWrite(data->source, "corto_ll_clear(list);\n");

    g_fileDedent(data->source);
    g_fileWrite(data->source, "}\n\n");

    return 0;
}

/* Walk list */
static corto_int16 c_apiWalkList(corto_list o, c_apiWalk_t* data) {

    g_fileWrite(data->header, "\n/* %s */\n", corto_fullpath(NULL, o));

    data->current = o;

    if (c_apiListTypeInsert(o, "__insert", data)) {
        goto error;
    }

    if (c_apiListTypeInsert(o, "__append", data)) {
        goto error;
    }

    if (corto_collection(o)->element_type->reference) {
        if (c_apiListTypeRemove(o, data)) {
            goto error;
        }
    }

    if (c_apiListTypeTake(o, "__takeFirst", data)) {
        goto error;
    }

    if (c_apiListTypeTake(o, "__last", data)) {
        goto error;
    }

    if (c_apiListTypeGet(o, data)) {
        goto error;
    }

    if (c_apiListTypeClear(o, data)) {
        goto error;
    }

    if (strcmp(g_getAttribute(data->g, "bootstrap"), "true") && corto_parentof(o) != root_o) {
        corto_id id;
        corto_id localId;
        c_short_id(data->g, localId, o);
        c_id(data->g, id, o);
        if (strcmp(localId, id)) {
            g_fileWrite(data->header, "\n");
            g_fileWrite(data->header, "#define %s__insert %s__insert\n", localId, id);
            g_fileWrite(data->header, "#define %s__append %s__append\n", localId, id);
            g_fileWrite(data->header, "#define %s__append_alloc %s__append_alloc\n", localId, id);
            if (corto_collection(o)->element_type->reference) {
                g_fileWrite(data->header, "#define %s__remove %s__remove\n", localId, id);
            }
            g_fileWrite(data->header, "#define %s__takeFirst %s__takeFirst\n", localId, id);
            g_fileWrite(data->header, "#define %s__last %s__last\n", localId, id);
            g_fileWrite(data->header, "#define %s__get %s__get\n", localId, id);
            g_fileWrite(data->header, "#define %s__clear %s__clear\n", localId, id);
        }
    }

    return 0;
error:
    return -1;
}

/* Forward collection objects for which code will be generated */
int c_apiCollectionWalk(void* o, void* userData) {

    /* Forward sequences, lists and maps */
    switch(corto_collection(o)->kind) {
        case CORTO_SEQUENCE:
            if (c_apiWalkSequence(corto_sequence(o), userData)) {
                goto error;
            }
            break;
        case CORTO_LIST:
            if (c_apiWalkList(corto_list(o), userData)) {
                goto error;
            }
            break;
        case CORTO_MAP:
            /*if (c_apiWalkMap(corto_map(o), userData)) {
             goto error;
             }*/
            break;
        default:
            break;
    }


    return 1;
error:
    return 0;
}
