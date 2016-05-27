
#include "api.h"
#include "corto/gen/c/common/common.h"
#include "ctype.h"

typedef struct c_arg {
    corto_string name;
    corto_type type;
    corto_bool move;
    corto_string nativeCast;
} c_arg;

static void c_apiCastMacroAddArg(corto_ll args, corto_string name, corto_type type) {
    if (args) {
        c_arg *arg = corto_alloc(sizeof(c_arg));
        arg->name = corto_strdup(name);
        arg->type = type;
        arg->move = FALSE;
        arg->nativeCast = NULL;
        corto_llAppend(args, arg);
    }
}

static void c_apiCastMacroAddArgNativeCast(corto_ll args, corto_string name, corto_string type) {
    if (args) {
        c_arg *arg = corto_alloc(sizeof(c_arg));
        arg->name = corto_strdup(name);
        arg->type = NULL;
        arg->move = FALSE;
        arg->nativeCast = corto_strdup(type);
        corto_llAppend(args, arg);
    }
}

static void c_apiCastMacroAddArgMove(corto_ll args, corto_string name, corto_type type) {
    if (args) {
        c_arg *arg = corto_alloc(sizeof(c_arg));
        arg->name = corto_strdup(name);
        arg->type = type;
        arg->move = TRUE;
        arg->nativeCast = NULL;
        corto_llAppend(args, arg);
    }
}

static void c_apiCastMacroFreeArgs(corto_ll args) {
    corto_iter iter = corto_llIter(args);
    while (corto_iterHasNext(&iter)) {
        c_arg *arg = corto_iterNext(&iter);
        corto_dealloc(arg->name);
        corto_dealloc(arg);
    }
    corto_llFree(args);
}

static corto_int16 c_apiCastAuto(corto_type t, corto_bool scoped, corto_string func, c_apiWalk_t *data) {
    corto_uint32 count = 0;
    corto_id ret, id;
    g_fullOid(data->g, t, id);

    g_fileWrite(
        data->header,
        "#define %s%s_auto(",
        id,
        func);

    if (!scoped) {
        g_fileWrite(data->header, "_name");
        count = 1;
    }

    corto_iter iter = corto_llIter(data->args);
    while (corto_iterHasNext(&iter)) {
        c_arg *arg = corto_iterNext(&iter);
        if (count) {
            g_fileWrite(data->header, ", ");
        }
        g_fileWrite(data->header, "%s", arg->name);
        count++;
    }

    g_fileWrite(
        data->header,
        ") %s _name = %s%s(%s",
        c_typeret(data->g, t, ret),
        id,
        func,
        scoped ? "_parent, #_name" : "");

    iter = corto_llIter(data->args);
    count = 0;

    if (scoped) {
        corto_iterHasNext(&iter) ? corto_iterNext(&iter) : 0; /* _parent */
        corto_iterHasNext(&iter) ? corto_iterNext(&iter) : 0; /* _name */
        count = 2;
    }

    while (corto_iterHasNext(&iter)) {
        c_arg *arg = corto_iterNext(&iter);
        if (count) {
            g_fileWrite(data->header, ", ");
        }
        g_fileWrite(data->header, "%s", arg->name);
        count++;
    }

    g_fileWrite(data->header, "); (void)_name\n");

    return 0;
}

static corto_int16 c_apiCastMacro(corto_string id, corto_string name, c_apiWalk_t *data) {
    corto_uint32 count = 0;
    corto_bool cpp = !strcmp(gen_getAttribute(data->g, "c4cpp"), "true");

    g_fileWrite(data->header, "#define %s%s(", id, name);
    corto_iter iter = corto_llIter(data->args);
    while (corto_iterHasNext(&iter)) {
        c_arg *arg = corto_iterNext(&iter);
        if (count) {
            g_fileWrite(data->header, ", ");
        }
        g_fileWrite(data->header, "%s", arg->name);
        count++;
    }

    g_fileWrite(data->header, ") _%s%s(", id, name);
    iter = corto_llIter(data->args);
    count = 0;
    while (corto_iterHasNext(&iter)) {
        c_arg *arg = corto_iterNext(&iter);
        if (count) {
            g_fileWrite(data->header, ", ");
        }
        if (arg->move && cpp) {
            g_fileWrite(data->header, "std::move(");
        }
        if (arg->type && arg->type->reference && (arg->type->kind != CORTO_VOID)) {
            corto_id id;
            g_fullOid(data->g, arg->type, id);
            g_fileWrite(data->header, "%s(%s)", id, arg->name);
        } else {
            if (arg->nativeCast) {
                g_fileWrite(data->header, "(%s)", arg->nativeCast);
            }
            g_fileWrite(data->header, "%s", arg->name);
        }
        if (arg->move && cpp) {
            g_fileWrite(data->header, ")");
        }
        count++;
    }

    g_fileWrite(data->header, ")\n");

    return 0;
}

