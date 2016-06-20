/*
 * c_interface.c
 *
 *  Created on: Oct 4, 2012
 *      Author: sander
 */

#include "corto/corto.h"
#include "corto/gen/c/common/common.h"

typedef struct c_interfaceExisting {
    corto_string id;
    corto_string src;
} c_interfaceExisting;

typedef struct c_typeWalk_t {
    corto_generator g;
    g_file header;
    g_file source;
    g_file wrapper;
    g_file mainHeader;
    corto_uint16 firstComma;
    corto_bool generateHeader;
    corto_bool generateSource;
    corto_id sizeExpr;
    corto_ll generated;
} c_typeWalk_t;

static g_file c_interfaceOpenFile(corto_string name, c_typeWalk_t *data) {
    g_file result = g_fileOpen(data->g, name);
    if (result) {
        corto_llAppend(data->generated, corto_strdup(name));
        g_fileWrite(result, "/* $CORTO_GENERATED\n");
        g_fileWrite(result, " *\n");
        g_fileWrite(result, " * %s\n", name);
        g_fileWrite(result, " *\n");
        g_fileWrite(result, " * Only code written between the begin and end tags will be preserved\n");
        g_fileWrite(result, " * when the file is regenerated.\n");
        g_fileWrite(result, " */\n\n");
    }
    return result;
}

static int c_interfaceParamNameSource(corto_parameter *o, void *userData) {
    c_typeWalk_t* data = userData;
    corto_id name;
    if (data->firstComma) {
        g_fileWrite(data->source, ", ");
    }
    g_fileWrite(data->source, "%s", c_paramName(o->name, name));
    data->firstComma++;
    return 1;
}

static int c_interfaceParamCastDef(corto_parameter *o, void *userData) {
    c_typeWalk_t* data = userData;
    if (*o->name != '$') {
        corto_id name;
        if (data->firstComma) {
            g_fileWrite(data->header, ", ");
        }
        g_fileWrite(data->header, "%s", c_paramName(o->name, name));
        data->firstComma++;
    }
    return 1;
}

static corto_bool c_interfaceParamRequiresCast(corto_type t, corto_bool isReference) {
    if ((isReference || t->reference) &&
        (t->kind != CORTO_VOID) && (t->kind != CORTO_ANY) &&
        (corto_checkAttr(t, CORTO_ATTR_SCOPED))) {
        return TRUE;
    } else {
        return FALSE;
    }
}

static int c_interfaceParamCastWalk(corto_parameter *o, void *userData) {
    c_typeWalk_t* data = userData;
    corto_id specifier, postfix, name;

    if (data->firstComma) {
        g_fileWrite(data->header, ", ");
    }

    if (c_specifierId(data->g, o->type, specifier, NULL, postfix)) {
        goto error;
    }

    /* If parameter is a meta argument, stringify it */
    if (*o->name == '$') {
        if (!strcmp(o->name, "$__line")) {
            g_fileWrite(data->header, "__LINE__");
        } else if (!strcmp(o->name, "$__file")) {
            g_fileWrite(data->header, "__FILE__");
        } else {
            g_fileWrite(data->header, "#%s", o->name + 1);
        }
    } else {
        if (c_interfaceParamRequiresCast(o->type, o->passByReference)) {
            g_fileWrite(data->header, "%s(%s)", specifier, o->name);
        } else {
            g_fileWrite(data->header, "%s", c_paramName(o->name, name));
        }
    }

    data->firstComma = TRUE;

    return 1;
error:
    return 0;
}

