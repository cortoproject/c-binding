/*
 * c_api.c
 *
 *  Created on: Oct 10, 2012
 *      Author: sander
 */

#include "api.h"
#include "corto/gen/c/common/common.h"

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

    g_fileWrite(data->header, "\n");

    return 0;
error:
    return -1;
}

/* Forward objects for which code will be generated. */
static int c_apiWalk(corto_object o, void* userData) {
    c_apiWalk_t* data = userData;

    if (corto_class_instanceof(corto_type_o, o) && !corto_instanceof(corto_native_type_o, o)) {
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
static g_file c_apiHeaderOpen(c_apiWalk_t *data) {
    g_file result;
    corto_bool local = !strcmp(g_getAttribute(data->g, "local"), "true");
    corto_bool app = !strcmp(g_getAttribute(data->g, "app"), "true");
    corto_id headerFileName, path;
    corto_id name = {0};
    char *namePtr = g_getName(data->g), *ptr = name + 1;

    if (!app && !local) {
        corto_fullpath(name, g_getCurrent(data->g));

        while (*namePtr == *ptr && *namePtr && *ptr) {
            namePtr ++;
            ptr ++;
        }

        if (ptr[0] == '/') {
            ptr ++;
        }
    }

    if (!ptr[0]) {
        strcpy(name, "_api");
        ptr = name;
    }

    /* Create file */
    sprintf(headerFileName, "%s.h", ptr);

    if (!local && !app) {
        g_fileWrite(data->mainHeader, "#include <%s/c/%s>\n", g_getName(data->g), headerFileName);
    }

    /* Create file */
    result = g_fileOpen(data->g, headerFileName);

    /* Obtain path for macro */
    corto_path(path, root_o, g_getCurrent(data->g), "_");
    corto_strupper(path);

    /* Print standard comments and includes */
    g_fileWrite(result, "/* %s\n", headerFileName);
    g_fileWrite(result, " *\n");
    g_fileWrite(result, " * API convenience functions for C-language.\n");
    g_fileWrite(result, " * This file contains generated code. Do not modify!\n");
    g_fileWrite(result, " */\n\n");
    g_fileWrite(result, "#ifndef %s__API_H\n", path);
    g_fileWrite(result, "#define %s__API_H\n\n", path);

    c_includeFrom(data->g, result, corto_o, "corto.h");
    if (!strcmp(g_getAttribute(data->g, "bootstrap"), "true")) {
        g_fileWrite(result, "#include <%s/_project.h>\n", g_getName(data->g));
    } else {
        c_includeFrom(data->g, result, g_getCurrent(data->g), "_project.h");
        c_includeFrom(data->g, result, g_getCurrent(data->g), "_type.h");
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
static g_file c_apiSourceOpen(g_generator g) {
    g_file result;
    corto_id sourceFileName;
    corto_bool cpp = !strcmp(g_getAttribute(g, "c4cpp"), "true");
    corto_bool local = !strcmp(g_getAttribute(g, "local"), "true");
    corto_bool app = !strcmp(g_getAttribute(g, "app"), "true");
    corto_id name = {0};
    char *namePtr = g_getName(g), *ptr = name + 1;
    corto_bool hidden = FALSE;

    if (!app && !local) {
        corto_fullpath(name, g_getCurrent(g));

        while (*namePtr == *ptr && *namePtr && *ptr) {
            namePtr ++;
            ptr ++;
        }

        if (ptr[0] == '/') {
            ptr ++;
        }
    } else {
        hidden = TRUE;
    }

    if (!ptr[0]) {
        strcpy(name, "_api");
        ptr = name;
    }

    /* Create file */
    sprintf(sourceFileName, "%s.%s", ptr, cpp ? "cpp" : "c");

    if (hidden) {
        result = g_hiddenFileOpen(g, "%s", sourceFileName);
    } else {
        result = g_fileOpen(g, "%s", sourceFileName);
    }

    if (!result) {
        goto error;
    }

    /* Print standard comments and includes */
    g_fileWrite(result, "/* %s\n", sourceFileName);
    g_fileWrite(result, " *\n");
    g_fileWrite(result, " * API convenience functions for C-language.\n");
    g_fileWrite(result, " * This file contains generated code. Do not modify!\n");
    g_fileWrite(result, " */\n\n");

    if (!local && !app) {
        g_fileWrite(result, "#include <%s/c/c.h>\n", g_getName(g));
        corto_id path;
        corto_path(path, root_o, g_getCurrent(g), "/");
        
        /* Add header files for dependent packages */
        if (g->imports) {
            corto_iter iter = corto_ll_iter(g->imports);
            while (corto_iter_hasNext(&iter)) {
                corto_object import = corto_iter_next(&iter);
                corto_string str = corto_path(NULL, NULL, import, "/");
                corto_string package = corto_locate(str, NULL, CORTO_LOCATION_FULLNAME);
                if (!package) {
                    corto_seterr("project configuration contains unresolved package '%s'", str);
                    goto error;
                } else {
                    corto_string name = corto_locate(str, NULL, CORTO_LOCATION_NAME);
                    g_fileWrite(result, "#include <%s/%s.h>\n", package, name);
                    corto_dealloc(name);
                    corto_dealloc(package);
                }
            }
        }
    } else {
        g_fileWrite(result, "#include <include/_api.h>\n");
        g_fileWrite(result, "#include <include/_load.h>\n", g_getName(g));
    }

    g_fileWrite(result, "\n");

    return result;
error:
    return NULL;
}

static int c_apiWalkPackages(corto_object o, void* userData) {
    c_apiWalk_t *data = userData;

    CORTO_UNUSED(o);

    if (!g_walkRecursive(data->g, c_apiWalk, data)) {
        goto error;
    }

    c_apiHeaderClose(data->header);

    return 1;
error:
    return 0;
}

static int c_apiVariableWalk(void *o, void *userData) {
    c_apiWalk_t *data = userData;
    corto_id varId, typeId, packageId;
    corto_fullpath(packageId, g_getCurrent(data->g));
    c_varId(data->g, o, varId);
    c_typeret(data->g, corto_typeof(o), C_ByReference, typeId);
    g_fileWrite(data->source, "corto_type _%s;\n", varId);
    g_fileWrite(
        data->source,
        "#define %s _%s ? _%s : (_%s = *(corto_type*)corto_load_sym(\"%s\", &_package, \"%s\"))\n", 
        varId, varId, varId, varId, packageId, varId);
    g_fileWrite(data->source, "\n");

    return 1;
}

/* Generator main */
corto_int16 corto_genMain(g_generator g) {
    corto_bool local = !strcmp(g_getAttribute(g, "local"), "true");
    corto_bool app = !strcmp(g_getAttribute(g, "app"), "true");

    c_apiWalk_t walkData;
    memset(&walkData, 0, sizeof(walkData));
    char *cwd = corto_strdup(corto_cwd());

    /* Create project files */
    if (!local && !app) {
        if (!corto_fileTest("c/rakefile")) {
            corto_int8 ret, sig;
            corto_id cmd;
            sprintf(cmd, "corto create package %s/c --unmanaged --notest --nobuild --silent", g_getName(g));
            sig = corto_proccmd(cmd, &ret);
            if (sig || ret) {
                corto_seterr("failed to setup project for '%s/c'", 
                    g_getName(g));
                goto error;
            }

            /* Overwrite rakefile */
            g_file rakefile = g_fileOpen(g, "c/rakefile");
            if (!rakefile) {
                corto_seterr("failed to open c/rakefile: %s", corto_lasterr());
                goto error;
            }
            g_fileWrite(rakefile, "PACKAGE = '%s/c'\n\n", g_getName(g));
            g_fileWrite(rakefile, "NOCORTO = true\n");
            g_fileWrite(rakefile, "require \"#{ENV['CORTO_BUILD']}/package\"\n");
            g_fileClose(rakefile);
        }

        corto_chdir("c");

        walkData.mainHeader = g_fileOpen(g, "c.h");
        if (!walkData.mainHeader) {
            goto error;
        }
    } else {
        walkData.mainHeader = NULL;
    }
    walkData.g = g;

    walkData.types = c_findType(g, corto_type_o);
    walkData.collections = c_findType(g, corto_collection_o);
    walkData.iterators = c_findType(g, corto_iterator_o);
    walkData.args = NULL;

    /* Default prefixes for corto namespaces */
    g_parse(g, corto_o, FALSE, FALSE, "");
    g_parse(g, corto_lang_o, FALSE, FALSE, "corto");
    g_parse(g, corto_core_o, FALSE, FALSE, "corto");
    g_parse(g, corto_native_o, FALSE, FALSE, "corto_native");
    g_parse(g, corto_secure_o, FALSE, FALSE, "corto_secure");

    /* Open API header */
    walkData.header = c_apiHeaderOpen(&walkData);
    if (!walkData.header) {
        goto error;
    }

    /* Open API source file */
    walkData.source = c_apiSourceOpen(g);
    if (!walkData.source) {
        goto error;
    }

    /* Define local variables for package objects if functions are generated in
     * a separate package (to prevent cyclic dependencies between libs) */
    if (!app && !local && corto_ll_size(walkData.types)) {
        g_fileWrite(walkData.source, "static corto_dl _package;\n");
        corto_ll_walk(walkData.types, c_apiVariableWalk, &walkData);
    }

    if (!g_walkNoScope(g, c_apiWalkPackages, &walkData)) {
        goto error;
    }

    corto_ll_walk(walkData.collections, c_apiCollectionWalk, &walkData);
    corto_ll_walk(walkData.iterators, c_apiIteratorWalk, &walkData);

    corto_ll_free(walkData.collections);
    corto_ll_free(walkData.iterators);

    if (!local) {
        corto_chdir(cwd);
        corto_dealloc(cwd);
    }

    return 0;
error:
    return -1;
}
