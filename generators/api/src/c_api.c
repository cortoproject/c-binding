/*
 * c_api.c
 *
 *  Created on: Oct 10, 2012
 *      Author: sander
 */

#include "c_api.h"
#include "c_common.h"

/* Walk all types */
static corto_int16 c_apiWalkType(corto_type o, c_apiWalk_t* data) {

    /* Generate _create function */
    if (c_apiTypeCreate(o, data)) {
        goto error;
    }

    /* Generate _createChild function */
    if (c_apiTypeCreateChild(o, data)) {
        goto error;
    }

    /* Generate _update function */
    if (c_apiTypeUpdate(o, data)) {
        goto error;
    }

    g_fileWrite(data->header, "\n");

    return 0;
error:
    return -1;
}

/* Walk non-void types */
static corto_int16 c_apiWalkNonVoid(corto_type o, c_apiWalk_t* data) {

    /* Generate _declare function */
    if (c_apiTypeDeclare(o, data)) {
        goto error;
    }

    /* Generate _declareChild function */
    if (c_apiTypeDeclareChild(o, data)) {
        goto error;
    }

    /* Generate _define function */
    if (c_apiTypeDefine(o, data)) {
        goto error;
    }

    /* Generate _set function */
    if (c_apiTypeSet(o, data)) {
        goto error;
    }

    /* Generate _str function */
    if (c_apiTypeStr(o, data)) {
        goto error;
    }

    /* Generate _fromStr function */
    if (c_apiTypeFromStr(o, data)) {
        goto error;
    }

    /* Generate _copy function */
    if (c_apiTypeCopy(o, data)) {
        goto error;
    }

    /* Generate _compare function */
    if (c_apiTypeCompare(o, data)) {
        goto error;
    }

    g_fileWrite(data->header, "\n");

    return 0;
error:
    return -1;
}

/* Walk all types */
static corto_int16 c_apiWalkNonReference(corto_type o, c_apiWalk_t* data) {

    /* Generate _init function */
    if (c_apiTypeInit(o, data)) {
        goto error;
    }

    /* Generate _deinit function */
    if (c_apiTypeDeinit(o, data)) {
        goto error;
    }

    g_fileWrite(data->header, "\n");

    return 0;
error:
    return -1;
}

/* Forward objects for which code will be generated. */
static int c_apiWalk(corto_object o, void* userData) {
    c_apiWalk_t* data = userData;

    if (corto_class_instanceof(corto_type_o, o)) {
        g_fileWrite(data->header, "/* %s */\n", corto_fullpath(NULL, o));

        data->current = o;

        /* Build nameconflict cache */
        if (corto_type(o)->kind == CORTO_COMPOSITE) {
            data->memberCache = corto_genMemberCacheBuild(o);
        }

        if (c_apiWalkType(corto_type(o), userData)) {
            goto error;
        }

        if (corto_type(o)->kind != CORTO_VOID) {
            if (c_apiWalkNonVoid(corto_type(o), userData)) {
                goto error;
            }
            if (!corto_type(o)->reference) {
                if (c_apiWalkNonReference(corto_type(o), userData)) {
                    goto error;
                }
            }
        }

        /* Clear nameconflict cache */
        if (corto_type(o)->kind == CORTO_COMPOSITE) {
            if (corto_interface(o)->kind == CORTO_DELEGATE) {
                if (c_apiDelegateCall(corto_delegate(o), userData)) {
                    goto error;
                }
                if (c_apiDelegateInitCallback(corto_delegate(o), FALSE, userData)) {
                    goto error;
                }
                if (c_apiDelegateInitCallback(corto_delegate(o), TRUE, userData)) {
                    goto error;
                }
            }
            corto_genMemberCacheClean(data->memberCache);
        }
    }

    return 1;
error:
    return 0;
}

/* Open headerfile, write standard header. */
static g_file c_apiHeaderOpen(corto_generator g) {
    g_file result;
    corto_id headerFileName, path;

    /* Create file */
    sprintf(headerFileName, "_api.h");
    result = g_fileOpen(g, headerFileName);

    /* Obtain path for macro */
    corto_path(path, root_o, g_getCurrent(g), "_");
    corto_strupper(path);

    /* Print standard comments and includes */
    g_fileWrite(result, "/* %s\n", headerFileName);
    g_fileWrite(result, " *\n");
    g_fileWrite(result, " * API convenience functions for C-language.\n");
    g_fileWrite(result, " * This file contains generated code. Do not modify!\n");
    g_fileWrite(result, " */\n\n");
    g_fileWrite(result, "#ifndef %s__API_H\n", path);
    g_fileWrite(result, "#define %s__API_H\n\n", path);

    c_includeFrom(g, result, corto_o, "corto.h");
    if (!strcmp(gen_getAttribute(g, "bootstrap"), "true")) {
        g_fileWrite(result, "#include \"%s/_interface.h\"\n", g_getName(g));
    } else {
        c_includeFrom(g, result, g_getCurrent(g), "_interface.h");
    }

    g_fileWrite(result, "#ifdef __cplusplus\n");
    g_fileWrite(result, "extern \"C\" {\n");
    g_fileWrite(result, "#endif\n");

    return result;
}