/* Assign primitive value */
static corto_int16 c_apiAssign (
    corto_type t,
    corto_bool ptr,
    corto_string lvalue,
    corto_string rvalue,
    c_apiWalk_t *data)
{
    if (t->reference) {
        g_fileWrite(data->source, "corto_setref(%s%s, %s);\n",
                ptr ? "" : "&", lvalue, rvalue);
    } else if (t->kind == CORTO_PRIMITIVE) {
        if (corto_primitive(t)->kind == CORTO_TEXT) {
            g_fileWrite(data->source, "corto_setstr(%s%s, %s);\n",
                ptr ? "" : "&", lvalue, rvalue);
        } else {
            g_fileWrite(data->source, "%s%s = %s;\n",
                (!c_typeRequiresPtr(t) && ptr) ? "*" : "",
                lvalue, rvalue);
        }
    } else {
        corto_id id, postfix;
        corto_bool noAmpersand =
            c_typeRequiresPtr(t) ||
            ((t->kind == CORTO_COLLECTION) &&
              (corto_collection(t)->kind == CORTO_ARRAY)
            );

        c_specifierId(data->g, t, id, NULL, postfix);
        g_fileWrite(data->source, "corto_copyp(&%s, %s_o, %s%s);\n",
            lvalue, id,
            noAmpersand ? "" : "&",
            rvalue);
    }

    return 0;
}

/* Translate members to function parameters. */
static corto_int16 c_apiAssignMember(corto_serializer s, corto_value* v, void* userData) {
    c_apiWalk_t *data;
    corto_member m;
    corto_id memberIdTmp, memberParamId, memberId;

    CORTO_UNUSED(s);

    if (v->kind == CORTO_MEMBER) {
        m = v->is.member.t;
        data = userData;
        corto_id ptr, lvalue, type;

        corto_genMemberName(data->g, data->memberCache, m, memberIdTmp);
        g_id(data->g, memberIdTmp, memberParamId);
        g_id(data->g, corto_idof(m), memberId);
        g_fullOid(data->g, corto_parentof(m), type);

        sprintf(lvalue,
            "((%s)_this)->%s",
            c_typeptr(data->g, corto_parentof(m), ptr),
            memberId);

        c_apiAssign(m->type, FALSE, lvalue, memberParamId, data);
        data->parameterCount++;
    }

    return 0;
}

/* Translate members to function parameters. */
static corto_int16 c_apiParamMember(corto_serializer s, corto_value* v, void* userData) {
    c_apiWalk_t* data;
    corto_member m;
    corto_id typeSpec, typePostfix, memberIdTmp, memberId;

    CORTO_UNUSED(s);

    if (v->kind == CORTO_MEMBER) {
        m = v->is.member.t;
        data = userData;

        if (data->parameterCount) {
            g_fileWrite(data->header, ", ");
            g_fileWrite(data->source, ", ");
        }

        /* Get type-specifier */
        c_specifierId(data->g, m->type, typeSpec, NULL, typePostfix);
        corto_genMemberName(data->g, data->memberCache, m, memberIdTmp);

        if (data->args) {
            c_apiCastMacroAddArg(data->args, g_id(data->g, memberIdTmp, memberId), m->type);
        }

        g_fileWrite(data->header, "%s%s%s %s",
               typeSpec,
               typePostfix,
               c_typeRequiresPtr(m->type) ? "*" : "",
               g_id(data->g, memberIdTmp, memberId));

        g_fileWrite(data->source, "%s%s%s %s",
                typeSpec,
                typePostfix,
                c_typeRequiresPtr(m->type) ? "*" : "",
                g_id(data->g, memberIdTmp, memberId));

        data->parameterCount++;
    }

    return 0;
}

/* Member parameter serializer */
static struct corto_serializer_s c_apiParamSerializer(void) {
    struct corto_serializer_s s;

    corto_serializerInit(&s);
    s.metaprogram[CORTO_MEMBER] = c_apiParamMember;
    s.access = CORTO_LOCAL|CORTO_READONLY|CORTO_PRIVATE|CORTO_HIDDEN;
    s.accessKind = CORTO_NOT;

    return s;
}

/* Member parameter serializer */
static struct corto_serializer_s c_apiAssignSerializer(void) {
    struct corto_serializer_s s;

    corto_serializerInit(&s);
    s.metaprogram[CORTO_MEMBER] = c_apiAssignMember;
    s.access = CORTO_LOCAL|CORTO_READONLY|CORTO_PRIVATE|CORTO_HIDDEN;
    s.accessKind = CORTO_NOT;

    return s;
}