static int c_interfaceCastMacro(corto_function o, corto_string functionName, c_typeWalk_t *data) {
    data->firstComma = FALSE;

    g_fileWrite(data->header, "#define %s(", functionName, functionName);

    if (c_procedureHasThis(o)) {
        g_fileWrite(data->header, "_this");
        data->firstComma = TRUE;
    }

    if (!c_paramWalk(o, c_interfaceParamCastDef, data)) {
        goto error;
    }

    g_fileWrite(data->header, ") _%s(", functionName);

    if (c_procedureHasThis(o)) {
        if (corto_procedure(corto_typeof(o))->kind != CORTO_METAPROCEDURE) {
            corto_id classId;
            corto_type parentType = corto_parentof(o);
            g_fullOid(data->g, parentType, classId);
            if (parentType->reference) {
                g_fileWrite(data->header, "%s(_this)", classId);
            } else {
                g_fileWrite(data->header, "_this", classId);
            }
        } else {
            g_fileWrite(data->header, "_this");
        }
        data->firstComma = TRUE;
    } else {
        data->firstComma = FALSE;
    }

    if (!c_paramWalk(o, c_interfaceParamCastWalk, data)) {
        goto error;
    }

    g_fileWrite(data->header, ")\n");

    return 0;
error:
    return -1;
}

/* Generate implementation for virtual methods */
static int c_interfaceGenerateVirtual(corto_method o, c_typeWalk_t* data) {
    corto_id id, returnTypeId, returnPostfix;
    corto_bool returnsValue;
    corto_id nameString;
    g_file originalSource = data->source;
    corto_bool cpp = !strcmp(gen_getAttribute(data->g, "c4cpp"), "true");

    /* Replace the source with the wrapper so that all nested functions use the correct outputfile.
     * This file will be restored at the end of the function */
    data->source = data->wrapper;

    c_writeExport(data->g, data->header);
    g_fileWrite(data->header, " ");
    if (c_decl(data->g, data->header, corto_function(o), TRUE, TRUE)) {
        goto error;
    }
    g_fileWrite(data->header, ";\n");

    g_fileWrite(data->wrapper, "\n");
    if (c_decl(data->g, data->wrapper, corto_function(o), TRUE, cpp)) {
        goto error;
    }

    if (((corto_function)o)->returnType &&
        ((corto_function(o)->returnType->kind != CORTO_VOID) ||
         corto_function(o)->returnType->reference))
    {
        returnsValue = TRUE;
        c_specifierId(data->g, corto_function(o)->returnType, returnTypeId, NULL, returnPostfix);
    } else {
        returnsValue = FALSE;
        strcpy(returnTypeId, "void");
    }

    /* Obtain string for function name */
    c_escapeString(corto_idof(o), nameString);

    /* Write casting macro to header */
    c_interfaceCastMacro(corto_function(o), g_fullOid(data->g, o, id), data);

    /* Begin of function */
    g_fileWrite(data->wrapper, "\n{\n");
    g_fileIndent(data->wrapper);
    g_fileWrite(data->wrapper, "static corto_uint32 _methodId;\n");
    g_fileWrite(data->wrapper, "corto_method _method;\n");
    if (returnsValue) {
        g_fileWrite(data->wrapper, "%s _result;\n", returnTypeId);
    }
    g_fileWrite(data->wrapper, "corto_interface _abstract;\n\n");
    if (!cpp) {
        g_fileWrite(data->wrapper, "_abstract = corto_interface(corto_typeof(this));\n\n");
    } else {
        g_fileWrite(data->wrapper, "_abstract = corto_interface(corto_typeof(_this));\n\n");
    }
    g_fileWrite(data->wrapper, "/* Determine methodId once, then cache it for subsequent calls. */\n");
    g_fileWrite(data->wrapper, "if (!_methodId) {\n");
    g_fileIndent(data->wrapper);
    g_fileWrite(data->wrapper, "_methodId = corto_interface_resolveMethodId(_abstract, \"%s\");\n", nameString);
    g_fileDedent(data->wrapper);
    g_fileWrite(data->wrapper, "}\n");
    g_fileWrite(
      data->wrapper,
      "corto_assert(_methodId, \"virtual '%s' not found in '%%s'%%s%%s\", corto_fullpath(NULL, _abstract), corto_lasterr()?\": \":\"\", corto_lasterr());\n\n",
      nameString);
    g_fileWrite(data->wrapper, "/* Lookup method-object. */\n");
    g_fileWrite(data->wrapper, "_method = corto_interface_resolveMethodById(_abstract, _methodId);\n");
    if (!cpp) {
        g_fileWrite(data->wrapper, "corto_assert(_method != NULL, \"unresolved method '%%s::%s@%%d'\", corto_idof(this), _methodId);\n\n", nameString);
        if (returnsValue) {
            g_fileWrite(data->wrapper, "corto_call(corto_function(_method), &_result, this");
        } else {
            g_fileWrite(data->wrapper, "corto_call(corto_function(_method), NULL, this");
        }
    } else {
        g_fileWrite(data->wrapper, "corto_assert(_method != NULL, \"unresolved method '%%s::%s@%%d'\", corto_idof(_this), _methodId);\n\n", nameString);
        if (returnsValue) {
            g_fileWrite(data->wrapper, "corto_call(corto_function(_method), &_result, _this");
        } else {
            g_fileWrite(data->wrapper, "corto_call(corto_function(_method), NULL, _this");
        }
    }

    data->firstComma = 3;
    if (!c_paramWalk(o, c_interfaceParamNameSource, data)) {
        goto error;
    }
    g_fileWrite(data->wrapper, ");\n");

    if (returnsValue) {
        g_fileWrite(data->wrapper, "\n");
        g_fileWrite(data->wrapper, "return _result;\n");
    }
    g_fileDedent(data->wrapper);
    g_fileWrite(data->wrapper, "}\n");

    data->source = originalSource;

    return 0;
error:
    return -1;
}

