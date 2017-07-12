/*
 * c_interface.c
 *
 *  Created on: Oct 4, 2012
 *      Author: sander
 */

#include "corto/corto.h"
#include "corto/gen/c/common/common.h"
#include <corto/cdiff/cdiff.h>

#define GENERATED_MARKER "This is a managed file. Do not delete this comment."

typedef struct c_interfaceExisting {
    corto_string id;
    corto_string src;
} c_interfaceExisting;

typedef struct c_typeWalk_t {
    g_generator g;
    cdiff_file source;
    g_file wrapper;
    g_file mainHeader;
    g_file interfaceHeader;
    corto_uint16 firstComma;
    corto_bool generateHeader;
    corto_bool generateSource;
    corto_id sizeExpr;
    corto_ll generated;
    corto_bool mainWritten;
} c_typeWalk_t;

static cdiff_file c_interfaceOpenFile(corto_string name, c_typeWalk_t *data) {
    corto_id filepath;
    cdiff_file result = cdiff_file_open(g_filePath(data->g, filepath, name));
    if (result) {
        corto_ll_append(data->generated, corto_strdup(name));
        cdiff_file_write(result, "/* %s */\n\n", GENERATED_MARKER);
    }
    return result;
}

static int c_interfaceParamNameSource(corto_parameter *o, void *userData) {
    c_typeWalk_t* data = userData;
    corto_id name;
    if (data->firstComma) {
        g_fileWrite(data->wrapper, ", ");
    }
    g_fileWrite(data->wrapper, "%s", c_paramName(o->name, name));
    data->firstComma++;
    return 1;
}

static int c_interfaceParamTypeSource(corto_parameter *o, void *userData) {
    c_typeWalk_t* data = userData;
    corto_id type;
    if (data->firstComma) {
        g_fileWrite(data->wrapper, ", ");
    }
    g_fileWrite(data->wrapper, "%s%s",
        g_fullOid(data->g, o->type, type),
        c_paramRequiresPtr(o) ? "*" : "");
    data->firstComma++;
    return 1;
}

static int c_interfaceParamCastDef(corto_parameter *o, void *userData) {
    c_typeWalk_t* data = userData;
    if (*o->name != '$') {
        corto_id name;
        if (data->firstComma) {
            g_fileWrite(data->interfaceHeader, ", ");
        }
        g_fileWrite(data->interfaceHeader, "%s", c_paramName(o->name, name));
        data->firstComma++;
    }
    return 1;
}

static corto_bool c_interfaceParamRequiresCast(corto_type t, corto_bool isReference, corto_inout inout) {
    if ((isReference || t->reference) &&
        (t->kind != CORTO_VOID) && (t->kind != CORTO_ANY) &&
        (corto_checkAttr(t, CORTO_ATTR_NAMED)) &&
        (!inout)) 
    {
        return TRUE;
    } else {
        return FALSE;
    }
}

static int c_interfaceParamCastWalk(corto_parameter *o, void *userData) {
    c_typeWalk_t* data = userData;
    corto_id specifier, postfix, name;

    if (data->firstComma) {
        g_fileWrite(data->interfaceHeader, ", ");
    }

    if (c_specifierId(data->g, o->type, specifier, NULL, postfix)) {
        goto error;
    }

    /* If parameter is a meta argument, stringify it */
    if (*o->name == '$') {
        if (!strcmp(o->name, "$__line")) {
            g_fileWrite(data->interfaceHeader, "__LINE__");
        } else if (!strcmp(o->name, "$__file")) {
            g_fileWrite(data->interfaceHeader, "__FILE__");
        } else {
            g_fileWrite(data->interfaceHeader, "#%s", o->name + 1);
        }
    } else {
        if (c_interfaceParamRequiresCast(o->type, o->passByReference, o->inout)) {
            g_fileWrite(data->interfaceHeader, "%s(%s)", specifier, o->name);
        } else {
            g_fileWrite(data->interfaceHeader, "%s", c_paramName(o->name, name));
        }
    }

    data->firstComma = TRUE;

    return 1;
error:
    return 0;
}