corto_int16 c_apiTypeInitAny(corto_type t, c_apiWalk_t *data) {
    CORTO_UNUSED(t);

    if (data->parameterCount) {
        g_fileWrite(data->header, ", ");
        g_fileWrite(data->source, ", ");
    } else {
        data->parameterCount += 2;
    }
    g_fileWrite(data->header, "corto_type type, void *value");
    g_fileWrite(data->source, "corto_type type, void *value");
    c_apiCastMacroAddArg(data->args, "type", corto_type(corto_type_o));
    c_apiCastMacroAddArg(data->args, "value", NULL);
    return 0;
}

corto_int16 c_apiTypeInitPrimitive(corto_type t, c_apiWalk_t *data) {
    corto_id id, postfix;
    c_specifierId(data->g, t, id, NULL, postfix);

    if (data->parameterCount) {
        g_fileWrite(data->header, ", ");
        g_fileWrite(data->source, ", ");
    } else {
        data->parameterCount ++;
    }

    g_fileWrite(data->header, "%s value", id);
    g_fileWrite(data->source, "%s value", id);
    c_apiCastMacroAddArg(data->args, "value", NULL);
    return 0;
}

corto_int16 c_apiTypeInitComposite(corto_type t, c_apiWalk_t *data) {
    struct corto_serializer_s s = c_apiParamSerializer();
    corto_metaWalk(&s, corto_type(t), data);

    if (corto_instanceof(corto_procedure_o, t)) {
        if (data->parameterCount) {
            g_fileWrite(data->source, ", ");
            g_fileWrite(data->header, ", ");
        }

        g_fileWrite(data->header, "void(*_impl)(void)");
        g_fileWrite(data->source, "void(*_impl)(void)");
        c_apiCastMacroAddArgNativeCast(data->args, "_impl", "void(*)(void)");
        data->parameterCount++;
    }

    return 0;
}

corto_int16 c_apiTypeInitCollection(corto_type t, c_apiWalk_t *data) {
    corto_id elementId;

    if (data->parameterCount) {
        g_fileWrite(data->header, ", ");
        g_fileWrite(data->source, ", ");
    } else {
        data->parameterCount += 2;
    }

    g_fullOid(data->g, corto_collection(t)->elementType, elementId);
    g_fileWrite(data->header, "corto_uint32 length, %s* elements", elementId);
    g_fileWrite(data->source, "corto_uint32 length, %s* elements", elementId);
    c_apiCastMacroAddArg(data->args, "length", NULL);
    c_apiCastMacroAddArgMove(data->args, "elements", NULL);

    return 0;
}

/* Add initializer arguments to function declaration for type */
corto_int16 c_apiTypeInitArgs(corto_type t, c_apiWalk_t *data) {
    corto_int16 result = 0;

    switch(t->kind) {
    case CORTO_VOID:
    case CORTO_ITERATOR:
        break;
    case CORTO_ANY:
        result = c_apiTypeInitAny(t, data);
        break;
    case CORTO_PRIMITIVE:
        result = c_apiTypeInitPrimitive(t, data);
        break;
    case CORTO_COMPOSITE:
        result = c_apiTypeInitComposite(t, data);
        break;
    case CORTO_COLLECTION:
        result = c_apiTypeInitCollection(t, data);
        break;
    }

    return result;
}

/* Assign any */
corto_int16 c_apiTypeAssignAny(corto_type t, c_apiWalk_t *data) {
    CORTO_UNUSED(t);
    g_fileWrite(data->source, "corto_any v;\n");
    g_fileWrite(data->source, "v.value = value;\n");
    g_fileWrite(data->source, "v.type = type;\n");
    g_fileWrite(data->source, "_this->owner = TRUE;\n");
    g_fileWrite(data->source, "corto_copyp(_this, corto_any_o, &v);\n");
    return 0;
}

/* Assign primitve */
corto_int16 c_apiTypeAssignPrimitive(corto_type t, c_apiWalk_t *data) {
    c_apiAssign(t, TRUE, "_this", "value", data);
    return 0;
}

/* Assign composite */
corto_int16 c_apiTypeAssignComposite(corto_type t, c_apiWalk_t *data) {
    struct corto_serializer_s s = c_apiAssignSerializer();
    corto_id id;

    g_fullOid(data->g, t, id);

    corto_metaWalk(&s, corto_type(t), data);

    if (corto_instanceof(corto_procedure_o, t)) {
        g_fileWrite(data->source, "corto_function(_this)->kind = CORTO_PROCEDURE_CDECL;\n");
        g_fileWrite(data->source, "corto_function(_this)->fptr = (corto_word)_impl;\n");
    }

    return 0;
}