/* Generate methods for class */
static int c_interfaceClassProcedure(corto_object o, void *userData) {
    c_typeWalk_t* data;
    corto_bool defined = FALSE;

    data = userData;

    /* Only generate code for procedures */
    if (corto_class_instanceof(corto_procedure_o, corto_typeof(o))) {
        corto_id fullname, functionName, signatureName, returnSpec, returnPostfix;
        corto_string snippet, header;
        corto_procedureKind kind;
        corto_type returnType;
        corto_string doStubs = gen_getAttribute(data->g, "stubs");
        corto_bool cpp = !strcmp(gen_getAttribute(data->g, "c4cpp"), "true");

        kind = corto_procedure(corto_typeof(o))->kind;
        defined = corto_checkState(o, CORTO_DEFINED) && (corto_function(o)->kind != CORTO_PROCEDURE_STUB);

        /* Check whether generation of stubs must be forced */
        if (doStubs) {
            if (!strcmp(doStubs, "true")) {
                defined = TRUE;
            } else if(!strcmp(doStubs, "false")) {
                defined = FALSE;
            }
        }

        /* If procedure is a delegate, generate delegate forwarding-function. Nothing
         * further needs to be generated in the sourcefile for a delegate. */
        switch (kind) {
        case CORTO_METHOD:
            if (corto_method(o)->_virtual) {
                c_interfaceGenerateVirtual(o, data);
            }
            break;
        default:
            if (defined) {
                goto ok;
            }
            break;
        }

        /* Generate function-return type string */
        returnType = ((corto_function)o)->returnType;
        if (returnType) {
            c_specifierId(data->g, returnType, returnSpec, NULL, returnPostfix);
        } else {
            strcpy(returnSpec, "void");
            *returnPostfix = '\0';
        }

        corto_fullpath(fullname, o);
        c_functionName(data->g, o, functionName);
        if (corto_function(o)->overloaded) {
            strcpy(signatureName, fullname + 1); /* Skip scope */
        } else {
            corto_signatureName(fullname + 1, signatureName);
        }

        g_fileWrite(data->header, "\n");

        /* Start of function */
        c_writeExport(data->g, data->header);
        g_fileWrite(data->header, " ");
        if (c_decl(data->g, data->header, corto_function(o), FALSE, TRUE)) {
            goto error;
        }
        g_fileWrite(data->header, ";\n");

        /* Write to sourcefile */
        g_fileWrite(data->source, "\n");

        /* Lookup header for function */
        header = g_fileLookupHeader(data->source, signatureName);
        if (header) {
            g_fileWrite(data->source, "/* $header(%s)", signatureName);
            g_fileWrite(data->source, "%s", header);
            g_fileWrite(data->source, "$end */\n");
        }

        if (c_decl(data->g, data->source, corto_function(o), FALSE, cpp)) {
            goto error;
        }

        /* Begin of function */
        g_fileWrite(data->source, "\n{\n");
        g_fileWrite(data->source, "/* $begin(%s)", signatureName);
        g_fileIndent(data->source);

        /* Support both short and full name when function is not overloaded */
        snippet = g_fileLookupSnippet(data->source, signatureName);
        if (!snippet && (!corto_function(o)->overloaded)) {
            snippet = g_fileLookupSnippet(data->source, fullname);
        }

        /* Lookup if there is an existing implementation. */
        if (!defined) {
            if (!snippet) {
                g_fileWrite(data->source, " */\n\n");
                g_fileWrite(data->source, "/* << Insert implementation >> */\n\n");
                g_fileDedent(data->source);
                g_fileWrite(data->source, "/* ");
            } else {
                g_fileDedent(data->source);
                g_fileWrite(data->source, "%s", snippet);
            }
        } else {
            corto_id id;
            corto_uint32 i;
            corto_parameter *p;

            g_fileWrite(data->source, " */\n");

            if ((returnType->kind != CORTO_VOID) || (returnType->reference)) {
                corto_id specifier;
                g_fullOid(data->g, returnType, specifier);
                g_fileWrite(data->source, "%s _result;\n", specifier);
            } else {
                returnType = NULL;
            }

            /* If function is already defined, it is already implemented. The generator will generate a stub instead. */
            g_fileWrite(data->source, "corto_call(corto_function(%s_o)", g_fullOid(data->g, o, id));
            if (returnType) {
                g_fileWrite(data->source, ",&_result");
            } else {
                g_fileWrite(data->source, ",NULL");
            }
            if (corto_class_instanceof(corto_interface_o, corto_parentof(o))) {
                if (corto_procedure(corto_typeof(o))->kind != CORTO_FUNCTION) {
                    if (!cpp) {
                        g_fileWrite(data->source, ",this");
                    } else {
                        g_fileWrite(data->source, ",_this");
                    }
                }
            }
            for (i=0; i<corto_function(o)->parameters.length; i++) {
                p = &corto_function(o)->parameters.buffer[i];
                g_fileWrite(data->source, ",%s", g_id(data->g, p->name, id));
            }
            g_fileWrite(data->source, ");\n");
            if (returnType) {
                g_fileWrite(data->source, "return _result;\n");
            }
            g_fileDedent(data->source);
            g_fileWrite(data->source, "/* ");
        }

        g_fileWrite(data->source, "$end */\n");
        g_fileWrite(data->source, "}\n");

        /* Write the macro wrapper for automatic casting of parameter types */
        c_interfaceCastMacro(o, functionName, data);
    }

ok:
    return 1;
error:
    return 0;
}

