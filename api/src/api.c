/*
 * c_api.c
 *
 *  Created on: Oct 10, 2012
 *      Author: sander
 */

#include "api.h"
#include "driver/gen/c/common/common.h"

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
    strupper(path);

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
                    corto_throw("project configuration contains unresolved package '%s'", str);
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

static int c_apiWalkPackages(corto_object o, void* userData) {
    c_apiWalk_t *data = userData;
    corto_bool local = !strcmp(g_getAttribute(data->g, "local"), "true");
    corto_bool app = !strcmp(g_getAttribute(data->g, "app"), "true");
    corto_bool bootstrap = !strcmp(g_getAttribute(data->g, "bootstrap"), "true");

    CORTO_UNUSED(o);

    /* Open API header */
    data->header = c_apiHeaderOpen(data);
    if (!data->header) {
        goto error;
    }

    /* Open API source file */
    data->source = c_apiSourceOpen(data->g);
    if (!data->source) {
        goto error;
    }

    data->collections = c_findType(data->g, corto_collection_o);
    data->iterators = c_findType(data->g, corto_iterator_o);
    data->args = NULL;

    /* Define local variables for package objects if functions are generated in
     * a separate package (to prevent cyclic dependencies between libs) */
    if (!app && !local && !bootstrap) {
        data->types = c_findType(data->g, corto_type_o);
        if (data->types && corto_ll_count(data->types)) {
            g_fileWrite(data->source, "static corto_dl _package;\n");
            corto_ll_walk(data->types, c_apiVariableWalk, data);
            corto_ll_free(data->types);
        }
    }

    if (!g_walkRecursive(data->g, c_apiWalk, data)) {
        goto error;
    }

    corto_ll_walk(data->collections, c_apiCollectionWalk, data);
    corto_ll_walk(data->iterators, c_apiIteratorWalk, data);

    corto_ll_free(data->collections);
    corto_ll_free(data->iterators);

    c_apiHeaderClose(data->header);

    return 1;
error:
    return 0;
}

/* Generator main */
corto_int16 genmain(g_generator g) {
    corto_bool local = !strcmp(g_getAttribute(g, "local"), "true");
    corto_bool app = !strcmp(g_getAttribute(g, "app"), "true");

    c_apiWalk_t walkData;
    memset(&walkData, 0, sizeof(walkData));
    char *cwd = corto_strdup(corto_cwd());

    /* Create project files */
    if (!local && !app) {
        if (!corto_file_test("c/project.json")) {
            corto_int8 ret, sig;
            corto_id cmd;
            sprintf(cmd, "corto create package %s/c --unmanaged --notest --nobuild --silent -o c", g_getName(g));
            sig = corto_proccmd(cmd, &ret);
            if (sig || ret) {
                corto_throw("failed to setup project for '%s/c'", g_getName(g));
                goto error;
            }

            /* Overwrite rakefile */
            g_file rakefile = g_fileOpen(g, "c/project.json");
            if (!rakefile) {
                corto_throw("failed to open c/project.json");
                goto error;
            }
            g_fileWrite(rakefile, "{\n");
            g_fileWrite(rakefile, "    \"id\": \"%s/c\",\n", g_getName(g));
            g_fileWrite(rakefile, "    \"type\": \"package\",\n");
            g_fileWrite(rakefile, "    \"value\": {\n");

            g_fileWrite(rakefile, "        \"use\": [\"corto\"");
            if (g->imports) {
                corto_iter iter = corto_ll_iter(g->imports);
                while (corto_iter_hasNext(&iter)) {
                    corto_object import = corto_iter_next(&iter);
                    corto_string str = corto_path(NULL, NULL, import, "/");

                    /* Do not use /c packages in /c package */
                    char *lastElem = strrchr(str, '/');
                    if (lastElem) {
                        if (!strcmp(lastElem, "/c")) {
                            continue;
                        }
                    }

                    g_fileWrite(rakefile, ", \"%s\"", str);
                }
            }
            g_fileWrite(rakefile, "],\n");

            g_fileWrite(rakefile, "        \"language\": \"%s\",\n", strcmp(g_getAttribute(g, "c4cpp"), "true") ? "c" : "c4cpp");
            g_fileWrite(rakefile, "        \"managed\": false\n");
            g_fileWrite(rakefile, "    }\n");
            g_fileWrite(rakefile, "}\n");
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

    /* Default prefixes for corto namespaces */
    g_parse(g, corto_o, FALSE, FALSE, "");
    g_parse(g, corto_lang_o, FALSE, FALSE, "corto");
    g_parse(g, corto_vstore_o, FALSE, FALSE, "corto");
    g_parse(g, corto_native_o, FALSE, FALSE, "corto_native");
    g_parse(g, corto_secure_o, FALSE, FALSE, "corto_secure");

    if (!g_walkNoScope(g, c_apiWalkPackages, &walkData)) {
        goto error;
    }

    if (!local) {
        corto_chdir(cwd);
        corto_dealloc(cwd);
    }

    return 0;
error:
    return -1;
}