/* Assign collection */
corto_int16 c_apiTypeAssignCollection(corto_type t, c_apiWalk_t *data) {
    corto_id id, cVar, lvalue;
    corto_type elementType = corto_collection(t)->elementType;

    g_fullOid(data->g, t, id);

    strcpy(cVar, "_this");

    switch(corto_collection(t)->kind) {
    case CORTO_SEQUENCE:
        strcpy(cVar, "_this->buffer");
        g_fileWrite(data->source, "%sSize(_this, length);\n", id);
    case CORTO_ARRAY:
        g_fileWrite(data->source, "corto_uint32 i = 0;\n");
        g_fileWrite(data->source, "for (i = 0; i < length; i ++) {\n");
        g_fileIndent(data->source);
        sprintf(lvalue, "%s[i]", cVar);
        if (elementType->reference || (elementType->kind == CORTO_PRIMITIVE)) {
            c_apiAssign(
              corto_collection(t)->elementType, FALSE, lvalue, "elements[i]", data);
        } else {
            c_apiAssign(
              corto_collection(t)->elementType, FALSE, lvalue, "&elements[i]", data);
        }
        g_fileDedent(data->source);
        g_fileWrite(data->source, "}\n");
        break;
    case CORTO_LIST:
        g_fileWrite(data->source, "corto_uint32 i = 0;\n");
        g_fileWrite(data->source, "%sClear(*%s);\n", id, cVar);
        g_fileWrite(data->source, "for (i = 0; i < length; i ++) {\n");
        g_fileIndent(data->source);
        if (elementType->reference || (elementType->kind == CORTO_PRIMITIVE)) {
            g_fileWrite(data->source, "%sAppend(*%s, elements[i]);\n", id, cVar);
        } else {
            g_fileWrite(data->source, "%sAppend(*%s, &elements[i]);\n", id, cVar);
        }
        g_fileDedent(data->source);
        g_fileWrite(data->source, "}\n");
        break;
    case CORTO_MAP:
        break;
    }

    return 0;
}

/* Add initializer arguments to function declaration for type */
corto_int16 c_apiTypeInitAssign(corto_type t, c_apiWalk_t *data) {
    corto_int16 result = 0;

    switch(t->kind) {
    case CORTO_VOID:
    case CORTO_ITERATOR:
        break;
    case CORTO_ANY:
        result = c_apiTypeAssignAny(t, data);
        break;
    case CORTO_PRIMITIVE:
        result = c_apiTypeAssignPrimitive(t, data);
        break;
    case CORTO_COMPOSITE:
        result = c_apiTypeAssignComposite(t, data);
        break;
    case CORTO_COLLECTION:
        result = c_apiTypeAssignCollection(t, data);
        break;
    }

    return result;
}

corto_int16 c_apiTypeCreateIntern(corto_type t, c_apiWalk_t *data, corto_string func, corto_bool scoped, corto_bool define) {
    corto_id id, ret;
    corto_generator g = data->g;

    g_fullOid(data->g, t, id);
    c_typeret(g, t, ret);

    /* Collect arguments for casting macro */
    data->args = corto_llNew();
    data->parameterCount = 0;

    /* Function declaration */
    c_writeExport(data->g, data->header);
    g_fileWrite(data->header, " %s _%s%s(", ret, id, func);

    /* Function implementation */
    g_fileWrite(data->source, "%s _%s%s(", ret, id, func);

    if (scoped) {
        g_fileWrite(data->header, "corto_object _parent, corto_string _name");
        g_fileWrite(data->source, "corto_object _parent, corto_string _name");
        c_apiCastMacroAddArg(data->args, "_parent", corto_object_o);
        c_apiCastMacroAddArg(data->args, "_name", corto_type(corto_string_o));
        data->parameterCount = 2;
    }

    if (define) {
        c_apiTypeInitArgs(t, data);
    }

    if (!data->parameterCount) {
        g_fileWrite(data->header, "void");
        g_fileWrite(data->source, "void");
    }

    g_fileWrite(data->header, ");\n");
    g_fileWrite(data->source, ") {\n");

    /* Write casting macro */
    c_apiCastMacro(id, func, data);
    c_apiCastAuto(t, scoped, func, data);
    c_apiCastMacroFreeArgs(data->args);
    data->args = NULL;

    g_fileIndent(data->source);
    g_fileWrite(data->source, "%s _this;\n", ret);
    if (scoped) {
        g_fileWrite(
          data->source,
          "_this = %s(corto_declareChild(_parent, _name, %s_o));\n", id, id);
    } else {
        g_fileWrite(
          data->source,
          "_this = %s(corto_declare(%s_o));\n", id, id);
    }

    g_fileWrite(data->source, "if (!_this) {\n");
    g_fileIndent(data->source);
    g_fileWrite(data->source, "return NULL;\n");
    g_fileDedent(data->source);
    g_fileWrite(data->source, "}\n");

    if (define) {
        /* Assignments */
        c_apiTypeInitAssign(t, data);

        if (t->kind != CORTO_VOID) {
            /* Define object */
            g_fileWrite(data->source, "if (corto_define(_this)) {\n");
            g_fileIndent(data->source);
            g_fileWrite(data->source, "corto_release(_this);\n");
            g_fileWrite(data->source, "_this = NULL;\n");
            g_fileDedent(data->source);
            g_fileWrite(data->source, "}\n");
        }
    }

    g_fileWrite(data->source, "return _this;\n");
    g_fileDedent(data->source);
    g_fileWrite(data->source, "}\n\n");

    return 0;
}