/* Check if there are procedures in the scope of an object. */
static int c_interfaceCheckProcedures(void *o, void *udata) {
    CORTO_UNUSED(udata);

    /* If the type of the type of the object is a procedure, return 0. */
    if (corto_class_instanceof(corto_procedure_o, corto_typeof(o))) {
        return 0;
    }
    if (corto_instanceof(corto_delegate_o, o)) {
        return 0;
    }

    return 1;
}

/* Open generator headerfile */
static g_file c_interfaceHeaderFileOpen(corto_generator g, corto_object o, c_typeWalk_t *data) {
    g_file result;
    corto_id headerFileName;
    corto_object topLevelObject = g_getCurrent(g);
    corto_id path;

    /* Create file */
    c_filename(g, headerFileName, o, "h");

    result = g_fileOpen(g, headerFileName);
    if (!result) {
        goto error;
    }

    if (!data->mainHeader) {
        corto_id mainHeader;
        if (o == topLevelObject) {
            data->mainHeader = result;
        } else {
            if (strcmp(gen_getAttribute(g, "bootstrap"), "true")) {
                c_filename(g, headerFileName, o, "h");
            } else {
                sprintf(headerFileName, "_%s.h", corto_idof(o));
            }

            data->mainHeader = g_fileOpen(g, mainHeader);
            if (!data->mainHeader) {
                goto error;
            }
        }
    }

    if (o != topLevelObject) {
        c_include(data->g, data->mainHeader, o);
    }

    /* Print standard comments and includes */
    corto_path(path, root_o, o, "_");
    corto_strupper(path);

    g_fileWrite(result, "/* %s\n", headerFileName);
    g_fileWrite(result, " *\n");
    g_fileWrite(result, " * This file contains generated code. Do not modify!\n");
    g_fileWrite(result, " */\n\n");
    g_fileWrite(result, "#ifndef %s_H\n", path);
    g_fileWrite(result, "#define %s_H\n\n", path);

    /* Include corto, if not generating corto */
    if (o != corto_o) {
        c_includeFrom(g, result, corto_o, "corto.h");
    }

    if (!strcmp(gen_getAttribute(g, "bootstrap"), "true")) {
        g_fileWrite(result, "#include <%s/_interface.h>\n", g_getName(g));
    } else {
        c_includeFrom(g, result, g_getCurrent(g), "_interface.h");
    }

    /* If a header exists, write it */
    if (topLevelObject) {
        corto_string snippet;
        if ((snippet = g_fileLookupHeader(result, ""))) {
            g_fileWrite(result, "/* $header()");
            g_fileWrite(result, "%s", snippet);
            g_fileWrite(result, "$end */\n\n");
        }
    }

    if (o == topLevelObject) {
        /* Add header files for dependent packages */
        corto_ll packages = corto_loadGetPackages();
        if (packages) {
            corto_iter iter = corto_llIter(packages);
            while (corto_iterHasNext(&iter)) {
                corto_string str = corto_iterNext(&iter);
                corto_string package = corto_locate(str, CORTO_LOCATION_FULLNAME);
                if (!package) {
                    corto_seterr("package.txt contains unresolved package '%s'", str);
                    goto error;
                } else {
                    corto_string name = corto_locate(str, CORTO_LOCATION_NAME);
                    g_fileWrite(
                      data->mainHeader, "#include <%s/%s.h>\n", package, name);
                    corto_dealloc(name);
                    corto_dealloc(package);
                }
            }
            corto_loadFreePackages(packages);
            g_fileWrite(result, "\n");
        }
    }

    c_includeFrom(g, result, g_getCurrent(g), "_type.h");
    c_includeFrom(g, result, g_getCurrent(g), "_api.h");
    c_includeFrom(g, result, g_getCurrent(g), "_meta.h");

    g_fileWrite(result, "\n");

    if (topLevelObject) {
        corto_string snippet;
        if ((snippet = g_fileLookupSnippet(result, ""))) {
            g_fileWrite(result, "\n");
            g_fileWrite(result, "/* $body()");
            g_fileWrite(result, "%s", snippet);
            g_fileWrite(result, "$end */\n\n");
        }
    }

    g_fileWrite(result, "#ifdef __cplusplus\n");
    g_fileWrite(result, "extern \"C\" {\n");
    g_fileWrite(result, "#endif\n\n");

    return result;
error:
    return NULL;
}

