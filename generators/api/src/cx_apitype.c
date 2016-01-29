
#include "c_api.h"
#include "c_common.h"
#include "ctype.h"

typedef struct c_arg {
    corto_string name;
    corto_type type;
} c_arg;

static void c_apiCastMacroAddArg(corto_ll args, corto_string name, corto_type type) {
    c_arg *arg = corto_alloc(sizeof(c_arg));
    arg->name = corto_strdup(name);
    arg->type = type;
    corto_llAppend(args, arg);
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

static corto_int16 c_apiCastAuto(corto_string id, corto_bool scoped, corto_bool ref, corto_string func, c_apiWalk_t *data) {
    corto_uint32 count = 0;

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
        ") %s%s _name = %s%s(%s",
        id,
        ref ? "" : "*",
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
        if (arg->type && arg->type->reference && (arg->type->kind != CORTO_VOID)) {
            corto_id id;
            g_fullOid(data->g, arg->type, id);
            g_fileWrite(data->header, "%s(%s)", id, arg->name);
        } else {
            g_fileWrite(data->header, "%s", arg->name);
        }
        count++;
    }

    g_fileWrite(data->header, ")\n");

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

        corto_genMemberName(data->g, data->memberCache, m, memberIdTmp);
        g_id(data->g, memberIdTmp, memberParamId);
        g_id(data->g, corto_nameof(m), memberId);

        /* If member is of array-type, use memcpy */
        if ((m->type->kind == CORTO_COLLECTION) && (corto_collection(m->type)->kind == CORTO_ARRAY)) {
            corto_id typeId, postfix;
            /* Get typespecifier */
            if (c_specifierId(data->g, m->type, typeId, NULL, postfix)) {
                goto error;
            }
            g_fileWrite(data->source, "memcpy(");

            /* Cast object to right type */
            if (data->current == corto_parentof(m)) {
                g_fileWrite(data->source, "this->%s",
                        memberId);
            } else {
                corto_id typeId;
                g_fileWrite(data->source, "%s(this)->%s",
                        g_fullOid(data->g, corto_parentof(m), typeId), memberId);
            }

            g_fileWrite(data->source, ", %s, sizeof(%s%s));\n", memberParamId, typeId, postfix);
        } else {
            if (m->type->reference) {
                g_fileWrite(data->source, "corto_setref(&");
            } else if (m->type->kind == CORTO_COMPOSITE) {
                g_fileWrite(data->source, "corto_copyp(&");
            } else if ((m->type->kind == CORTO_PRIMITIVE) && (corto_primitive(m->type)->kind == CORTO_TEXT)) {
                g_fileWrite(data->source, "corto_setstr(&");
            }

            /* Cast object to right type */
            if (data->current == corto_parentof(m)) {
                g_fileWrite(data->source, "this->%s",
                        memberId);
            } else {
                if (corto_type(corto_parentof(m))->reference) {
                    corto_id typeId;
                    g_fileWrite(data->source, "%s(this)->%s",
                            g_fullOid(data->g, corto_parentof(m), typeId), memberId);
                } else {
                    corto_id typeId;
                    g_fileWrite(data->source, "((%s*)this)->%s",
                            g_fullOid(data->g, corto_parentof(m), typeId), memberId);
                }
            }

            /* Strdup strings */
            if ((m->type->kind == CORTO_PRIMITIVE) && (corto_primitive(m->type)->kind == CORTO_TEXT)) {
                g_fileWrite(data->source, ", %s);\n", memberParamId);
            } else if (m->type->reference) {
                if (m->type->kind != CORTO_VOID) {
                    corto_id id;
                    g_fileWrite(data->source, ", %s(%s));\n", g_fullOid(data->g, m->type, id), memberParamId);
                } else {
                    g_fileWrite(data->source, ", %s);\n", memberParamId);
                }
            } else if (m->type->kind == CORTO_COMPOSITE) {
                corto_id objId;
                g_fileWrite(data->source, ", %s_o, %s);\n",
                g_fullOid(data->g, m->type, objId),
                memberParamId);
            } else {
                g_fileWrite(data->source, " = %s;\n", memberParamId);
            }
        }

        data->parameterCount++;
    }

    return 0;
error:
    return -1;
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