corto_int16 c_apiTypeCreate(corto_type t, c_apiWalk_t *data) {
    return c_apiTypeCreateIntern(t, data, "Create", FALSE, TRUE);
}

corto_int16 c_apiTypeCreateChild(corto_type t, c_apiWalk_t *data) {
    return c_apiTypeCreateIntern(t, data, "CreateChild", TRUE, TRUE);
}

corto_int16 c_apiTypeDeclare(corto_type t, c_apiWalk_t *data) {
    return c_apiTypeCreateIntern(t, data, "Declare", FALSE, FALSE);
}

corto_int16 c_apiTypeDeclareChild(corto_type t, c_apiWalk_t *data) {
    return c_apiTypeCreateIntern(t, data, "DeclareChild", TRUE, FALSE);
}

corto_int16 c_apiTypeDefineIntern(corto_type t, c_apiWalk_t *data, corto_bool isUpdate, corto_bool doUpdate) {
    corto_id id, ptr;
    corto_string func = isUpdate ? doUpdate ? "Update" : "Set" : "Define";
    corto_generator g = data->g;

    data->args = corto_llNew();
    data->parameterCount = 1;
    g_fullOid(data->g, t, id);

    c_writeExport(data->g, data->header);

    if (isUpdate && !doUpdate) {
      g_fileWrite(data->header, " void ");
      g_fileWrite(data->source, "void ");
    } else {
      g_fileWrite(data->header, " corto_int16 ");
      g_fileWrite(data->source, "corto_int16 ");
    }

    g_fileWrite(data->header, "_%s%s(%s _this", id, func, c_typeptr(g, t, ptr));
    g_fileWrite(data->source, "_%s%s(%s _this", id, func, c_typeptr(g, t, ptr));
    c_apiCastMacroAddArg(data->args, "_this", t);

    c_apiTypeInitArgs(t, data);

    /* Write closing brackets for argumentlist in source and header */
    g_fileWrite(data->header, ");\n");
    g_fileWrite(data->source, ") {\n");
    g_fileIndent(data->source);
    g_fileWrite(data->source, "CORTO_UNUSED(_this);\n");

    /* Write cast macro */
    c_apiCastMacro(id, func, data);
    c_apiCastMacroFreeArgs(data->args);
    data->args = NULL;

    if (isUpdate && doUpdate && (t->kind != CORTO_VOID)) {
      g_fileWrite(data->source, "if (!corto_updateBegin(_this)) {\n");
        g_fileIndent(data->source);
    }

    /* Member assignments */
    c_apiTypeInitAssign(t, data);

    /* Define object */
    if (isUpdate && doUpdate) {
        if (t->kind != CORTO_VOID) {
            g_fileWrite(data->source, "corto_updateEnd(_this);\n");
            g_fileDedent(data->source);
            g_fileWrite(data->source, "} else {\n");
            g_fileIndent(data->source);
            g_fileWrite(data->source, "return -1;\n");
            g_fileDedent(data->source);
            g_fileWrite(data->source, "}\n");
            g_fileWrite(data->source, "return 0;\n");
        } else {
            g_fileWrite(data->source, "return corto_update(_this);\n");
        }
    } else if (!isUpdate) {
        g_fileWrite(data->source, "return corto_define(_this);\n");
    }

    g_fileDedent(data->source);
    g_fileWrite(data->source, "}\n\n");

    return 0;
}

corto_int16 c_apiTypeDefine(corto_type t, c_apiWalk_t *data) {
  return c_apiTypeDefineIntern(t, data, FALSE, FALSE);
}

corto_int16 c_apiTypeUpdate(corto_type t, c_apiWalk_t *data) {
  return c_apiTypeDefineIntern(t, data, TRUE, TRUE);
}

corto_int16 c_apiTypeSet(corto_type t, c_apiWalk_t *data) {
    return c_apiTypeDefineIntern(t, data, TRUE, FALSE);
}