/* Close headerfile */
static void c_interfaceHeaderFileClose(g_file file) {

    /* Print standard comments and includes */
    g_fileWrite(file, "\n");
    g_fileWrite(file, "#ifdef __cplusplus\n");
    g_fileWrite(file, "}\n");
    g_fileWrite(file, "#endif\n");
    g_fileWrite(file, "#endif\n\n");
}

static g_file c_interfaceWrapperFileOpen(corto_generator g) {
    g_file result;
    corto_char fileName[512];
    corto_bool cpp = !strcmp(gen_getAttribute(g, "c4cpp"), "true");

    corto_object o = g_getCurrent(g);
    sprintf(fileName, "_wrapper.%s", cpp ? "cpp" : "c");

    if (!strcmp(gen_getAttribute(g, "bootstrap"), "true")) {
        result = g_fileOpen(g, fileName);
    } else {
        result = g_hiddenFileOpen(g, fileName);
    }

    if(!result) {
        goto error;
    }

    /* Print standard comments and includes */
    g_fileWrite(result, "/* %s\n", fileName);
    g_fileWrite(result, " *\n");
    g_fileWrite(result, " * This file contains wrapper functions for %s.\n",
        corto_fullpath(NULL, o));
    g_fileWrite(result, " */\n\n");

    c_include(g, result, g_getCurrent(g));
    c_includeFrom(g, result, g_getCurrent(g), "_meta.h");

    return result;
error:
    return NULL;
}