static int c_interfaceCastMacro(corto_function o, char *macroName, char *functionName, char *prefix, c_typeWalk_t *data) {
    data->firstComma = FALSE;

    if (prefix) {
        g_fileWrite(data->interfaceHeader, "#define %s%s(", prefix, macroName);
    } else {
        g_fileWrite(data->interfaceHeader, "#define %s(", macroName);
    }

    if (c_procedureHasThis(o)) {
        g_fileWrite(data->interfaceHeader, "_this");
        data->firstComma = TRUE;
    }

    if (!c_paramWalk(o, c_interfaceParamCastDef, data)) {
        goto error;
    }

    g_fileWrite(data->interfaceHeader, ") _%s(", functionName);

    if (c_procedureHasThis(o)) {
        corto_type thisType = corto_procedure(corto_typeof(o))->thisType;
        if (!thisType || thisType->reference) {
            corto_id classId;
            corto_type parentType = corto_parentof(o);
            g_fullOid(data->g, parentType, classId);
            if (parentType->reference) {
                g_fileWrite(data->interfaceHeader, "%s(_this)", classId);
            } else {
                g_fileWrite(data->interfaceHeader, "_this", classId);
            }
        } else {
            g_fileWrite(data->interfaceHeader, "_this");
        }
        data->firstComma = TRUE;
    } else {
        data->firstComma = FALSE;
    }

    if (!c_paramWalk(o, c_interfaceParamCastWalk, data)) {
        goto error;
    }

    g_fileWrite(data->interfaceHeader, ")\n");

    return 0;
error:
    return -1;
}

/* Generate implementation for virtual methods */
static int c_interfaceGenerateVirtual(corto_method o, c_typeWalk_t* data) {
    corto_id id, returnTypeId;
    corto_bool returnsValue;
    char *nameString = NULL;

    /* Write casting macro to header */
    g_fileWrite(data->interfaceHeader, "\n");
    g_fullOid(data->g, o, id);
    c_interfaceCastMacro(corto_function(o), id, id, NULL, data);

    c_writeExport(data->g, data->interfaceHeader);
    g_fileWrite(data->interfaceHeader, "\n");

    corto_buffer declBuffer = CORTO_BUFFER_INIT;
    if (c_decl(data->g, &declBuffer, corto_function(o), TRUE, TRUE /* never write 'this' in public header */, FALSE)) {
        goto error;
    }
    char *decl = corto_buffer_str(&declBuffer);
    g_fileWrite(data->interfaceHeader, "%s;\n", decl);
    g_fileWrite(data->wrapper, "%s\n", decl);
    free(decl);

    if (((corto_function)o)->returnType &&
        ((corto_function(o)->returnType->kind != CORTO_VOID) ||
         corto_function(o)->returnType->reference))
    {
        returnsValue = TRUE;
        c_typeret(data->g, corto_function(o)->returnType, C_ByValue, returnTypeId);
    } else {
        returnsValue = FALSE;
        strcpy(returnTypeId, "void");
    }

    /* Obtain string for function name */
    nameString = c_escapeString(corto_idof(o));

    /* Begin of function */
    g_fileWrite(data->wrapper, "\n{\n");
    g_fileIndent(data->wrapper);
    g_fileWrite(data->wrapper, "static corto_uint32 _methodId;\n");
    g_fileWrite(data->wrapper, "corto_method _method;\n");
    if (returnsValue) {
        g_fileWrite(data->wrapper, "%s _result;\n", returnTypeId);
    }
    g_fileWrite(data->wrapper, "corto_interface _abstract;\n\n");

    g_fileWrite(data->wrapper, "_abstract = corto_interface(corto_typeof(_this));\n\n");
    g_fileWrite(data->wrapper, "/* Determine methodId once, then cache it for subsequent calls. */\n");
    g_fileWrite(data->wrapper, "if (!_methodId) {\n");
    g_fileIndent(data->wrapper);
    g_fileWrite(data->wrapper, "_methodId = corto_interface_resolveMethodId(_abstract, \"%s\");\n", nameString);
    g_fileDedent(data->wrapper);
    g_fileWrite(data->wrapper, "}\n");
    g_fileWrite(
      data->wrapper,
      "corto_assert(_methodId, \"virtual '%s' not found in '%%s'%%s%%s\", corto_fullpath(NULL, _abstract), corto_lasterr() ? \": \" : \"\", corto_lasterr() ? corto_lasterr() : \"\");\n\n",
      nameString);
    g_fileWrite(data->wrapper, "/* Lookup method-object. */\n");
    g_fileWrite(data->wrapper, "_method = corto_interface_resolveMethodById(_abstract, _methodId);\n");
    g_fileWrite(data->wrapper,
      "corto_assert(_method != NULL, \"unresolved method '%%s::%s@%%d'\", corto_idof(_this), _methodId);\n\n",
      nameString);

    g_fileWrite(data->wrapper, "if (corto_function(_method)->kind == CORTO_PROCEDURE_CDECL) {\n");
    g_fileIndent(data->wrapper);
    if (returnsValue) {
        g_fileWrite(data->wrapper, "_result = ");
    }
    data->firstComma = 1;
    g_fileWrite(data->wrapper, "((%s ___ (*)(corto_object", returnTypeId);
    if (!c_paramWalk(o, c_interfaceParamTypeSource, data)) {
        goto error;
    }
    g_fileWrite(data->wrapper, "))((corto_function)_method)->fptr)(_this");
    if (!c_paramWalk(o, c_interfaceParamNameSource, data)) {
        goto error;
    }
    g_fileWrite(data->wrapper, ");\n");
    g_fileDedent(data->wrapper);
    g_fileWrite(data->wrapper, "} else {\n");
    g_fileIndent(data->wrapper);

    if (returnsValue) {
        g_fileWrite(data->wrapper, "corto_call(corto_function(_method), &_result, _this");
    } else {
        g_fileWrite(data->wrapper, "corto_call(corto_function(_method), NULL, _this");
    }
    data->firstComma = 3;
    if (!c_paramWalk(o, c_interfaceParamNameSource, data)) {
        goto error;
    }
    g_fileWrite(data->wrapper, ");\n");

    g_fileDedent(data->wrapper);
    g_fileWrite(data->wrapper, "}\n");


    if (returnsValue) {
        g_fileWrite(data->wrapper, "\n");
        g_fileWrite(data->wrapper, "return _result;\n");
    }
    g_fileDedent(data->wrapper);
    g_fileWrite(data->wrapper, "}\n");

    corto_dealloc(nameString);

    return 0;
error:
    if (nameString) corto_dealloc(nameString);
    return -1;
}

