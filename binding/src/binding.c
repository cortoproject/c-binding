
#include <binding/binding.h>

typedef struct c_binding_t {
    g_generator g;
    g_file header;
    bool firstComma;
} c_binding_t;

static
bool c_interfaceParamRequiresCast(
    corto_type t,
    bool isReference,
    corto_inout inout)
{
    if ((isReference || t->reference) &&
        (t->kind != CORTO_VOID) && (t->kind != CORTO_ANY) &&
        (corto_check_attr(t, CORTO_ATTR_NAMED)) &&
        (!inout))
    {
        return TRUE;
    } else {
        return FALSE;
    }
}

static
int c_interfaceParamCastDef(
    corto_parameter *o,
    void *userData)
{
    c_binding_t* data = userData;

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

static
int c_interfaceParamCastWalk(
    corto_parameter *o,
    void *userData)
{
    c_binding_t* data = userData;
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
        if (c_interfaceParamRequiresCast(o->type, o->passByReference, o->inout)) {
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

static
int c_binding_argCastMacro(
    corto_function o,
    char *macroName,
    char *functionName,
    char *prefix,
    c_binding_t *data)
{
    data->firstComma = FALSE;

    if (prefix) {
        g_fileWrite(data->header, "#define %s%s(", prefix, macroName);
    } else {
        g_fileWrite(data->header, "#define %s(", macroName);
    }

    if (c_procedureHasThis(o)) {
        g_fileWrite(data->header, "_this");
        data->firstComma = TRUE;
    }

    if (!c_paramWalk(o, c_interfaceParamCastDef, data)) {
        goto error;
    }

    g_fileWrite(data->header, ") _%s(", functionName);

    if (c_procedureHasThis(o)) {
        corto_type thisType = corto_procedure(corto_typeof(o))->thisType;
        if (!thisType || thisType->reference) {
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

static
int c_binding_shortVariableTranslateWalk(
    corto_object o,
    void *userData)
{
    corto_id id, impl_id;
    c_binding_t* data = userData;

    c_id(data->g, id, o);
    c_short_id(data->g, impl_id, o);

    if (strcmp(id, impl_id)) {
        g_fileWrite(data->header, "#define %s_o %s_o\n", impl_id, id);
    }

    return 1;
}

static
int c_binding_shortIdentifierTranslateWalk(
    corto_object o,
    void *userData)
{
    if (corto_instanceof(corto_type_o, o) ||
        corto_instanceof(corto_package_o, o) ||
        corto_instanceof(corto_function_o, o) ||
        corto_instanceof(corto_constant_o, o))
    {
        corto_id id, impl_id;
        c_binding_t* data = userData;

        c_id(data->g, id, o);
        c_short_id(data->g, impl_id, o);

        if (strcmp(id, impl_id)) {
            g_fileWrite(data->header, "#define %s %s\n", impl_id, id);
            if (corto_instanceof(corto_function_o, o)) {
                g_fileWrite(data->header, "#define safe_%s safe_%s\n", impl_id, id);
            }

            if (corto_instanceof(corto_function_o, o) && corto_function(o)->overridable) {
                /* Remove _v */
                int len = strlen(impl_id);
                impl_id[len - 2] = '\0';
                len = strlen(id);
                id[len - 2] = '\0';
                g_fileWrite(data->header, "#define %s %s\n", impl_id, id);
                g_fileWrite(data->header, "#define safe_%s safe_%s\n", impl_id, id);
            }
        }
    }

    return 1;
}

static
int c_binding_localProcedureTranslateWalk(
    corto_object o,
    void *userData)
{
    corto_id id;
    c_binding_t* data = userData;
    c_id(data->g, id, o);
    if (corto_instanceof(corto_function_o, o))
    {
        g_fileWrite(data->header, "#define %s _%s\n", id, id);
    }
    return 1;
}

static
int c_binding_paramType(
    corto_parameter *o,
    void *userData)
{
    c_binding_t* data = userData;
    corto_id type;
    if (data->firstComma) {
        g_fileWrite(data->header, ", ");
    }
    g_fileWrite(data->header, "%s", c_paramType(data->g, o, type));
    data->firstComma++;
    return 1;
}

static
int c_binding_paramName(
    corto_parameter *o,
    void *userData)
{
    c_binding_t* data = userData;
    corto_id name;
    if (data->firstComma) {
        g_fileWrite(data->header, ", ");
    }
    g_fileWrite(data->header, "%s", c_paramName(o->name, name));
    data->firstComma++;
    return 1;
}

static
int c_binding_overridableMethodWalk(
    corto_object o,
    void* userData)
{
    c_binding_t *data = userData;
    corto_id id;

    g_fullOid(data->g, o, id);

    if (corto_instanceof(corto_overridable_o, o)) {
        corto_interface interface = corto_parentof(o);

        /* If this is an interface method */
        if (interface->kind != CORTO_INTERFACE) {
            corto_id returnTypeId, typeVarId, methodVarId, typeId;
            bool returnsValue;

            g_fullOid(data->g, corto_parentof(o), typeId);
            c_varId(data->g, o, methodVarId);
            c_varId(data->g, corto_parentof(o), typeVarId);

            if (((corto_function)o)->returnType &&
                ((corto_function(o)->returnType->kind != CORTO_VOID) ||
                 corto_function(o)->returnType->reference))
            {
                returnsValue = TRUE;
                c_typeret(data->g, corto_function(o)->returnType, C_ByValue, false, returnTypeId);
            } else {
                returnsValue = FALSE;
                strcpy(returnTypeId, "void");
            }

            g_fileWrite(data->header, "#define %s(_this", id);
            data->firstComma = 1;
            if (!c_paramWalk(o, c_binding_paramName, data)) {
                goto error;
            }

            corto_id returnTypeCastId;
            c_typeret(data->g, corto_function(o)->returnType, C_Cast, false, returnTypeCastId);
            g_fileWrite(data->header, ") ( \\\n");
            g_fileWrite(data->header, "    ((corto_function)%s)->kind == CORTO_PROCEDURE_CDECL \\\n", methodVarId);
            g_fileWrite(data->header, "    ? ((%s (*)(corto_object", returnTypeCastId);
            if (!c_paramWalk(o, c_binding_paramType, data)) {
                goto error;
            }
            g_fileWrite(
                data->header,
                "))((corto_function)((corto_interface)corto_typeof(_this))->methods.buffer[((corto_method)%s)->index - 1])->fptr)(%s(_this)",
                methodVarId,
                typeId);
            if (!c_paramWalk(o, c_interfaceParamCastWalk, data)) {
                goto error;
            }
            g_fileWrite(data->header, ") \\\n");
            if (returnsValue) {
                g_fileWrite(
                    data->header,
                    "    : *(%s*)corto_invoke(((corto_interface)corto_typeof(_this))->methods.buffer[((corto_method)%s)->index - 1], alloca(sizeof(%s)), %s(_this)",
                    returnTypeId,
                    methodVarId,
                    returnTypeId,
                    typeId);
            } else {
                g_fileWrite(
                    data->header,
                    "    : (void)corto_invoke(((corto_interface)corto_typeof(_this))->methods.buffer[((corto_method)%s)->index - 1], NULL, %s(_this)",
                    methodVarId,
                    typeId);
            }
            data->firstComma = 3;
            if (!c_paramWalk(o, c_interfaceParamCastWalk, data)) {
                goto error;
            }
            g_fileWrite(data->header, ") \\\n");
            g_fileWrite(data->header, "    )\n");
        }
    }

    return 1;
error:
    return 0;
}

static
int c_binding_localArgCastMacroWalk(
    corto_object o,
    void* userData)
{
    c_binding_t *data = userData;

    if (corto_class_instanceof(corto_procedure_o, corto_typeof(o))) {
        corto_id functionName;
        c_functionName(data->g, o, functionName);
        c_binding_argCastMacro(o, functionName, functionName, "safe_", data);
    }

    return 1;
}

static
int c_binding_argCastMacroWalk(
    corto_object o,
    void* userData)
{
    c_binding_t *data = userData;

    if (corto_instanceof(corto_function_o, o)) {
        corto_id functionName;
        c_functionName(data->g, o, functionName);
        c_binding_argCastMacro(o, functionName, functionName, NULL, data);
    }

    return 1;
}

static
int c_binding_interfaceArgCastMacroWalk(
    corto_object o,
    void* userData)
{
    c_binding_t *data = userData;

    if (corto_instanceof(corto_method_o, o)) {
        corto_id functionName;
        if (corto_interface(corto_parentof(o))->kind == CORTO_INTERFACE) {
            g_fullOid(data->g, o, functionName); /* Don't append _v */
            c_binding_argCastMacro(o, functionName, functionName, NULL, data);
        }
    }

    return 1;
}

/* Print typedefs */
static
int c_binding_nonExpandingTypedef(
    corto_object o,
    void* userData)
{
    c_binding_t* data;

    data = userData;

    if (corto_class_instanceof(corto_type_o, o) &&
        !corto_instanceof(corto_native_type_o, o))
    {
        corto_id id;
        c_typeId(data->g, o, id);
        bool named = corto_check_attr(o, CORTO_ATTR_NAMED);
        if (!named) {
            g_fileWrite(data->header, "#ifndef _type_%s_DEFINED\n", id);
            g_fileWrite(data->header, "#define _type_%s_DEFINED\n", id);
        }
        g_fileWrite(data->header, "typedef %s _type_%s;\n", id, id);
        if (!named) {
            g_fileWrite(data->header, "#endif\n");
        }
    }

    return 0;
}

static
int c_binding_typeCastMacro(
    corto_object o,
    void* userData)
{
    c_binding_t* data;
    corto_id id, varId, ptr;

    data = userData;

    if (corto_class_instanceof(corto_type_o, o) &&
        !corto_instanceof(corto_native_type_o, o))
    {
        c_typeId(data->g, o, id);
        c_typeret(data->g, o, C_ByReference, false, ptr);
        c_varId(data->g, o, varId);

        if (corto_type(o)->kind != CORTO_VOID) {
            g_fileWrite(data->header,
                "#define %s(o) ((%s)corto_assert_type((corto_type)%s, o))\n",
                id, ptr, varId);
        } else {
            g_fileWrite(data->header,
                "#define %s(o) ((%s)o)\n",
                id, ptr);
        }
    }

    return 0;
}

int genmain(g_generator g) {
    c_binding_t walkdata;
    corto_id building_macro;

    walkdata.g = g;
    walkdata.header = c_headerOpen(g, "binding");
    walkdata.firstComma = false;

    c_buildingMacro(g, building_macro);

    /* #1 Type cast macro's */
    g_fileWrite(walkdata.header, "/* -- Type casting -- */\n");
    if (corto_genTypeDepWalk(g, NULL, c_binding_typeCastMacro, NULL, &walkdata)) {
        goto error;
    }

    /* #2 Typedefs that don't expand into runtime type checks */
    g_fileWrite(walkdata.header, "\n/* -- Non-expanding typedefs -- */\n");
    if (corto_genTypeDepWalk(g, NULL, c_binding_nonExpandingTypedef, NULL, &walkdata)) {
        goto error;
    }

    /* #3 Macro's that do runtime type checking of procedure arguments */
    g_fileWrite(walkdata.header, "\n/* -- Argument type casting -- */\n");
    g_fileWrite(walkdata.header, "#ifndef %s\n", building_macro);
    if (!g_walkAll(g, c_binding_argCastMacroWalk, &walkdata)) {
        goto error;
    }
    g_fileWrite(walkdata.header, "#else\n");
    g_fileWrite(walkdata.header,
        "/* Can't use argument type casting macro's within project, as they would\n");
    g_fileWrite(walkdata.header,
         " * substitute headers in implementation files. */\n");
    /* When building the package, the casting macro's would try to substitute
     * the function implementations. Therefore in this case, the macro will just
     * point to the function directly. */
    if (!g_walkAll(g, c_binding_localProcedureTranslateWalk, &walkdata)) {
        goto error;
    }

    /* safe_ macro for macro's that do argument checking within the package */
    g_fileWrite(walkdata.header, "/* Macro for doing argument type casting within project. */\n");
    if (!g_walkAll(g, c_binding_localArgCastMacroWalk, &walkdata)) {
        goto error;
    }
    g_fileWrite(walkdata.header, "#endif\n");
    if (!g_walkAll(g, c_binding_interfaceArgCastMacroWalk, &walkdata)) {
        goto error;
    }

    /* #4 Overridable methods */
    g_fileWrite(walkdata.header, "\n/* -- Overridable methods -- */\n");
    if (!g_walkAll(g, c_binding_overridableMethodWalk, &walkdata)) {
        goto error;
    }

    /* #6 Macro's that translate between short and full identifiers */
    g_fileWrite(walkdata.header, "\n/* -- Short identifier translation -- */\n");
    if (!g_walkAll(g, c_binding_shortIdentifierTranslateWalk, &walkdata)) {
        goto error;
    }

    /* #7 Macro's that translate between short and full variables */
    g_fileWrite(walkdata.header, "\n/* -- Short object variable translation -- */\n");
    if (!g_walkAll(g, c_binding_shortVariableTranslateWalk, &walkdata)) {
        goto error;
    }

    g_fileWrite(walkdata.header, "\n");
    c_headerClose(walkdata.header);

    return 0;
error:
    return -1;
}