/* Open generator sourcefile */
static g_file c_interfaceSourceFileOpen(corto_object o, c_typeWalk_t *data) {
    g_file result;
    corto_id fileName;

    c_filename(data->g, fileName, o, "c");

    result = c_interfaceOpenFile(fileName, data);
    if (!result) {
        goto error;
    }

    /* Include main header */
    c_include(data->g, result, g_getCurrent(data->g));

    return result;
error:
    return NULL;
}

/* Generate interface for class */
static corto_int16 c_interfaceObject(corto_object o, c_typeWalk_t* data) {
    corto_string snippet;
    int hasProcedures;
    corto_bool isInterface;
    corto_bool isTopLevelObject;
    corto_bool isBootstrap = !strcmp(gen_getAttribute(data->g, "bootstrap"), "true");

    hasProcedures = !corto_scopeWalk(o, c_interfaceCheckProcedures, NULL);
    isInterface = corto_class_instanceof(corto_interface_o, o);
    isTopLevelObject = o == g_getCurrent(data->g);

    /* Always generate header for interfaces */
    if (hasProcedures || isInterface || isTopLevelObject) {
        data->header = c_interfaceHeaderFileOpen(data->g, o, data);
        if (!data->header) {
            goto error;
        }
    }

    /* An interface implementation file is generated when the object is
     * an interface and has procedures. When the object is not an interface
     * but does have procedures (typical example is callbacks or static functions)
     * these are appended to the header of the first scope in the hierarchy. */
    if (hasProcedures || (isTopLevelObject && !isBootstrap)) {

        /* Create a wrapper file if it was not already created */
        if (!data->wrapper) {
            data->wrapper = c_interfaceWrapperFileOpen(data->g);
            if (!data->wrapper) {
                goto error;
            }
        }

        /* Open sourcefile */
        data->source = c_interfaceSourceFileOpen(o, data);
        if (!data->source) {
            goto error;
        }

        /* If a header exists, write it */
        if ((snippet = g_fileLookupHeader(data->source, ""))) {
            g_fileWrite(data->source, "\n");
            g_fileWrite(data->source, "/* $header()");
            g_fileWrite(data->source, "%s", snippet);
            g_fileWrite(data->source, "$end */\n");
        }

        /* Walk scope */
        if (!corto_scopeWalk(o, c_interfaceClassProcedure, data)) {
            goto error;
        }

        /* If top level file, generate main function */
        if (isTopLevelObject && !isBootstrap) {
            g_fileWrite(data->source, "\n");
            if ((snippet = g_fileLookupHeader(data->source, "main"))) {
                g_fileWrite(data->source, "\n");
                g_fileWrite(data->source, "/* $header(main)");
                g_fileWrite(data->source, "%s", snippet);
                g_fileWrite(data->source, "$end */\n");
            }
            g_fileWrite(data->source, "int %sMain(int argc, char* argv[]) {\n", corto_idof(o));
            g_fileWrite(data->source, "/* $begin(main)");
            g_fileIndent(data->source);
            if ((snippet = g_fileLookupSnippet(data->source, "main"))) {
                g_fileWrite(data->source, "%s", snippet);
                g_fileWrite(data->source, "$end */\n");
                g_fileDedent(data->source);
            } else {
                g_fileWrite(data->source, " */\n\n");
                g_fileWrite(data->source, "/* Insert code that must be run when component is loaded */\n\n");
                g_fileWrite(data->source, "return 0;\n");
                g_fileDedent(data->source);
                g_fileWrite(data->source, "/* $end */\n");
            }
            g_fileWrite(data->source, "}\n");
        }

        g_fileClose(data->source);
    }

    /* Close */
    if ((hasProcedures || isInterface) && !isTopLevelObject) {
        c_interfaceHeaderFileClose(data->header);
    }

    return 0;
error:
    return 1;
}