corto_int16 c_apiTypeStr(corto_type t, c_apiWalk_t *data) {
    corto_id id;
    corto_bool ptr = c_typeRequiresPtr(t);

    data->args = corto_llNew();
    g_fullOid(data->g, t, id);

    /* Function declaration */
    c_writeExport(data->g, data->header);
    g_fileWrite(data->header, " corto_string _%sStr(%s%s value);\n", id, id, ptr ? "*" : "");

    /* Function implementation */
    g_fileWrite(data->source, "corto_string _%sStr(%s%s value) {\n", id, id, ptr ? "*" : "");

    c_apiCastMacroAddArg(data->args, "value", t);
    c_apiCastMacro(id, "Str", data);
    c_apiCastMacroFreeArgs(data->args);
    data->args = NULL;

    g_fileIndent(data->source);
    g_fileWrite(data->source, "corto_string result;\n", id);

    g_fileWrite(data->source, "corto_value v;\n", id);
    if (t->reference) {
        g_fileWrite(data->source, "v = corto_value_object(value, corto_type(%s_o));\n", id);
    } else {
        corto_bool isPtr =
          ptr ||
          ((t->kind == CORTO_COLLECTION) &&
          (corto_collection(t)->kind == CORTO_ARRAY));

        g_fileWrite(
            data->source,
            "v = corto_value_value(corto_type(%s_o), %svalue);\n",
            id,
            isPtr ? "" : "&");
    }
    g_fileWrite(data->source, "result = corto_strv(&v, 0);\n");

    g_fileWrite(data->source, "return result;\n");
    g_fileDedent(data->source);
    g_fileWrite(data->source, "}\n\n");

    return 0;
}

static corto_int16 c_apiTypeInitIntern(corto_type t, c_apiWalk_t *data, corto_string func) {
    corto_id id, funcLower;

    data->args = corto_llNew();
    g_fullOid(data->g, t, id);

    /* Function declaration */
    c_writeExport(data->g, data->header);
    g_fileWrite(data->header, " corto_int16 _%s%s(%s%s value);\n", id, func, id, t->reference ? "" : "*");

    /* Function implementation */
    g_fileWrite(data->source, "corto_int16 _%s%s(%s%s value) {\n", id, func, id, t->reference ? "" : "*");

    c_apiCastMacroAddArg(data->args, "value", t);
    c_apiCastMacro(id, func, data);
    c_apiCastMacroFreeArgs(data->args);
    data->args = NULL;

    g_fileIndent(data->source);
    g_fileWrite(data->source, "corto_int16 result;\n", id);

    if (!strcmp(func, "Init")) {
        g_fileWrite(data->source, "memset(value, 0, corto_type(%s_o)->size);\n", id);
    }

    strcpy(funcLower, func);
    *funcLower = tolower(*funcLower);

    if (t->reference) {
        g_fileWrite(data->source, "result = corto_%s(value, 0);\n", funcLower);
    } else {
        g_fileWrite(data->source, "corto_value v;\n", id);
        g_fileWrite(data->source, "v = corto_value_value(corto_type(%s_o), value);\n", id);
        g_fileWrite(data->source, "result = corto_%sv(&v);\n", funcLower);
    }

    g_fileWrite(data->source, "return result;\n");
    g_fileDedent(data->source);
    g_fileWrite(data->source, "}\n\n");

    return 0;
}

corto_int16 c_apiTypeInit(corto_type t, c_apiWalk_t *data) {
  return c_apiTypeInitIntern(t, data, "Init");
}

corto_int16 c_apiTypeDeinit(corto_type t, c_apiWalk_t *data) {
  return c_apiTypeInitIntern(t, data, "Deinit");
}

corto_int16 c_apiTypeFromStr(corto_type t, c_apiWalk_t *data) {
    corto_id id, ptr, ret;
    corto_generator g = data->g;


    g_fullOid(data->g, t, id);

    /* Function declaration */
    c_writeExport(data->g, data->header);
    g_fileWrite(data->header, " %s %sFromStr(%s value, corto_string str);\n",
      c_typeret(g, t, ret), id, c_typeptr(g, t, ptr));

    /* Function implementation */
    g_fileWrite(data->source, "%s %sFromStr(%s value, corto_string str) {\n",
      c_typeret(g, t, ret), id, c_typeptr(g, t, ptr));

    g_fileIndent(data->source);
    g_fileWrite(data->source, "corto_fromStrp(&value, corto_type(%s_o), str);\n", id);
    g_fileWrite(data->source, "return value;\n");
    g_fileDedent(data->source);
    g_fileWrite(data->source, "}\n\n");

    return 0;
}