/* Close headerfile */
static void c_apiHeaderClose(g_file file) {

    /* Print standard comments and includes */
    g_fileWrite(file, "\n");
    g_fileWrite(file, "#ifdef __cplusplus\n");
    g_fileWrite(file, "}\n");
    g_fileWrite(file, "#endif\n");
    g_fileWrite(file, "#endif\n\n");
}

/* Open sourcefile */
static g_file c_apiSourceOpen(corto_generator g) {
    g_file result;
    corto_id sourceFileName;

    /* Create file */
    strcpy(sourceFileName, "_api.c");
    if (!strcmp(gen_getAttribute(g, "bootstrap"), "true")) {
        result = g_fileOpen(g, sourceFileName);
    } else {
        result = g_hiddenFileOpen(g, sourceFileName);
    }

    /* Print standard comments and includes */
    g_fileWrite(result, "/* %s\n", sourceFileName);
    g_fileWrite(result, " *\n");
    g_fileWrite(result, " * API convenience functions for C-language.\n");
    g_fileWrite(result, " * This file contains generated code. Do not modify!\n");
    g_fileWrite(result, " */\n\n");
    c_include(g, result, g_getCurrent(g));

    return result;
}

static corto_equalityKind c_apiCompareCollections(corto_collection c1, corto_collection c2) {
    corto_equalityKind result = CORTO_EQ;
    if (c1->kind != c2->kind) {
        result = CORTO_NEQ;
    } else if (c1->elementType != c2->elementType) {
        result = CORTO_NEQ;
    } else if (c1->max != c2->max) {
        result = CORTO_NEQ ;
    } else if (c1->kind == CORTO_MAP) {
        if (corto_map(c1)->keyType != corto_map(c2)->keyType) {
            result = CORTO_NEQ;
        }
    }
    return result;
}

static int c_apiCheckDuplicates(void* o, void* userData) {
    if (corto_checkAttr(o, CORTO_ATTR_SCOPED)) {
        return 1;
    } else {
        if (corto_instanceof(corto_collection_o, o)) {
            return c_apiCompareCollections(
                corto_collection(o),
                corto_collection(userData)) != CORTO_EQ;
        } else if (corto_instanceof(corto_iterator_o, o)) {
            return corto_iterator(o)->elementType ==
                corto_iterator(userData)->elementType;
        }
    }

    return 0;
}

static int c_apiFindCollections(corto_object o, void* userData) {
    c_apiWalk_t* data = userData;

    if (corto_instanceof(corto_type(corto_collection_o), o)) {
        if (!corto_llSize(data->collections) || corto_llWalk(data->collections, c_apiCheckDuplicates, o)) {
            corto_llAppend(data->collections, o);
        }
    }

    return 0;
}

static int c_apiFindIterators(corto_object o, void* userData) {
    c_apiWalk_t* data = userData;

    if (corto_instanceof(corto_type(corto_iterator_o), o)) {
        if (!corto_llSize(data->iterators) || corto_llWalk(data->iterators, c_apiCheckDuplicates, o)) {
            corto_llAppend(data->iterators, o);
        }
    }

    return 0;
}

/* Generator main */
corto_int16 corto_genMain(corto_generator g) {
    c_apiWalk_t walkData;

    /* Default prefixes for corto namespaces */
    gen_parse(g, corto_o, FALSE, FALSE, "");
    gen_parse(g, corto_lang_o, FALSE, FALSE, "corto");
    gen_parse(g, corto_core_o, FALSE, FALSE, "corto");

    walkData.g = g;
    walkData.header = c_apiHeaderOpen(g);
    walkData.source = c_apiSourceOpen(g);
    walkData.collections = corto_llNew();
    walkData.iterators = corto_llNew();

    g_walkRecursive(g, c_apiWalk, &walkData);

    /* Find collection types, including anonymous */
    if (corto_genDepWalk(g, c_apiFindCollections, NULL, &walkData)) {
        corto_trace("generation of api-routines failed while resolving collections.");
        return -1;
    }
    corto_llWalk(walkData.collections, c_apiCollectionWalk, &walkData);

    /* Find iterator types, including anonymous */
    if (corto_genDepWalk(g, c_apiFindIterators, NULL, &walkData)) {
        corto_trace("generation of api-routines failed while resolving iterators.");
        return -1;
    }
    corto_llWalk(walkData.iterators, c_apiIteratorWalk, &walkData);

    c_apiHeaderClose(walkData.header);

    corto_llFree(walkData.collections);
    corto_llFree(walkData.iterators);

    return 0;
}