/* Walk interfaces */
static int c_interfaceWalk(corto_object o, void *userData) {
    c_typeWalk_t* data;

    data = userData;

    /* Parse interfaces */
    if (c_interfaceObject(o, data)) {
        goto error;
    }

    /* Walk scope of object */
    if (!corto_scopeWalk(o, c_interfaceWalk, data)) {
        goto error;
    }

    return 1;
error:
    return 0;
}

static corto_bool c_interfaceIsGenerated(corto_string file) {
    corto_string content = corto_fileLoad(file);
    corto_bool result = FALSE;

    if (content) {
        if (strlen(content) > 1024) {
            /* Token must appear before the first 1024 bytes */
            content[1024] = '\0';
        }

        if (strstr(content, "$CORTO_GENERATED")) {
            result = TRUE;
        }

        corto_dealloc(content);
    }
    return result;
}

static corto_bool c_interfaceWasGeneratedNow(
    corto_string name,
    c_typeWalk_t *data)
{
    corto_iter iter = corto_llIter(data->generated);

    while(corto_iterHasNext(&iter)) {
        corto_string file = corto_iterNext(&iter);
        if (!strcmp(file, name)) {
            return TRUE;
        }
    }

    return FALSE;
}

/* Mark files that haven't been regenerated */
static int c_interfaceMarkUnusedFiles(c_typeWalk_t *data) {
    corto_ll files = corto_opendir("./src");
    corto_iter iter = corto_llIter(files);

    while(corto_iterHasNext(&iter)) {
        corto_string file = corto_iterNext(&iter);
        corto_id id;
        sprintf(id, "./src/%s", file);
        if (c_interfaceIsGenerated(id)) {
            if (!c_interfaceWasGeneratedNow(file, data)) {
                if (!strstr(id, ".old")) {
                    corto_id newname;
                    sprintf(newname, "src/%s.old", file);
                    rename (id, newname);
                    printf("c_interface: %s: stale file, please remove (renamed to %s.old)\n", file, file);

                    /* Remove corresponding headerfile */
                    sprintf(newname, "include/%s", file);
                    char *ext = strchr(newname, '.');
                    *(ext + 1) = 'h';
                    *(ext + 2) = '\0';
                    corto_rm(newname);
                } else {
                    printf("c_interface: %s: stale file, please remove\n", file);
                }
            }
        }
    }

    iter = corto_llIter(data->generated);
    while (corto_iterHasNext(&iter)) {
        corto_dealloc(corto_iterNext(&iter));
    }

    corto_closedir(files);
    corto_llFree(data->generated);

    return 0;
}

/* Entry point for generator */
int corto_genMain(corto_generator g) {
    c_typeWalk_t walkData;

    /* Create source and include directories */
    corto_mkdir("src");
    corto_mkdir("include");

    if (strcmp(gen_getAttribute(g, "bootstrap"), "true")) {
        corto_mkdir(".corto");
    }

    /* Default prefixes for corto namespaces */
    gen_parse(g, corto_o, FALSE, FALSE, "");
    gen_parse(g, corto_lang_o, FALSE, FALSE, "corto");
    gen_parse(g, corto_core_o, FALSE, FALSE, "corto");
    gen_parse(g, corto_native_o, FALSE, FALSE, "corto_native");

    /* Prepare walkData, create header- and sourcefile */
    walkData.g = g;
    walkData.header = NULL;
    walkData.source = NULL;
    walkData.wrapper = NULL;
    walkData.mainHeader = NULL;
    walkData.generated = corto_llNew();

    /* Walk objects, generate procedures and class members */
    if (!g_walkNoScope(g, c_interfaceWalk, &walkData)) {
        goto error;
    }

    if (walkData.mainHeader) {
        c_interfaceHeaderFileClose(walkData.mainHeader);
    }


    c_interfaceMarkUnusedFiles(&walkData);

    return 0;
error:
    return -1;
}