corto_int16 c_apiTypeCopyIntern(
    corto_type t,
    c_apiWalk_t *data,
    corto_string func,
    corto_string returnType,
    corto_bool outParam)
{
    corto_id id, funcLower;
    corto_bool ptr = c_typeRequiresPtr(t);
    corto_bool outPtr = outParam && !t->reference;

    data->args = corto_llNew();
    g_fullOid(data->g, t, id);

    /* Function declaration */
    c_writeExport(data->g, data->header);
    g_fileWrite(data->header, " %s %s%s%s(%s%s%s dst, %s%s src);\n",
        returnType,
        t->reference ? "_" : "",
        id, func, id,
        (ptr || outPtr) ? "*" : "",
        outParam ? "*" : "",
        id,
        ptr ? "*" : "");

    /* Function implementation */
    g_fileWrite(data->source, "%s %s%s%s(%s%s%s dst, %s%s src) {\n",
        returnType,
        t->reference ? "_" : "",
        id, func, id,
        (ptr || outPtr) ? "*" : "",
        outParam ? "*" : "",
        id,
        ptr ? "*" : "");

    if (t->reference) {
        if (outParam) {
            c_apiCastMacroAddArg(data->args, "dst", NULL);
        } else {
            c_apiCastMacroAddArg(data->args, "dst", t);
        }
        c_apiCastMacroAddArg(data->args, "src", t);
        c_apiCastMacro(id, func, data);
        c_apiCastMacroFreeArgs(data->args);
        data->args = NULL;
    }

    g_fileIndent(data->source);

    strcpy(funcLower, func);
    *funcLower = tolower(*funcLower);

    if (t->reference) {
        g_fileWrite(data->source, "return corto_%s(%sdst, src);\n", funcLower, outParam ? "(corto_object*)" : "");
    } else {
        corto_bool isPtr = ptr || ((t->kind == CORTO_COLLECTION) && (corto_collection(t)->kind == CORTO_ARRAY));
        g_fileWrite(data->source, "return corto_%sp(%sdst, %s_o, %ssrc);\n", funcLower, isPtr ? "" : "&", id, isPtr ? "" : "&");
    }

    g_fileDedent(data->source);
    g_fileWrite(data->source, "}\n\n");

    return 0;
}

corto_int16 c_apiTypeCopy(corto_type t, c_apiWalk_t *data) {
    return c_apiTypeCopyIntern(t, data, "Copy", "corto_int16", TRUE);
}

corto_int16 c_apiTypeCompare(corto_type t, c_apiWalk_t *data) {
    return c_apiTypeCopyIntern(t, data, "Compare", "corto_equalityKind", FALSE);
}

corto_int16 c_apiDelegateInitCallbackAuto(
    corto_delegate t,
    corto_bool instance,
    c_apiWalk_t *data)
{
    corto_id id;

    g_fullOid(data->g, t, id);

    if (!instance) {
        g_fileWrite(data->header, "#define %sInitC_auto(d, callback) ", id);
        g_fileWrite(data->header, "%s d; %sInitC(&d, callback)\n", id, id);
    } else {
        g_fileWrite(
            data->header,
            "#define %sInitCInstance_auto(d, instance, callback)",
            id);
        g_fileWrite(
            data->header,
            "%s d; %sInitCInstance(&d, instance, callback)\n", id, id);
    }

    return 0;
}

corto_int16 c_apiDelegateInitCallback(
    corto_delegate t,
    corto_bool instance,
    c_apiWalk_t *data)
{
    corto_id returnId, id, paramId;
    int i, firstComma = 0;

    if (c_apiDelegateInitCallbackAuto(t, instance, data)) {
        goto error;
    }

    g_fullOid(data->g, t, id);
    g_fullOid(data->g, t->returnType, returnId);

    c_writeExport(data->g, data->header);
    if (!instance) {
        g_fileWrite(data->header, " corto_int16 %sInitC(%s *d, ", id, id);
        g_fileWrite(data->source, "corto_int16 %sInitC(%s *d, ", id, id);
    } else {
        g_fileWrite(
            data->header,
            " corto_int16 %sInitCInstance(%s *d, corto_object instance, ",
            id, id);

        g_fileWrite(
            data->source,
            "corto_int16 %sInitCInstance(%s *d, corto_object instance, ",
            id, id);
    }

    g_fileWrite(data->header, "%s ___ (*callback)(", returnId);
    g_fileWrite(data->source, "%s ___ (*callback)(", returnId);

    if (instance) {
        g_fileWrite(data->header, "corto_object");
        g_fileWrite(data->source, "corto_object");
        firstComma = 1;
    }

    for (i = 0; i < t->parameters.length; i++) {
        corto_parameter *p = &t->parameters.buffer[i];
        if (firstComma) {
            g_fileWrite(data->header, ", ");
            g_fileWrite(data->source, ", ");
        }
        g_fileWrite(data->header, "%s", g_fullOid(data->g, p->type, paramId));
        g_fileWrite(data->source, "%s", g_fullOid(data->g, p->type, paramId));
        if (c_paramRequiresPtr(p)) {
            g_fileWrite(data->header, "*");
            g_fileWrite(data->source, "*");
        }
        firstComma ++;
    }

    if (!firstComma) {
        g_fileWrite(data->header, "void");
        g_fileWrite(data->source, "void");
    }

    g_fileWrite(data->header, "));\n");
    g_fileWrite(data->source, ")) {\n");
    g_fileIndent(data->source);

    if (instance) {
        g_fileWrite(data->source, "d->_parent.instance = instance;\n");
        g_fileWrite(data->source, "corto_claim(instance);\n");
    }
    g_fileWrite(data->source, "d->_parent.procedure = corto_functionDeclare();\n");
    g_fileWrite(data->source, "d->_parent.procedure->kind = CORTO_PROCEDURE_CDECL;\n", id);
    g_fileWrite(
        data->source, "corto_function_parseParamString(d->_parent.procedure, \"(");

    firstComma = 0;
    if (instance) {
        g_fileWrite(data->source, "object instance");
        firstComma = 1;
    }

    for (i = 0; i < t->parameters.length; i++) {
        corto_parameter *p = &t->parameters.buffer[i];
        if (firstComma) {
            g_fileWrite(data->source, ", ");
        }
        g_fileWrite(data->source, "%s %s", corto_fullpath(NULL, p->type), p->name);
        firstComma ++;
    }

    g_fileWrite(data->source, ")\");\n");
    g_fileWrite(
        data->source, "d->_parent.procedure->fptr = (corto_word)callback;\n");

    g_fileWrite(data->source, "corto_define(d->_parent.procedure);\n");
    g_fileWrite(data->source, "return 0;\n");
    g_fileDedent(data->source);
    g_fileWrite(data->source, "}\n\n");

    return 0;
error:
    return -1;
}