corto_int16 c_apiTypeCreateIntern(corto_type t, c_apiWalk_t *data, corto_string func, corto_bool scoped, corto_bool define) {
    corto_id id;
    struct corto_serializer_s s;
    corto_ll args = corto_llNew();

    data->parameterCount = 0;
    g_fullOid(data->g, t, id);

    /* Collect arguments for casting macro */
    data->args = args;

    /* Function declaration */
    c_writeExport(data->g, data->header);
    g_fileWrite(data->header, " %s%s _%s%s(", id, t->reference ? "" : "*", id, func);

    /* Function implementation */
    g_fileWrite(data->source, "%s%s _%s%s(", id, t->reference ? "" : "*", id, func);

    if (scoped) {
        g_fileWrite(data->header, "corto_object _parent, corto_string _name");
        g_fileWrite(data->source, "corto_object _parent, corto_string _name");
        c_apiCastMacroAddArg(args, "_parent", corto_object_o);
        c_apiCastMacroAddArg(args, "_name", corto_type(corto_string_o));
        data->parameterCount = 2;
    }

    /* Write public members as arguments for source and header */
    if (define) {
        if (t->kind == CORTO_COMPOSITE) {
            s = c_apiParamSerializer();
            corto_metaWalk(&s, corto_type(t), data);
        }

        if ((t->kind != CORTO_COMPOSITE) && (t->kind != CORTO_COLLECTION) && (t->kind != CORTO_VOID)) {
            if (data->parameterCount) {
                g_fileWrite(data->source, ", ");
                g_fileWrite(data->header, ", ");
            }
            g_fileWrite(data->source, "%s value", id);
            g_fileWrite(data->header, "%s value", id);
            c_apiCastMacroAddArg(args, "value", t);
            data->parameterCount++;
        }

        if (corto_instanceof(corto_procedure_o, t)) {
            if (data->parameterCount) {
                g_fileWrite(data->source, ", ");
                g_fileWrite(data->header, ", ");
            }
            g_fileWrite(data->source, "void(*_impl)(corto_function f, void *result, void *args)");
            g_fileWrite(data->header, "void(*_impl)(corto_function f, void *result, void *args)");
            c_apiCastMacroAddArg(args, "_impl", NULL);
            data->parameterCount++;
        }
    }

    /* If there are no parameters, write 'void' */
    if (!data->parameterCount) {
        g_fileWrite(data->header, "void");
        g_fileWrite(data->source, "void");
    }

    /* Write closing brackets for argumentlist in source and header */
    g_fileWrite(data->header, ");\n");

    /* Write casting macro */
    c_apiCastMacro(id, func, data);
    c_apiCastAuto(id, scoped, t->reference, func, data);
    c_apiCastMacroFreeArgs(data->args);
    data->args = NULL;

    g_fileWrite(data->source, ") {\n");

    g_fileIndent(data->source);
    g_fileWrite(data->source, "%s%s this;\n", id, t->reference ? "" : "*");
    if (scoped) {
        g_fileWrite(data->source, "this = corto_declareChild(_parent, _name, %s_o);\n", id);
    } else {
        g_fileWrite(data->source, "this = corto_declare(%s_o);\n", id);
    }

    g_fileWrite(data->source, "if (!this) {\n");
    g_fileIndent(data->source);
    g_fileWrite(data->source, "return NULL;\n");
    g_fileDedent(data->source);
    g_fileWrite(data->source, "}\n");

    if (define) {
      /* Assignments */
        if ((t->kind != CORTO_COMPOSITE) && (t->kind != CORTO_COLLECTION) && (t->kind != CORTO_VOID)) {
            g_fileWrite(data->source, "*this = value;\n");
        } else {
            s = c_apiAssignSerializer();
            corto_metaWalk(&s, corto_type(t), data);

            if (corto_instanceof(corto_procedure_o, t)) {
                g_fileWrite(data->source, "corto_function(this)->impl = (corto_word)_impl;\n");
            }
        }
        if (t->kind != CORTO_VOID) {
            /* Define object */
            g_fileWrite(data->source, "if (this && corto_define(this)) {\n");
            g_fileIndent(data->source);
            g_fileWrite(data->source, "corto_release(this);\n");
            g_fileWrite(data->source, "this = NULL;\n");
            g_fileDedent(data->source);
            g_fileWrite(data->source, "}\n");
        }
    }

    g_fileWrite(data->source, "return this;\n");
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
    corto_id id;
    struct corto_serializer_s s;
    corto_string func = isUpdate ? doUpdate ? "Update" : "Set" : "Define";

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

    /* Function declaration */
    g_fileWrite(data->header, "_%s%s(%s%s _this", id, func, id, t->reference ? "" : "*");

    /* Function implementation */
    g_fileWrite(data->source, "_%s%s(%s%s this", id, func, id, t->reference ? "" : "*");

    c_apiCastMacroAddArg(data->args, "_this", t);

    /* Write public members as arguments for source and header */
    if (t->kind == CORTO_COMPOSITE) {
      s = c_apiParamSerializer();
      corto_metaWalk(&s, corto_type(t), data);
  }

    if ((data->parameterCount == 1) &&
        (t->kind != CORTO_COMPOSITE) &&
        (t->kind != CORTO_VOID)) {
        g_fileWrite(data->source, ", %s value", id);
        g_fileWrite(data->header, ", %s value", id);
        c_apiCastMacroAddArg(data->args, "value", NULL);
    }

    if (corto_instanceof(corto_procedure_o, t) && !strcmp(func, "Define")) {
        if (data->parameterCount) {
            g_fileWrite(data->source, ", ");
            g_fileWrite(data->header, ", ");
        }
        g_fileWrite(data->source, "void(*_impl)(corto_function f, void *result, void *args)");
        g_fileWrite(data->header, "void(*_impl)(corto_function f, void *result, void *args)");
        c_apiCastMacroAddArg(data->args, "_impl", NULL);
        data->parameterCount++;
    }

    /* Write closing brackets for argumentlist in source and header */
    g_fileWrite(data->header, ");\n");
    g_fileWrite(data->source, ") {\n");
    g_fileIndent(data->source);

    /* Write cast macro */
    c_apiCastMacro(id, func, data);
    c_apiCastMacroFreeArgs(data->args);
    data->args = NULL;

    if (isUpdate && doUpdate && (t->kind != CORTO_VOID)) {
      g_fileWrite(data->source, "if (!corto_updateBegin(this)) {\n");
        g_fileIndent(data->source);
    }

    /* Member assignments */
    if (data->parameterCount > 1) {
        s = c_apiAssignSerializer();
        corto_metaWalk(&s, corto_type(t), data);
    } else if ((t->kind != CORTO_COMPOSITE) && (t->kind != CORTO_VOID)) {
        if (t->kind != CORTO_COLLECTION) {
            g_fileWrite(data->source, "*this = value;\n");
        } else {
            if (corto_collection(t)->kind == CORTO_SEQUENCE) {
                g_fileWrite(data->source, "corto_copyp(this, %s_o, &value);\n", id);
            } else {
                g_fileWrite(data->source, "corto_copyp(this, %s_o, value);\n", id);
            }
        }
    } else if (isUpdate && !doUpdate) {
        g_fileWrite(data->source, "CORTO_UNUSED(this);\n");
    }

    /* Define object */
    if (isUpdate && doUpdate) {
        if (t->kind != CORTO_VOID) {
            g_fileWrite(data->source, "corto_updateEnd(this);\n");
            g_fileDedent(data->source);
            g_fileWrite(data->source, "} else {\n");
            g_fileIndent(data->source);
            g_fileWrite(data->source, "return -1;\n");
            g_fileDedent(data->source);
            g_fileWrite(data->source, "}\n");
            g_fileWrite(data->source, "return 0;\n");
        } else {
            g_fileWrite(data->source, "return corto_update(this);\n");
        }
    } else if (!isUpdate) {
        if (corto_instanceof(corto_procedure_o, t)) {
            g_fileWrite(data->source, "corto_function(this)->impl = (corto_word)_impl;\n");
        }
      g_fileWrite(data->source, "return corto_define(this);\n");
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
    corto_bool pointer = (t->kind == CORTO_COMPOSITE) && !t->reference;

    data->args = corto_llNew();
    g_fullOid(data->g, t, id);

    /* Function declaration */
    c_writeExport(data->g, data->header);
    g_fileWrite(data->header, " corto_string _%sStr(%s%s value);\n", id, id, pointer ? "*" : "");

    /* Function implementation */
    g_fileWrite(data->source, "corto_string _%sStr(%s%s value) {\n", id, id, pointer ? "*" : "");

    c_apiCastMacroAddArg(data->args, "value", t);
    c_apiCastMacro(id, "Str", data);
    c_apiCastMacroFreeArgs(data->args);
    data->args = NULL;

    g_fileIndent(data->source);
    g_fileWrite(data->source, "corto_string result;\n", id);

    g_fileWrite(data->source, "corto_value v;\n", id);
    if (t->reference) {
        g_fileWrite(data->source, "corto_valueObjectInit(&v, value, corto_type(%s_o));\n", id);
    } else {
        g_fileWrite(data->source, "corto_valueValueInit(&v, NULL, corto_type(%s_o), &value);\n", id);
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
        g_fileWrite(data->source, "corto_valueValueInit(&v, NULL, corto_type(%s_o), value);\n", id);
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
    corto_id id;

    g_fullOid(data->g, t, id);

    /* Function declaration */
    c_writeExport(data->g, data->header);
    g_fileWrite(data->header, " %s%s %sFromStr(%s%s value, corto_string str);\n",
      id, t->reference ? "" : "*", id, id, t->reference ? "" : "*");

    /* Function implementation */
    g_fileWrite(data->source, "%s%s %sFromStr(%s%s value, corto_string str) {\n",
      id, t->reference ? "" : "*", id, id, t->reference ? "" : "*");

    g_fileIndent(data->source);
    g_fileWrite(data->source, "corto_fromStrp(&value, corto_type(%s_o), str);\n", id);
    g_fileWrite(data->source, "return value;\n");
    g_fileDedent(data->source);
    g_fileWrite(data->source, "}\n\n");

    return 0;
}

corto_int16 c_apiTypeCopyIntern(corto_type t, c_apiWalk_t *data, corto_string func, corto_bool outParam) {
    corto_id id, funcLower;

    data->args = corto_llNew();
    g_fullOid(data->g, t, id);

    /* Function declaration */
    c_writeExport(data->g, data->header);
    g_fileWrite(data->header, " corto_int16 _%s%s(%s%s %sdst, %s%s src);\n",
      id, func, id, t->reference ? "" : "*", outParam ? "*" : "", id, t->reference ? "" : "*");

    /* Function implementation */
    g_fileWrite(data->source, "corto_int16 _%s%s(%s%s %sdst, %s%s src) {\n",
      id, func, id, t->reference ? "" : "*", outParam ? "*" : "", id, t->reference ? "" : "*");

    if (outParam) {
        c_apiCastMacroAddArg(data->args, "dst", NULL);
    } else {
        c_apiCastMacroAddArg(data->args, "dst", t);
    }
    c_apiCastMacroAddArg(data->args, "src", t);
    c_apiCastMacro(id, func, data);
    c_apiCastMacroFreeArgs(data->args);
    data->args = NULL;

    g_fileIndent(data->source);

    strcpy(funcLower, func);
    *funcLower = tolower(*funcLower);

  if (t->reference) {
      g_fileWrite(data->source, "return corto_%s(%sdst, src);\n", funcLower, outParam ? "(corto_object*)" : "");
  } else {
      g_fileWrite(data->source, "corto_value v1, v2;\n", id);
      g_fileWrite(data->source, "corto_valueValueInit(&v1, NULL, corto_type(%s_o), dst);\n", id);
      g_fileWrite(data->source, "corto_valueValueInit(&v2, NULL, corto_type(%s_o), src);\n", id);
      g_fileWrite(data->source, "return corto_%sv(&v1, &v2);\n", funcLower);
  }

  g_fileDedent(data->source);
  g_fileWrite(data->source, "}\n\n");

  return 0;
}

corto_int16 c_apiTypeCopy(corto_type t, c_apiWalk_t *data) {
    return c_apiTypeCopyIntern(t, data, "Copy", TRUE);
}

corto_int16 c_apiTypeCompare(corto_type t, c_apiWalk_t *data) {
    return c_apiTypeCopyIntern(t, data, "Compare", FALSE);
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

    g_fileWrite(data->header, "%s(*callback)(", returnId);
    g_fileWrite(data->source, "%s(*callback)(", returnId);

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

    g_fileWrite(data->header, "));\n");
    g_fileWrite(data->source, ")) {\n");
    g_fileIndent(data->source);

    if (instance) {
        g_fileWrite(data->source, "d->_parent.instance = instance;\n");
        g_fileWrite(data->source, "corto_claim(instance);\n");
    }
    g_fileWrite(data->source, "d->_parent.procedure = corto_functionDeclare();\n");
    g_fileWrite(data->source, "void __%s(corto_function f, void *result, void *args);\n", id);
    g_fileWrite(data->source, "d->_parent.procedure->impl = (corto_word)__%s;\n", id);
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
        data->source, "d->_parent.procedure->implData = (corto_word)callback;\n");

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