/* Generate methods for class */
static int c_interfaceClassProcedure(corto_object o, void *userData) {
    c_typeWalk_t* data;
    corto_bool defined = FALSE;

    data = userData;

    /* Only generate code for procedures */
    if (corto_class_instanceof(corto_procedure_o, corto_typeof(o))) {
        corto_id fullname, functionName;
        corto_type returnType = corto_function(o)->returnType;
        corto_string doStubs = g_getAttribute(data->g, "stubs");
        corto_bool cpp = !strcmp(g_getAttribute(data->g, "c4cpp"), "true");
        
        corto_fullpath(fullname, o);
        c_functionName(data->g, o, functionName);

        defined = corto_checkState(o, CORTO_DEFINED) && (corto_function(o)->kind != CORTO_PROCEDURE_STUB);

        /* Check whether generation of stubs must be forced */
        if (doStubs) {
            if (!strcmp(doStubs, "true")) {
                defined = TRUE;
            } else if(!strcmp(doStubs, "false")) {
                defined = FALSE;
            }
        }

        /* Write forward declaration */
        g_fileWrite(data->interfaceHeader, "\n");
        g_fileWrite(data->interfaceHeader, "/* forward declaration of the function. the '_' in front of the name allows\n");
        g_fileWrite(data->interfaceHeader, " * insertion of type-safe casts */\n");

        c_writeExport(data->g, data->interfaceHeader);
        g_fileWrite(data->interfaceHeader, "\n");

        corto_buffer declBuffer = CORTO_BUFFER_INIT;
        if (c_decl(data->g, &declBuffer, corto_function(o), FALSE, TRUE /* never use 'this' in public headers */, FALSE)) {
            goto error;
        }
        char *decl = corto_buffer_str(&declBuffer);
        g_fileWrite(data->interfaceHeader, "%s;\n", decl);
        free(decl);

        /* Write implicit macro wrapper for automatic casting of parameter types */
        corto_id macro;
        g_fileWrite(data->interfaceHeader, "\n");
        g_fileWrite(data->interfaceHeader, "/* implicit type-safe macro (not available for project implementation) */\n");
        g_fileWrite(data->interfaceHeader, "#ifndef %s\n", c_buildingMacro(data->g, macro));
        c_interfaceCastMacro(o, functionName, functionName, NULL, data);

        /* When building the project, casting macro's are explicit. This is done
         * so that the function names don't have to start with '_' */
        g_fileWrite(data->interfaceHeader, "#else\n");
        g_fileWrite(data->interfaceHeader, "#define %s _%s\n", functionName, functionName);
        
        if (strcmp(g_getAttribute(data->g, "bootstrap"), "true")) {
            corto_id localName;
            c_functionLocalName(data->g, o, localName);
            g_fileWrite(data->interfaceHeader, "\n");
            g_fileWrite(data->interfaceHeader, "/* Allow for type-safe, shorthand version to be used within project. In\n");
            g_fileWrite(data->interfaceHeader, " * C++ the name clash risk is too big because '::' is used for scoping */\n");
            g_fileWrite(data->interfaceHeader, "#ifndef __cplusplus\n");
            c_interfaceCastMacro(o, localName, functionName, NULL, data);
            g_fileWrite(data->interfaceHeader, "#endif\n");
        }
        g_fileWrite(data->interfaceHeader, "#endif\n");

        /* Write explicit casting macro */
        g_fileWrite(data->interfaceHeader, "\n");
        g_fileWrite(data->interfaceHeader, "/* explicit type-safe macro */\n");
        c_interfaceCastMacro(o, functionName, functionName, "safe_", data);

        /* Write to sourcefile */
        cdiff_file_write(data->source, "\n");

        /* Open element for function */
        cdiff_file_elemBegin(data->source, functionName);

        /* Write header for function */
        if (c_decl(data->g, &declBuffer, corto_function(o), FALSE, cpp, TRUE)) {
            goto error;
        }
        decl = corto_buffer_str(&declBuffer);

        cdiff_file_headerBegin(data->source);
        cdiff_file_write(data->source, "%s\n{", decl);
        cdiff_file_headerEnd(data->source);
        free(decl);

        /* Write body for function if this is first time function is written */
        if (!cdiff_file_bodyBegin(data->source)) {
            cdiff_file_write(data->source, "\n");
            cdiff_file_indent(data->source);
            cdiff_file_write(data->source, "/* Insert implementation */\n");
            cdiff_file_dedent(data->source);

            /* Lookup if there is an existing implementation. */
            if (defined) {
                corto_id id;
                corto_uint32 i;
                corto_parameter *p;

                if ((returnType->kind != CORTO_VOID) || (returnType->reference)) {
                    corto_id specifier;
                    c_typeret(data->g, returnType, C_ByValue, specifier);
                    cdiff_file_write(data->source, "%s _result;\n", specifier);
                } else {
                    returnType = NULL;
                }

                /* If function is already defined, it is already implemented. The generator will generate a stub instead. */
                cdiff_file_write(data->source, "corto_call(corto_function(%s_o)", g_fullOid(data->g, o, id));
                if (returnType) {
                    cdiff_file_write(data->source, ",&_result");
                } else {
                    cdiff_file_write(data->source, ",NULL");
                }
                if (corto_class_instanceof(corto_interface_o, corto_parentof(o))) {
                    if (corto_procedure(corto_typeof(o))->hasThis) {
                        if (!cpp) {
                            cdiff_file_write(data->source, ",this");
                        } else {
                            cdiff_file_write(data->source, ",_this");
                        }
                    }
                }
                for (i=0; i<corto_function(o)->parameters.length; i++) {
                    p = &corto_function(o)->parameters.buffer[i];
                    cdiff_file_write(data->source, ",%s", g_id(data->g, p->name, id));
                }
                cdiff_file_write(data->source, ");\n");
                if (returnType) {
                    cdiff_file_write(data->source, "return _result;\n");
                }
                cdiff_file_dedent(data->source);
            }

            cdiff_file_write(data->source, "}\n");
            cdiff_file_bodyEnd(data->source);
        }

        cdiff_file_elemEnd(data->source);

        if (corto_function(o)->overridable) {
            c_interfaceGenerateVirtual(o, data);
        }
    }

    return 1;
error:
    return 0;
}