corto_int16 c_apiDelegateCall(corto_delegate t, c_apiWalk_t *data) {
    corto_id returnId, id, paramType, paramName;
    g_fullOid(data->g, t->returnType, returnId);
    g_fullOid(data->g, t, id);
    corto_bool hasReturn = t->returnType->reference || (t->returnType->kind != CORTO_VOID);

    g_fileWrite(data->header, "corto_int16 %sCall(%s *_delegate", id, id);
    g_fileWrite(data->source, "corto_int16 %sCall(%s *_delegate", id, id);

    if (hasReturn) {
        g_fileWrite(data->header, ", %s* _result", returnId);
        g_fileWrite(data->source, ", %s* _result", returnId);
    }

    {corto_parameterseqForeach(t->parameters, p) {
        g_fullOid(data->g, p.type, paramType);
        g_id(data->g, p.name, paramName);
        corto_bool ptr =
            !p.type->reference &&
            (p.passByReference || (p.type->kind == CORTO_COMPOSITE));

        g_fileWrite(data->header, ", %s%s %s", paramType, ptr ? "*" : "", paramName);
        g_fileWrite(data->source, ", %s%s %s", paramType, ptr ? "*" : "", paramName);
    }}

    g_fileWrite(data->header, ");\n");
    g_fileWrite(data->source, ") {\n");
    g_fileIndent(data->source);

    g_fileWrite(data->source, "if (_delegate->_parent.procedure) {\n");
    g_fileIndent(data->source);
    g_fileWrite(data->source, "if (_delegate->_parent.instance) {\n");
    g_fileIndent(data->source);
    if (hasReturn) {
        g_fileWrite(data->source, "corto_call(_delegate->_parent.procedure, _result, _delegate->_parent.instance");
    } else {
        g_fileWrite(data->source, "corto_call(_delegate->_parent.procedure, NULL, _delegate->_parent.instance");
    }
    {corto_parameterseqForeach(t->parameters, p) {
        g_fullOid(data->g, p.type, paramType);
        g_id(data->g, p.name, paramName);
        g_fileWrite(data->source, ", %s", paramName);
    }}
    g_fileWrite(data->source, ");\n");
    g_fileDedent(data->source);
    g_fileWrite(data->source, "} else {\n");
    g_fileIndent(data->source);
    if (hasReturn) {
        g_fileWrite(data->source, "corto_call(_delegate->_parent.procedure, _result");
    } else {
        g_fileWrite(data->source, "corto_call(_delegate->_parent.procedure, NULL");
    }
    {corto_parameterseqForeach(t->parameters, p) {
        g_fullOid(data->g, p.type, paramType);
        g_id(data->g, p.name, paramName);
        g_fileWrite(data->source, ", %s", paramName);
    }}
    g_fileWrite(data->source, ");\n");
    g_fileDedent(data->source);
    g_fileWrite(data->source, "}\n");
    g_fileDedent(data->source);
    g_fileWrite(data->source, "} else {\n");
    g_fileIndent(data->source);
    g_fileWrite(data->source, "return -1;\n");
    g_fileDedent(data->source);
    g_fileWrite(data->source, "}\n");
    g_fileWrite(data->source, "return 0;\n");
    g_fileDedent(data->source);
    g_fileWrite(data->source, "}\n\n");

    return 0;
}
