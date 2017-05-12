
#include "api.h"

static corto_bool c_apiElementRequiresInit(corto_type elementType) {
    corto_bool result = FALSE;

    if ((elementType->kind != CORTO_PRIMITIVE) && !elementType->reference) {
        result = TRUE;
    }

    return result;
}

/* Initialize or deinitialize element */
static corto_int16 c_apiElementInit(corto_type elementType, corto_string element, corto_bool isInit, c_apiWalk_t* data) {
    corto_id varId;
    c_varId(data->g, elementType, varId);

    g_fileWrite(data->source, "{\n");
    g_fileIndent(data->source);
    g_fileWrite(data->source, "corto_value v;\n");
    g_fileWrite(data->source, "v = corto_value_value(%s, corto_type(%s));\n", element, varId);
    if (isInit) {
        g_fileWrite(data->source, "corto_value_init(&v);\n");
    } else {
        g_fileWrite(data->source, "corto_value_deinit(&v);\n");
    }
    g_fileDedent(data->source);
    g_fileWrite(data->source, "}\n");

    return 0;
}

/* Create append-function */
static corto_int16 c_apiSequenceTypeAppendAlloc(corto_sequence o, c_apiWalk_t* data) {
    corto_id id, elementId, varId;
    corto_bool prefix;
    corto_type elementType = corto_collection(o)->elementType;

    c_specifierId(data->g, corto_type(o), id, NULL, NULL);
    c_specifierId(data->g, corto_type(elementType), elementId, &prefix, NULL);
    c_varId(data->g, elementType, varId);

    /* Function declaration */
    c_writeExport(data->g, data->header);
    g_fileWrite(data->header, " %s* %sAppendAlloc(%s *seq);\n", elementId, id, id);

    /* Function implementation */
    g_fileWrite(data->source, "%s* %sAppendAlloc(%s *seq) {\n", elementId, id, id);

    g_fileIndent(data->source);
    g_fileWrite(data->source, "corto_uint32 size;\n");
    g_fileWrite(data->source, "seq->length++;\n");
    g_fileWrite(data->source, "seq->buffer = (%s*)corto_realloc(seq->buffer, seq->length * (size=corto_type_sizeof(corto_type(%s))));\n", elementId, varId);
    g_fileWrite(data->source, "memset(%sseq->buffer[seq->length-1], 0, size);\n", elementType->reference?"":"&");
    if (c_apiElementRequiresInit(elementType)) {
        if (!elementType->reference) {
            c_apiElementInit(elementType, "&seq->buffer[seq->length-1]", TRUE, data);
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
    corto_type elementType = corto_collection(o)->elementType;

    c_specifierId(data->g, corto_type(o), id, NULL, NULL);
    c_specifierId(data->g, corto_type(elementType), elementId, &prefix, NULL);
    c_varId(data->g, elementType, varId);

    /* Function declaration */
    c_writeExport(data->g, data->header);
    g_fileWrite(data->header, " %s* %sAppend(%s *seq, %s element);\n", elementId, id, id, elementId);

    /* Function implementation */
    g_fileWrite(data->source, "%s* %sAppend(%s *seq, %s element) {\n", elementId, id, id, elementId);

    g_fileIndent(data->source);
    g_fileWrite(data->source, "corto_uint32 size;\n");
    g_fileWrite(data->source, "seq->length++;\n");
    g_fileWrite(data->source, "seq->buffer = (%s*)corto_realloc(seq->buffer, seq->length * (size=corto_type_sizeof(corto_type(%s))));\n", elementId, varId);
    g_fileWrite(data->source, "memset(%sseq->buffer[seq->length-1], 0, size);\n", elementType->reference?"":"&");
    if (c_apiElementRequiresInit(elementType)) {
        if (!elementType->reference) {
            c_apiElementInit(elementType, "&seq->buffer[seq->length-1]", TRUE, data);
        }
    }
    if (elementType->reference) {
        g_fileWrite(data->source, "corto_ptr_setref(&seq->buffer[seq->length-1], element);\n");
    } else {
        g_fileWrite(data->source, "corto_ptr_copy(&seq->buffer[seq->length-1], %s, &element);\n", varId);
    }

    g_fileWrite(data->source, "return &seq->buffer[seq->length-1];\n");
    g_fileDedent(data->source);
    g_fileWrite(data->source, "}\n\n");

    return 0;
}

/* Create size-function */
static corto_int16 c_apiSequenceTypeSize(corto_sequence o, c_apiWalk_t* data) {
    corto_id id, elementId, varId;
    corto_bool prefix;
    corto_type elementType = corto_collection(o)->elementType;

    c_specifierId(data->g, corto_type(o), id, NULL, NULL);
    c_specifierId(data->g, corto_type(elementType), elementId, &prefix, NULL);
    c_varId(data->g, elementType, varId);

    /* Function declaration */
    c_writeExport(data->g, data->header);
    g_fileWrite(data->header, " void %sSize(%s *seq, corto_uint32 length);\n", id, id);

    /* Function implementation */
    g_fileWrite(data->source, "void %sSize(%s *seq, corto_uint32 length) {\n", id, id);
    g_fileIndent(data->source);
    g_fileWrite(data->source, "corto_uint32 size;\n");

    /* Deinitialize old elements if new size is smaller than old size - only valid for non-primitive types */
    if (c_apiElementRequiresInit(elementType)) {
        g_fileWrite(data->source, "if (length < seq->length) {\n");
        g_fileIndent(data->source);
        g_fileWrite(data->source, "corto_uint32 i;\n");
        g_fileWrite(data->source, "for(i=length; i<seq->length; i++) {\n");
        g_fileIndent(data->source);
        c_apiElementInit(elementType, "&seq->buffer[i]", FALSE, data);
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
    if (elementType->kind != CORTO_PRIMITIVE) {
        g_fileWrite(data->source, "corto_uint32 i;\n");
    }
    g_fileWrite(data->source, "memset(&seq->buffer[seq->length], 0, size * (length - seq->length));\n");
    if (elementType->kind != CORTO_PRIMITIVE) {
        g_fileWrite(data->source, "for(i=seq->length; i<length; i++) {\n");
        g_fileIndent(data->source);
        c_apiElementInit(elementType, "&seq->buffer[i]", FALSE, data);
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
    corto_type elementType = corto_collection(o)->elementType;

    c_specifierId(data->g, corto_type(o), id, NULL, NULL);
    c_specifierId(data->g, corto_type(elementType), elementId, &prefix, NULL);

    /* Function declaration */
    c_writeExport(data->g, data->header);
    g_fileWrite(data->header, " void %sClear(%s *seq);\n", id, id);

    /* Function implementation */
    g_fileWrite(data->source, "void %sClear(%s *seq) {\n", id, id);
    g_fileIndent(data->source);
    g_fileWrite(data->source, "%sSize(seq, 0);\n", id);
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
    if (c_apiSequenceTypeSize(o, data)) {
        goto error;
    }

    /* Generate clear */
    if (c_apiSequenceTypeClear(o, data)) {
        goto error;
    }

    return 0;
error:
    return -1;
}

static corto_string corto_operationToApi(corto_string operation, corto_id id) {
    sprintf(id, "corto_ll_%s", operation);
    id[strlen("corto_ll_")] = tolower(id[strlen("corto_ll_")]);
    return id;
}

/* Create insert function for types that require allocation */
static corto_int16 c_apiListTypeInsertAlloc(corto_list o, corto_string operation, c_apiWalk_t* data) {
    corto_id id, elementId, api, varId;
    corto_bool prefix;
    corto_type elementType = corto_collection(o)->elementType;

    c_specifierId(data->g, corto_type(o), id, NULL, NULL);
    c_specifierId(data->g, corto_type(elementType), elementId, &prefix, NULL);
    c_varId(data->g, elementType, varId);

    /* Function declaration */
    c_writeExport(data->g, data->header);
    g_fileWrite(data->header, " %s* %s%sAlloc(%s list);\n", elementId, id, operation, id);

    /* Function implementation */
    g_fileWrite(data->source, "%s* %s%sAlloc(%s list) {\n", elementId, id, operation, id);

    g_fileIndent(data->source);
    g_fileWrite(data->source, "%s* result;\n", elementId);
    g_fileWrite(data->source, "result = (%s*)corto_calloc(corto_type_sizeof(corto_type(%s)));\n",
        elementId,
        varId);

    /* Initialize element */
    c_apiElementInit(elementType, "result", TRUE, data);

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
    corto_type elementType = corto_collection(o)->elementType;
    corto_bool requiresAlloc = corto_collection_requiresAlloc(elementType);
    corto_bool ptr = c_typeRequiresPtr(elementType);

    c_specifierId(data->g, corto_type(o), id, NULL, NULL);
    c_specifierId(data->g, corto_type(elementType), elementId, &prefix, NULL);
    c_varId(data->g, elementType, varId);

    c_writeExport(data->g, data->header);

    if (requiresAlloc) {
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
    if (requiresAlloc) {
        g_fileWrite(data->source, "%s *result = %s%sAlloc(list);\n", elementId, id, operation);
        g_fileWrite(data->source, "corto_ptr_copy(result, %s, %selement);\n", varId, ptr ? "" : "&");
    } else {
        if ((elementType->kind == CORTO_PRIMITIVE) && (corto_primitive(elementType)->kind == CORTO_TEXT)) {
            g_fileWrite(data->source, "%s(list, (void*)corto_strdup(element));\n", corto_operationToApi(operation, api));
        } else {
            g_fileWrite(data->source, "%s(list, (void*)(corto_word)element);\n", corto_operationToApi(operation, api));
        }
        if (elementType->reference) {
            g_fileWrite(data->source, "corto_claim(element);\n");
        }
    }

    if (requiresAlloc) {
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

    if (corto_collection_requiresAlloc(corto_collection(o)->elementType)) {
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
    corto_type elementType = corto_collection(o)->elementType;

    c_specifierId(data->g, corto_type(o), id, NULL, NULL);
    c_specifierId(data->g, corto_type(elementType), elementId, &prefix, NULL);

    c_writeExport(data->g, data->header);

    g_fileWrite(data->header, " void ");
    g_fileWrite(data->source, "void ");

    /* Function declaration */
    g_fileWrite(data->header, "%sRemove(%s list, %s element);\n",
        id, id, elementId);

    /* Function implementation */
    g_fileWrite(data->source, "%sRemove(%s list, %s element) {\n",
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
    corto_type elementType = corto_collection(o)->elementType;
    corto_bool allocRequired = corto_collection_requiresAlloc(elementType);

    c_specifierId(data->g, corto_type(o), id, NULL, NULL);
    c_specifierId(data->g, corto_type(elementType), elementId, &prefix, NULL);

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
    corto_type elementType = corto_collection(o)->elementType;
    corto_bool allocRequired = corto_collection_requiresAlloc(elementType);
    corto_bool deref = (!elementType->reference && (elementType->kind == CORTO_COMPOSITE));

    c_specifierId(data->g, corto_type(o), id, NULL, NULL);
    c_specifierId(data->g, corto_type(elementType), elementId, &prefix, NULL);

    /* Function declaration */
    c_writeExport(data->g, data->header);
    g_fileWrite(
      data->header,
      " %s%s %sGet(%s list, corto_uint32 index);\n",
      elementId,
      deref ? "*" : "", id, id);

    /* Function implementation */
    g_fileWrite(
      data->source,
      "%s%s %sGet(%s list, corto_uint32 index) {\n",
      elementId,
      deref ? "*" : "", id, id);

    g_fileIndent(data->source);

    /* Insert element to list */
    if (allocRequired) {
        g_fileWrite(
          data->source,
          "return %s(%s%s)corto_ll_get(list, index);\n",
          (elementType->kind == CORTO_PRIMITIVE) ? "*" : "",
          elementId,
          allocRequired ? "*" : "");
    } else {
        g_fileWrite(
          data->source,
          "return (%s%s)(corto_word)corto_ll_get(list, index);\n",
          elementId,
          allocRequired ? "*" : "");
    }

    g_fileDedent(data->source);
    g_fileWrite(data->source, "}\n\n");

    return 0;
}

/* Create insert function for types that require allocation */
static corto_int16 c_apiListTypeSize(corto_list o, c_apiWalk_t* data) {
    corto_id id, elementId;
    corto_bool prefix;
    corto_type elementType = corto_collection(o)->elementType;

    c_specifierId(data->g, corto_type(o), id, NULL, NULL);
    c_specifierId(data->g, corto_type(elementType), elementId, &prefix, NULL);

    /* Function declaration */
    c_writeExport(data->g, data->header);
    g_fileWrite(data->header, " corto_uint32 %sSize(%s list);\n", id, id);

    /* Function implementation */
    g_fileWrite(data->source, "corto_uint32 %sSize(%s list) {\n", id, id);
    g_fileIndent(data->source);

    /* Insert element to list */
    g_fileWrite(data->source, "return corto_ll_size(list);\n");

    g_fileDedent(data->source);
    g_fileWrite(data->source, "}\n\n");

    return 0;
}

static corto_int16 c_apiListTypeClear(corto_list o, c_apiWalk_t* data) {
    corto_id id, elementId, varId;
    corto_bool prefix;
    corto_type elementType = corto_collection(o)->elementType;

    c_specifierId(data->g, corto_type(o), id, NULL, NULL);
    c_specifierId(data->g, corto_type(elementType), elementId, &prefix, NULL);
    c_varId(data->g, elementType, varId);

    /* Function declaration */
    c_writeExport(data->g, data->header);
    g_fileWrite(data->header, " void %sClear(%s list);\n", id, id);

    /* Function implementation */
    g_fileWrite(data->source, "void %sClear(%s list) {\n", id, id);
    g_fileIndent(data->source);

    g_fileWrite(data->source, "corto_iter iter = corto_ll_iter(list);\n");
    g_fileWrite(data->source, "while(corto_iter_hasNext(&iter)) {\n");
    g_fileIndent(data->source);
    g_fileWrite(data->source, "void *ptr = corto_iter_next(&iter);\n");

    if (elementType->reference) {
        g_fileWrite(data->source, "corto_release(ptr);\n");
    } else {
        g_fileWrite(data->source, "corto_ptr_deinit(ptr, %s);\n", varId);
        if (corto_collection_requiresAlloc(elementType)) {
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

    if (c_apiListTypeInsert(o, "Insert", data)) {
        goto error;
    }

    if (c_apiListTypeInsert(o, "Append", data)) {
        goto error;
    }

    if (corto_collection(o)->elementType->reference) {
        if (c_apiListTypeRemove(o, data)) {
            goto error;
        }
    }

    if (c_apiListTypeTake(o, "TakeFirst", data)) {
        goto error;
    }

    if (c_apiListTypeTake(o, "Last", data)) {
        goto error;
    }

    if (c_apiListTypeGet(o, data)) {
        goto error;
    }

    if (c_apiListTypeSize(o, data)) {
        goto error;
    }

    if (c_apiListTypeClear(o, data)) {
        goto error;
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