/* Check if there are procedures in the scope of an object. */
static int c_interfaceCheckProcedures(void *o, void *ctx) {
    CORTO_UNUSED(ctx);

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
static corto_int16 c_interfaceHeaderWrite(
    g_generator g,
    g_file result,
    corto_object o,
    corto_string name,
    corto_string headerFileName,
    corto_bool mainHeader,
    c_typeWalk_t *data)
{
    corto_bool bootstrap = !strcmp(g_getAttribute(g, "bootstrap"), "true");
    corto_bool local = !strcmp(g_getAttribute(g, "local"), "true");
    corto_bool app = !strcmp(g_getAttribute(g, "app"), "true");

    corto_bool error = FALSE;
    corto_id path;

    CORTO_UNUSED(data);

    strcpy(path, name);
    corto_strupper(path);
    char *ptr, ch;
    for (ptr = path; (ch = *ptr); ptr++) if (ch == '/') *ptr = '_';

    g_fileWrite(result, "/* %s\n", headerFileName);
    g_fileWrite(result, " *\n");
    if (mainHeader) {
        g_fileWrite(result, " * This is the main package file. Include this file in other projects.\n");
        g_fileWrite(result, " * Only modify inside the header-end and body-end sections.\n");
    } else {
        g_fileWrite(result, " * This file contains generated package function and method declarations.\n");
        g_fileWrite(result, " * You should not manually modify the contents of this file.\n");
    }

    g_fileWrite(result, " */\n\n");
    g_fileWrite(result, "#ifndef %s_H\n", path);
    g_fileWrite(result, "#define %s_H\n\n", path);

    /* Include corto, if not generating corto */
    if (o && (o != corto_o)) {
        c_includeFrom(g, result, corto_o, "corto.h");
    }

    if (bootstrap) {
        g_fileWrite(result, "#include <%s/_project.h>\n", g_getProjectName(g));
    } else {
        c_include(g, result, corto_o);
        c_includeFrom(g, result, g_getCurrent(g), "_project.h");
    }

    if (mainHeader) {
        /* Add header files for dependent packages */
        if (g->imports) {
            corto_iter iter = corto_ll_iter(g->imports);
            while (corto_iter_hasNext(&iter)) {
                corto_object import = corto_iter_next(&iter);
                corto_string str = corto_path(NULL, NULL, import, "/");
                corto_string package = corto_locate(str, NULL, CORTO_LOCATION_FULLNAME);
                if (!package) {
                    corto_seterr("project configuration contains unresolved package '%s'", str);

                    /* Don't break out of generation here, as this will mess up the
                     * file's code snippet */
                    error = TRUE;
                } else {
                    corto_string name = corto_locate(str, NULL, CORTO_LOCATION_NAME);
                    g_fileWrite(result, "#include <%s/%s.h>\n", package, name);
                    corto_dealloc(name);
                    corto_dealloc(package);
                }
            }
        }
    }
    if (error) {
        goto error;
    }

    /* If a header exists, write it */
    if (mainHeader) {
        corto_string snippet;
        g_fileWrite(result, "\n");
        if ((snippet = g_fileLookupHeader(result, ""))) {
            g_fileWrite(result, "/* $header()");
            g_fileWrite(result, "%s", snippet);
            g_fileWrite(result, "$end */\n");
        } else {
            g_fileWrite(result, "/* $header() */\n");
            g_fileWrite(result, "/* Definitions that are required by package headers (native types) go here. */\n");
            g_fileWrite(result, "/* $end */\n");
        }
    }

    if (o) {
        g_fileWrite(result, "\n");
        c_includeFrom(g, result, g_getCurrent(g), "_type.h");
        c_includeFrom(g, result, g_getCurrent(g), "_interface.h");
        c_includeFrom(g, result, g_getCurrent(g), "_load.h");

        if (!bootstrap) {
            if (local || app) {
                c_includeFrom(g, result, g_getCurrent(g), "_api.h");   
            } else {
                c_includeFrom(g, result, g_getCurrent(g), "c/_api.h");
            }
        }
    }

    if (mainHeader) {
        corto_string snippet;

        g_fileWrite(result, "\n");
        if ((snippet = g_fileLookupSnippet(result, ""))) {
            g_fileWrite(result, "/* $body()");
            g_fileWrite(result, "%s", snippet);
            g_fileWrite(result, "$end */\n");
        } else {
            g_fileWrite(result, "/* $body() */\n");
            g_fileWrite(result, "/* Definitions here that need your package headers go here. */\n", snippet);
            g_fileWrite(result, "/* $end */\n");
        }
    }

    if (!mainHeader) {
        g_fileWrite(result, "\n");
        g_fileWrite(result, "#ifdef __cplusplus\n");
        g_fileWrite(result, "extern \"C\" {\n");
        g_fileWrite(result, "#endif\n");
    }

    return 0;
error:
    return -1;
}

/* Close headerfile */
static void c_interfaceHeaderFileClose(g_file file, corto_bool mainHeader, c_typeWalk_t *data) {

    CORTO_UNUSED(data);

    /* Print standard comments and includes */
    if (!mainHeader) {
        g_fileWrite(file, "\n");
        g_fileWrite(file, "#ifdef __cplusplus\n");
        g_fileWrite(file, "}\n");
        g_fileWrite(file, "#endif\n");
    }
    g_fileWrite(file, "\n");
    g_fileWrite(file, "#endif\n\n");
}

static g_file c_interfaceWrapperFileOpen(g_generator g) {
    g_file result;
    corto_char fileName[512];
    corto_bool cpp = !strcmp(g_getAttribute(g, "c4cpp"), "true");

    corto_object o = g_getCurrent(g);
    sprintf(fileName, "_wrapper.%s", cpp ? "cpp" : "c");

    if (!strcmp(g_getAttribute(g, "bootstrap"), "true")) {
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

    corto_buffer include = CORTO_BUFFER_INIT;
    c_include_toBuffer(g, &include, g_getCurrent(g));
    char *includeStr = corto_buffer_str(&include);
    g_fileWrite(result, includeStr);
    free(includeStr);

    c_includeFrom_toBuffer(g, &include, g_getCurrent(g), "_load.h");
    includeStr = corto_buffer_str(&include);
    g_fileWrite(result, includeStr);
    free(includeStr);

    return result;
error:
    return NULL;
}

/* Open generator sourcefile */
static cdiff_file c_interfaceSourceFileOpen(corto_object o, corto_string name, c_typeWalk_t *data) {
    cdiff_file result;
    corto_id fileName;
    corto_bool cpp = !strcmp(g_getAttribute(data->g, "c4cpp"), "true");

    if (o) {
        c_filename(data->g, fileName, o, cpp ? "cpp" : "c");
    } else {
        strcpy(fileName, name);
    }

    result = c_interfaceOpenFile(fileName, data);
    if (!result) {
        goto error;
    }

    /* Include main header */
    if (o) {
        corto_id header;
        cdiff_file_write(result, "#include <%s>\n", c_mainheader(data->g, header));
        cdiff_file_write(result, "\n");
    }

    return result;
error:
    return NULL;
}

static corto_int16 c_interfaceWriteMain(cdiff_file source, corto_string id, c_typeWalk_t* data) {
    CORTO_UNUSED(data);

    cdiff_file_write(source, "\n");

    cdiff_file_elemBegin(source, "%sMain", id);
    cdiff_file_headerBegin(source);
    cdiff_file_write(source, "int %sMain(int argc, char *argv[]) {", id);
    cdiff_file_headerEnd(source);

    if (!cdiff_file_bodyBegin(source)) {
        cdiff_file_write(source, "\n");
        cdiff_file_write(source, "\n");
        cdiff_file_indent(source);
        cdiff_file_write(source, "/* Insert implementation */\n");
        cdiff_file_write(source, "\n");
        cdiff_file_write(source, "return 0;\n");
        cdiff_file_dedent(source);
        cdiff_file_write(source, "}\n");
        cdiff_file_bodyEnd(source);
    }

    cdiff_file_elemEnd(source);

    return 0;
}

/* Generate interface for class */
static corto_int16 c_interfaceObject(corto_object o, c_typeWalk_t* data) {
    int hasProcedures;
    corto_bool isTopLevelObject;
    corto_bool isBootstrap = !strcmp(g_getAttribute(data->g, "bootstrap"), "true");

    hasProcedures = !corto_scopeWalk(o, c_interfaceCheckProcedures, NULL);
    isTopLevelObject = (o == g_getCurrent(data->g)) && corto_instanceof(corto_package_o, o);

    /* An interface implementation file is generated when the object is
     * an interface and has procedures. When the object is not an interface
     * but does have procedures (typical example is callbacks or static functions)
     * these are appended to the header of the first scope in the hierarchy. */
    if (hasProcedures || ((isTopLevelObject && !isBootstrap) && !strcmp(g_getProjectName(data->g), corto_idof(o)))) {

        /* Create a wrapper file if it was not already created */
        if (!data->wrapper) {
            data->wrapper = c_interfaceWrapperFileOpen(data->g);
            if (!data->wrapper) {
                goto error;
            }
        }

        /* Open sourcefile */
        data->source = c_interfaceSourceFileOpen(o, NULL, data);
        if (!data->source) {
            goto error;
        }

        /* Walk scope */
        corto_objectseq procs = corto_scopeClaim(o);
        corto_int32 i;
        for (i = 0; i < procs.length; i ++) {
            if (!c_interfaceClassProcedure(procs.buffer[i], data)) {
                break;
            }
        }
        corto_scopeRelease(procs);
        if (i != procs.length) {
            goto error;
        }

        if (isTopLevelObject && hasProcedures) {
            /* put newline between procedures and headers */
            g_fileWrite(data->interfaceHeader, "\n"); 
        }

        /* If top level file, generate main function */
        if (isTopLevelObject && !isBootstrap) {
            data->mainWritten = TRUE;
            if (c_interfaceWriteMain(data->source, g_getProjectName(data->g), data)) {
                goto error;
            }
        }

        cdiff_file_close(data->source);
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
    corto_objectseq scope = corto_scopeClaim(o);
    corto_int32 i;
    for (i = 0; i < scope.length; i++) {
        if (!c_interfaceWalk(scope.buffer[i], data)) {
            break;
        }
    }
    corto_scopeRelease(scope);
    if (i != scope.length) {
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

        if (strstr(content, GENERATED_MARKER)) {
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
    corto_iter iter = corto_ll_iter(data->generated);

    while(corto_iter_hasNext(&iter)) {
        corto_string file = corto_iter_next(&iter);
        if (!strcmp(file, name)) {
            return TRUE;
        }
    }

    return FALSE;
}

/* Mark files that haven't been regenerated */
static int c_interfaceMarkUnusedFiles(c_typeWalk_t *data) {
    corto_ll files = corto_opendir("./src");
    corto_iter iter = corto_ll_iter(files);

    while(corto_iter_hasNext(&iter)) {
        corto_string file = corto_iter_next(&iter);
        corto_id id;
        sprintf(id, "./src/%s", file);
        if (c_interfaceIsGenerated(id)) {
            if (!c_interfaceWasGeneratedNow(file, data)) {
                if (!strstr(id, ".old")) {
                    corto_id newname;
                    sprintf(newname, "src/%s.old", file);
                    corto_rename (id, newname);
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

    iter = corto_ll_iter(data->generated);
    while (corto_iter_hasNext(&iter)) {
        corto_dealloc(corto_iter_next(&iter));
    }

    corto_closedir(files);
    corto_ll_free(data->generated);

    return 0;
}

static corto_int16 c_interfaceWriteMainSource(c_typeWalk_t *data) {
    corto_id fileName, header;
    corto_bool cpp = !strcmp(g_getAttribute(data->g, "c4cpp"), "true");

    sprintf(fileName, "%s.%s", g_getProjectName(data->g), cpp ? "cpp" : "c");
    cdiff_file file = c_interfaceSourceFileOpen(NULL, fileName, data);
    if (!file) {
        goto error;
    }

    cdiff_file_write(file, "#include <%s>\n", c_mainheader(data->g, header));

    if (c_interfaceWriteMain(file, g_getProjectName(data->g), data)) {
        goto error;
    }

    return 0;
error:
    return -1;
}

/* Entry point for generator */
int corto_genMain(g_generator g) {
    c_typeWalk_t walkData;
    corto_bool bootstrap = !strcmp(g_getAttribute(g, "bootstrap"), "true");

    /* Create source and include directories */
    corto_mkdir("src");
    corto_mkdir("include");

    /* Default prefixes for corto namespaces */
    g_parse(g, corto_o, FALSE, FALSE, "");
    g_parse(g, corto_lang_o, FALSE, FALSE, "corto");
    g_parse(g, corto_core_o, FALSE, FALSE, "corto");
    g_parse(g, corto_native_o, FALSE, FALSE, "corto_native");
    g_parse(g, corto_secure_o, FALSE, FALSE, "corto_secure");

    /* Prepare walkData, create header- and sourcefile */
    walkData.g = g;
    walkData.source = NULL;
    walkData.wrapper = NULL;
    walkData.mainHeader = NULL;
    walkData.interfaceHeader = NULL;
    walkData.generated = corto_ll_new();
    walkData.mainWritten = FALSE;

    if (!bootstrap) {
        corto_mkdir(".corto");

        corto_object projectObject = NULL;
        corto_id headerFileName, projectName;

        /* If parsing an object and not a package, the mainheader should be
         * the name of the project, not the object */
        if (g_getCurrent(g)) {
            if (!corto_instanceof(corto_package_o, g_getCurrent(g))) {
                projectObject = g_getCurrent(g);
                sprintf(headerFileName, "%s.h", g_getProjectName(g));
                strcpy(projectName, g_getProjectName(g));
            } else {
                projectObject = g_getCurrent(g);
                sprintf(headerFileName, "%s.h", corto_idof(g_getCurrent(g)));
                corto_path(projectName, root_o, projectObject, "_");
            }
        } else {
            sprintf(headerFileName, "%s.h", g_getProjectName(g));
            strcpy(projectName, g_getName(g));
        }

        g_file mainHeader = g_fileOpen(g, headerFileName);
        if (!mainHeader) {
            corto_seterr("failed to open file '%s'", headerFileName);
            goto error;
        }

        walkData.mainHeader = mainHeader;

        /* Write header contents */
        if (c_interfaceHeaderWrite(
            g,
            mainHeader,
            projectObject,
            projectName,
            headerFileName,
            TRUE,
            &walkData))
        {
            goto error;
        }
    }

    if (g_getCurrent(g)) {
        walkData.interfaceHeader = g_fileOpen(g, "_interface.h");
        if (!walkData.interfaceHeader) {
            goto error;
        }

        corto_id path;
        corto_path(path, root_o, g_getCurrent(g), "_");
        strcat(path, "/interface");
        if (c_interfaceHeaderWrite(
            g,
            walkData.interfaceHeader,
            NULL,
            path,
            "_interface.h",
            FALSE,
            &walkData)) 
        {
            goto error;
        }
    }

    /* Walk objects, generate procedures and class members */
    if (!g_walkNoScope(g, c_interfaceWalk, &walkData)) {
        goto error;
    }

    if (walkData.mainHeader) {
        c_interfaceHeaderFileClose(walkData.mainHeader, TRUE, &walkData);
    }

    if (walkData.interfaceHeader) {
        c_interfaceHeaderFileClose(walkData.interfaceHeader, FALSE, &walkData);
    }

    /* If none of the scopes that was generated code for had the same name as
     * the project, or if this project does not have a package, generate a main
     * source and header file separately */
    if (!walkData.mainWritten && !bootstrap) {
        if (c_interfaceWriteMainSource(&walkData)) {
            goto error;
        }
    }

    c_interfaceMarkUnusedFiles(&walkData);

    return 0;
error:
    return -1;
}
