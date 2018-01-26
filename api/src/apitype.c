
#include "api.h"
#include "driver/gen/c/common/common.h"
#include "ctype.h"

typedef struct c_arg {
    corto_string name;
    corto_type type;
    corto_bool move;
    corto_string nativeCast;
    corto_bool optional;
    corto_bool isobject;
} c_arg;

static void c_apiCastMacroAddArg(corto_ll args, corto_string name, corto_type type, corto_bool optional) {
    if (args) {
        c_arg *arg = corto_alloc(sizeof(c_arg));
        arg->name = corto_strdup(name);
        arg->type = type;
        arg->move = FALSE;
        arg->nativeCast = NULL;
        arg->optional = optional;
        arg->isobject = type && type->reference && (type->kind != CORTO_VOID);
        corto_ll_append(args, arg);
    }
}

static void c_apiCastMacroAddThis(corto_ll args, corto_string name, corto_type type, corto_bool optional, corto_bool isobject, g_generator g) {
    if (args) {
        c_arg *arg = corto_alloc(sizeof(c_arg));
        arg->name = corto_strdup(name);
        arg->type = type;
        arg->move = FALSE;
        if (!isobject && (type->kind == CORTO_COLLECTION && corto_collection(type)->kind == CORTO_ARRAY)) {
            corto_id elemId;
            c_typeId(g, corto_collection(type)->elementType, elemId);
            strcat(elemId, "*");
            arg->nativeCast = corto_strdup(elemId);
        } else {
            arg->nativeCast = NULL;
        }
        arg->optional = optional;
        arg->isobject = isobject;
        corto_ll_append(args, arg);
    }
}

static void c_apiCastMacroAddArgNativeCast(corto_ll args, corto_string name, corto_string type) {
    if (args) {
        c_arg *arg = corto_alloc(sizeof(c_arg));
        arg->name = corto_strdup(name);
        arg->type = NULL;
        arg->move = FALSE;
        arg->nativeCast = corto_strdup(type);
        arg->optional = FALSE;
        arg->isobject = FALSE;
        corto_ll_append(args, arg);
    }
}

static void c_apiCastMacroAddArgMove(corto_ll args, corto_string name, corto_type type, corto_bool optional) {
    if (args) {
        c_arg *arg = corto_alloc(sizeof(c_arg));
        arg->name = corto_strdup(name);
        arg->type = type;
        arg->move = TRUE;
        arg->nativeCast = NULL;
        arg->optional = optional;
        arg->isobject = type && type->reference && (type->kind != CORTO_VOID);
        corto_ll_append(args, arg);
    }
}

static void c_apiCastMacroFreeArgs(corto_ll args) {
    corto_iter iter = corto_ll_iter(args);
    while (corto_iter_hasNext(&iter)) {
        c_arg *arg = corto_iter_next(&iter);
        corto_dealloc(arg->name);
        corto_dealloc(arg);
    }
    corto_ll_free(args);
}

static corto_int16 c_apiCastAuto(
    corto_type t,
    corto_bool scoped,
    corto_string func,
    corto_member m,
    c_apiWalk_t *data)
{
    corto_uint32 count = 0;
    corto_id ret, id;
    g_fullOid(data->g, t, id);

    g_fileWrite(
        data->header,
        "#define %s%s%s%s_auto(",
        id,
        func,
        m ? "_" : "",
        m ? corto_idof(m) : "");

    if (!scoped) {
        g_fileWrite(data->header, "_id");
        count = 1;
    }

    corto_iter iter = corto_ll_iter(data->args);
    while (corto_iter_hasNext(&iter)) {
        c_arg *arg = corto_iter_next(&iter);
        if (count) {
            g_fileWrite(data->header, ", ");
        }
        g_fileWrite(data->header, "%s", arg->name);
        count++;
    }

    g_fileWrite(
        data->header,
        ") %s _id = %s%s%s%s(%s",
        c_typeret(data->g, t, C_ByReference, ret),
        id,
        func,
        m ? "_" : "",
        m ? corto_idof(m) : "",
        scoped ? "_parent, #_id" : "");

    iter = corto_ll_iter(data->args);
    count = 0;

    if (scoped) {
        corto_iter_hasNext(&iter) ? corto_iter_next(&iter) : 0; /* _parent */
        corto_iter_hasNext(&iter) ? corto_iter_next(&iter) : 0; /* _id */
        count = 2;
    }

    while (corto_iter_hasNext(&iter)) {
        c_arg *arg = corto_iter_next(&iter);
        if (count) {
            g_fileWrite(data->header, ", ");
        }
        g_fileWrite(data->header, "%s", arg->name);
        count++;
    }

    g_fileWrite(data->header, "); (void)_id\n");

    return 0;
}

static corto_int16 c_apiCastMacroCall(
  corto_string id,
  corto_string name,
  corto_member m,
  c_apiWalk_t *data)
{
    g_fileWrite(data->header, " _%s%s%s%s(",
      id,
      name,
      m ? "_" : "",
      m ? corto_idof(m) : "");

    corto_bool cpp = !strcmp(g_getAttribute(data->g, "c4cpp"), "true");
    corto_iter iter = corto_ll_iter(data->args);
    corto_uint32 count = 0;
    while (corto_iter_hasNext(&iter)) {
        c_arg *arg = corto_iter_next(&iter);
        corto_id argName;
        if (arg->type && arg->optional) {
            corto_id typeId;
            c_typeId(data->g, arg->type, typeId);
            sprintf(argName, "%s__optional_##%s", typeId, arg->name);
        } else {
            strcpy(argName, arg->name);
        }
        if (count) {
            g_fileWrite(data->header, ", ");
        }
        if (arg->move && cpp) {
            g_fileWrite(data->header, "std::move(");
        }
        if (arg->isobject) {
            corto_id id;
            if (corto_typeof(arg->type) != (corto_type)corto_target_o) {
                g_fullOid(data->g, arg->type, id);
                g_fileWrite(data->header, "%s(%s)", id, argName);
            } else {
                g_fileWrite(data->header, "%s", argName);
            }
        } else {
            if (arg->nativeCast) {
                g_fileWrite(data->header, "(%s)", arg->nativeCast);
            }
            g_fileWrite(data->header, "%s", argName);
        }
        if (arg->move && cpp) {
            g_fileWrite(data->header, ")");
        }
        count++;
    }

    g_fileWrite(data->header, ")\n");

    return 0;
}

static corto_int16 c_apiCastMacro(
    corto_string id,
    corto_string name,
    corto_member m,
    c_apiWalk_t *data)
{
    corto_uint32 count = 0;

    g_fileWrite(data->header, "#define %s%s%s%s(",
      id,
      name,
      m ? "_" : "",
      m ? corto_idof(m) : "");

    corto_iter iter = corto_ll_iter(data->args);
    while (corto_iter_hasNext(&iter)) {
        c_arg *arg = corto_iter_next(&iter);
        if (count) {
            g_fileWrite(data->header, ", ");
        }
        g_fileWrite(data->header, "%s", arg->name);
        count++;
    }

    g_fileWrite(data->header, ")");

    c_apiCastMacroCall(id, name, m, data);

    return 0;
}

static void c_apiPrintArgs(corto_ll args, corto_uint32 count, corto_uint32 skip, g_file file) {
    corto_iter iter = corto_ll_iter(args);
    corto_uint32 i;

    for (i = 0; i < skip; i ++) {
        corto_iter_next(&iter);
    }
    while (corto_iter_hasNext(&iter)) {
        c_arg *arg = corto_iter_next(&iter);
        if (count) {
            g_fileWrite(file, ", ");
        }
        g_fileWrite(file, "%s", arg->name);
        count++;
    }
    g_fileWrite(file, ")");
}

static corto_int16 c_apiCastMacroSet(
    corto_string id,
    corto_type t,
    c_apiWalk_t *data)
{
    corto_id typeId;
    c_varId(data->g, t, typeId);

    /* optional NotSet macro */
    g_fileWrite(
      data->header,
      "#define %s__optional_not_set NULL\n",
      id);

    /* optional Set macro */
    g_fileWrite(
      data->header,
      "#define %s__optional_set(",
      id);
    c_apiPrintArgs(data->args, 0, 1, data->header);

    g_fileWrite(data->header, " (%s*)%sAssign((%s*)corto_ptr_new(%s)", id, id, id, typeId);
    c_apiPrintArgs(data->args, 1, 1, data->header);
    g_fileWrite(data->header, "\n");

    /* optional SetCond macro */
    g_fileWrite(
      data->header,
      "#define %s__optional_set_cond(cond",
      id);
    c_apiPrintArgs(data->args, 1, 1, data->header);

    g_fileWrite(data->header, " cond ? (%s*)%s_assign((%s*)corto_ptr_new(%s)", id, id, id, typeId);
    c_apiPrintArgs(data->args, 1, 1, data->header);
    g_fileWrite(data->header, " : NULL\n");

    /* Unset macro */
    g_fileWrite(
      data->header,
      "#define %s_unset(_this) _this ? corto_ptr_free(_this, %s), 0 : 0; _this = NULL;\n",
      id,
      typeId);

    /* Assign macro */
    g_fileWrite(data->header, "#define %s_assign(", id);
    c_apiPrintArgs(data->args, 0, 0, data->header);
    c_apiCastMacroCall(id, "_assign", NULL, data);

    /* Set macro */
    g_fileWrite(data->header, "#define %s_set(", id);
    c_apiPrintArgs(data->args, 0, 0, data->header);
    g_fileWrite(data->header, " _this = _this ? _this : (%s*)corto_ptr_new(%s);", id, typeId);
    c_apiCastMacroCall(id, "_assign", NULL, data);

    return 0;
}

/* Assign primitive value */
static corto_int16 c_apiAssign(
    corto_type t,
    corto_bool ptr,
    corto_modifier modifiers,
    corto_string lvalue,
    corto_string rvalue,
    c_apiWalk_t *data)
{
    if (corto_typeof(t) == (corto_type)corto_target_o) {
        corto_id _this;
        if (data->owned) {
            sprintf(_this, "%s->actual", lvalue);
        } else {
            sprintf(_this, "%s->target", lvalue);
        }
        c_apiAssign(corto_target(t)->type, FALSE, 0, _this, rvalue, data);
    } else if (modifiers & CORTO_OPTIONAL) {
        corto_id varId;
        g_fileWrite(data->source, "if (%s) {\n", lvalue);
        g_fileIndent(data->source);
        g_fileWrite(data->source, "corto_ptr_deinit(%s, %s);\n", lvalue, c_varId(data->g, t, varId));
        g_fileWrite(data->source, "corto_dealloc(%s);\n", lvalue);
        g_fileDedent(data->source);
        g_fileWrite(data->source, "}\n");
        g_fileWrite(data->source, "%s%s = %s;\n",
            (!c_typeRequiresPtr(t) && ptr) ? "*" : "",
            lvalue, rvalue);
    } else {
        if (t->reference) {
            g_fileWrite(data->source, "corto_set_ref(%s%s, %s);\n",
                    ptr ? "" : "&", lvalue, rvalue);
        } else if (t->kind == CORTO_PRIMITIVE) {
            if (corto_primitive(t)->kind == CORTO_TEXT) {
                g_fileWrite(data->source, "corto_set_str(%s%s, %s);\n",
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
            if (noAmpersand) {
                g_fileWrite(data->source, "if (%s) {\n", rvalue);
                g_fileIndent(data->source);
            }
            if ((t->kind == CORTO_COLLECTION) &&
                (corto_collection(t)->kind != CORTO_SEQUENCE))
            {
                g_fileWrite(data->source, "if (%s) {\n", rvalue);
                g_fileIndent(data->source);
            }
            g_fileWrite(data->source, "corto_ptr_copy(&%s, %s_o, %s%s);\n",
                lvalue, id,
                noAmpersand ? "" : "&",
                rvalue);
            if ((t->kind == CORTO_COLLECTION) &&
                (corto_collection(t)->kind != CORTO_SEQUENCE))
            {
                g_fileDedent(data->source);
                g_fileWrite(data->source, "}\n");
            }
            if (noAmpersand) {
                g_fileDedent(data->source);
                g_fileWrite(data->source, "}\n");
            }
        }
    }

    return 0;
}

/* Translate members to function parameters. */
static corto_int16 c_apiAssignMember(corto_walk_opt* s, corto_value* v, void* userData) {
    c_apiWalk_t *data;
    corto_member m;
    corto_id memberIdTmp, memberParamId, memberId;

    CORTO_UNUSED(s);

    if (v->kind == CORTO_MEMBER) {
        m = v->is.member.t;
        data = userData;
        corto_id ptr, lvalue, type;
        corto_bool isUnionCase =
          corto_instanceof(corto_union_o, corto_parentof(m));

        corto_genMemberName(data->g, data->memberCache, m, memberIdTmp);
        g_id(data->g, memberIdTmp, memberParamId);
        g_id(data->g, corto_idof(m), memberId);
        g_fullOid(data->g, corto_parentof(m), type);

        sprintf(lvalue,
            "((%s)%s)->%s%s",
            c_typeptr(data->g, corto_parentof(m), ptr),
            data->_this,
            isUnionCase ? "is." : "",
            memberId);

        c_apiAssign(m->type, m->modifiers & CORTO_OBSERVABLE, m->modifiers, lvalue, memberParamId, data);
        data->parameterCount++;
    }

    return 0;
}

/* Translate members to function parameters. */
static corto_int16 c_apiParamMember(corto_walk_opt* s, corto_value* v, void* userData) {
    c_apiWalk_t* data;
    corto_member m;
    corto_id typeSpec, typePostfix, memberIdTmp, memberId;

    CORTO_UNUSED(s);

    if (v->kind == CORTO_MEMBER) {
        m = v->is.member.t;
        data = userData;
        corto_bool isOptional = m->modifiers & CORTO_OPTIONAL;
        corto_bool requiresPtr = isOptional;
        corto_type type;
        if (corto_typeof(m->type) != corto_type(corto_target_o)) {
            type = m->type;
        } else {
            type = corto_target(m->type)->type;
        }
        bool isText = type->kind == CORTO_PRIMITIVE &&
            corto_primitive(type)->kind == CORTO_TEXT;

        if (data->parameterCount) {
            g_fileWrite(data->header, ", ");
            g_fileWrite(data->source, ", ");
        }

        if (isText && !isOptional) {
            strcpy(typeSpec, "const char *");
            typePostfix[0] = '\0';
        } else {
            c_specifierId(data->g, type, typeSpec, NULL, typePostfix);
        }
        requiresPtr |= c_typeRequiresPtr(type);

        corto_genMemberName(data->g, data->memberCache, m, memberIdTmp);

        if (data->args) {
            c_apiCastMacroAddArg(
              data->args,
              g_id(data->g, memberIdTmp, memberId),
              m->type,
              isOptional);
        }

        g_fileWrite(data->header, "%s%s%s %s",
               typeSpec,
               typePostfix,
               requiresPtr ? "*" : "",
               g_id(data->g, memberIdTmp, memberId));

        g_fileWrite(data->source, "%s%s%s %s",
                typeSpec,
                typePostfix,
                requiresPtr ? "*" : "",
                g_id(data->g, memberIdTmp, memberId));

        data->parameterCount++;
    }

    return 0;
}

/* Member parameter serializer */
static corto_walk_opt c_apiParamSerializer(void) {
    corto_walk_opt s;

    corto_walk_init(&s);
    s.metaprogram[CORTO_MEMBER] = c_apiParamMember;
    s.access = CORTO_LOCAL|CORTO_READONLY|CORTO_PRIVATE|CORTO_HIDDEN;
    s.accessKind = CORTO_NOT;
    s.optionalAction = CORTO_WALK_OPTIONAL_ALWAYS;

    return s;
}

/* Member parameter serializer */
static corto_walk_opt c_apiAssignSerializer(void) {
    corto_walk_opt s;

    corto_walk_init(&s);
    s.metaprogram[CORTO_MEMBER] = c_apiAssignMember;
    s.access = CORTO_LOCAL|CORTO_READONLY|CORTO_PRIVATE|CORTO_HIDDEN;
    s.accessKind = CORTO_NOT;
    s.optionalAction = CORTO_WALK_OPTIONAL_ALWAYS;

    return s;
}

corto_int16 c_apiTypeInitVoid(corto_type t, c_apiWalk_t *data) {
    CORTO_UNUSED(t);

    if (data->parameterCount) {
        g_fileWrite(data->header, ", ");
        g_fileWrite(data->source, ", ");
    } else {
        data->parameterCount += 1;
    }
    g_fileWrite(data->header, "corto_object value");
    g_fileWrite(data->source, "corto_object value");
    c_apiCastMacroAddArg(data->args, "value", corto_object_o, FALSE);
    return 0;
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
    c_apiCastMacroAddArg(data->args, "type", corto_type(corto_type_o), FALSE);
    c_apiCastMacroAddArg(data->args, "value", NULL, FALSE);
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
    c_apiCastMacroAddArg(data->args, "value", NULL, FALSE);
    return 0;
}

corto_int16 c_apiType_init_composite(corto_type t, c_apiWalk_t *data) {
    corto_walk_opt s = c_apiParamSerializer();
    corto_metawalk(&s, corto_type(t), data);

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

corto_int16 c_apiType_init_compositeMember(
    corto_member m,
    c_apiWalk_t *data)
{
    corto_walk_opt s = c_apiParamSerializer();
    corto_value info = corto_value_member(
        NULL,
        m,
        NULL
    );

    s.metaprogram[CORTO_MEMBER](&s, &info, data);

    return 0;
}


corto_int16 c_apiType_init_collection(corto_type t, c_apiWalk_t *data) {
    corto_id elementId;
    bool prefix;

    if (data->parameterCount) {
        g_fileWrite(data->header, ", ");
        g_fileWrite(data->source, ", ");
    } else {
        data->parameterCount += 2;
    }

    c_specifierId(data->g, corto_collection(t)->elementType, elementId, &prefix, NULL);

    g_fileWrite(data->header, "corto_uint32 length, %s* elements", elementId);
    g_fileWrite(data->source, "corto_uint32 length, %s* elements", elementId);
    c_apiCastMacroAddArg(data->args, "length", NULL, FALSE);
    c_apiCastMacroAddArgMove(data->args, "elements", NULL, FALSE);

    return 0;
}

/* Add initializer arguments to function declaration for type */
corto_int16 c_apiTypeInitArgs(corto_type t, corto_member m, c_apiWalk_t *data) {
    corto_int16 result = 0;

    switch(t->kind) {
    case CORTO_VOID:
        if (t->reference) {
            result = c_apiTypeInitVoid(t, data);
        }
        break;
    case CORTO_ITERATOR:
        break;
    case CORTO_ANY:
        result = c_apiTypeInitAny(t, data);
        break;
    case CORTO_PRIMITIVE:
        result = c_apiTypeInitPrimitive(t, data);
        break;
    case CORTO_COMPOSITE:
        if (!m) {
            result = c_apiType_init_composite(t, data);
        } else {
            corto_id dId;
            g_fullOid(data->g, corto_union(t)->discriminator, dId);
            if (data->parameterCount) {
                g_fileWrite(data->header, ", ");
                g_fileWrite(data->source, ", ");
            }
            g_fileWrite(data->header, "%s _d", dId);
            g_fileWrite(data->source, "%s _d", dId);
            c_apiCastMacroAddArg(data->args, "_d", corto_union(t)->discriminator, FALSE);
            data->parameterCount ++;
            result = c_apiType_init_compositeMember(m, data);
        }
        break;
    case CORTO_COLLECTION:
        result = c_apiType_init_collection(t, data);
        break;
    }

    return result;
}

/* Assign void reference */
corto_int16 c_apiTypeAssignVoid(corto_type t, corto_string _this, c_apiWalk_t *data) {
    CORTO_UNUSED(t);
    g_fileWrite(data->source, "corto_set_ref(%s, value);\n", _this);
    return 0;
}

/* Assign any */
corto_int16 c_apiTypeAssignAny(corto_type t, corto_string _this, c_apiWalk_t *data) {
    CORTO_UNUSED(t);
    g_fileWrite(data->source, "corto_any v;\n");
    g_fileWrite(data->source, "v.value = value;\n");
    g_fileWrite(data->source, "v.type = type;\n");
    g_fileWrite(data->source, "%s->owner = TRUE;\n", _this);
    g_fileWrite(data->source, "corto_ptr_copy(%s, corto_any_o, &v);\n", _this);
    return 0;
}

/* Assign primitve */
corto_int16 c_apiTypeAssignPrimitive(corto_type t, corto_string _this, c_apiWalk_t *data) {
    c_apiAssign(t, TRUE, 0, _this, "value", data);
    return 0;
}

/* Assign composite */
corto_int16 c_apiTypeAssignComposite(corto_type t, corto_string _this, c_apiWalk_t *data) {
    corto_walk_opt s = c_apiAssignSerializer();
    corto_id id;

    g_fullOid(data->g, t, id);

    data->_this = _this;
    corto_metawalk(&s, corto_type(t), data);

    if (corto_instanceof(corto_procedure_o, t)) {
        g_fileWrite(data->source, "corto_function(%s)->kind = CORTO_PROCEDURE_CDECL;\n", _this);
        g_fileWrite(data->source, "corto_function(%s)->fptr = (corto_word)_impl;\n", _this);
    }

    return 0;
}

/* Assign composite */
corto_int16 c_apiTypeAssignCompositeMember(corto_member m, corto_string _this, c_apiWalk_t *data) {
    corto_walk_opt s = c_apiAssignSerializer();
    corto_int32 i;
    corto_value info = corto_value_member(
        NULL,
        m,
        NULL
    );

    data->_this = _this;

    /* Assign single member for unions */
    corto_id varId;
    c_varId(data->g, corto_parentof(m), varId);
    g_fileWrite(data->source, "if (_d != %s->d) corto_ptr_deinit(_this, %s);\n", _this, varId);
    g_fileWrite(data->source, "switch(_d) {\n");
    if (corto_case(m)->discriminator.length) {
        for(i = 0; i < corto_case(m)->discriminator.length; i++) {
            g_fileWrite(data->source, "case %d:\n", corto_case(m)->discriminator.buffer[i]);
        }

        g_fileIndent(data->source);

        if (!(m->modifiers & CORTO_OPTIONAL)) {
            g_fileWrite(data->source, "if (%s->d != _d) {\n", _this);
            g_fileIndent(data->source);
            c_varId(data->g, m->type, varId);
            g_fileWrite(data->source, "corto_ptr_init(&%s->is, %s);\n", _this, varId);
            g_fileDedent(data->source);
            g_fileWrite(data->source, "}\n");
        }
        g_fileWrite(data->source, "%s->d = _d;\n", _this);

        s.metaprogram[CORTO_MEMBER](&s, &info, data);
        g_fileWrite(data->source, "break;\n");
        g_fileDedent(data->source);

        g_fileWrite(data->source, "default:\n");
        g_fileIndent(data->source);
        g_fileWrite(data->source,
          "corto_critical(\"invalid discriminator %%d for field %s\", _d);\n",
          corto_idof(m));
        g_fileWrite(data->source, "break;\n");
        g_fileDedent(data->source);
    } else {
        corto_uint32 caseCount = 0;

        /* If this is for assigning default member, ensure that the
         * discriminator is not set to one of the other cases */
        corto_union u = corto_union(corto_parentof(m));
        corto_uint32 memberId, caseId;
        for (memberId = 0; memberId < corto_interface(u)->members.length; memberId++) {
            corto_case c = corto_case(corto_interface(u)->members.buffer[memberId]);
            for (caseId = 0; caseId < c->discriminator.length; caseId ++) {
                g_fileWrite(data->source, "case %d:\n", c->discriminator.buffer[caseId]);
                caseCount ++;
            }
        }

        if (caseCount) {
            g_fileIndent(data->source);
            g_fileWrite(data->source,
              "corto_critical(\"invalid discriminator %%d for field %s\", _d);\n",
              corto_idof(m));
            g_fileWrite(data->source, "break;\n");
            g_fileDedent(data->source);
        }

        g_fileWrite(data->source, "default:\n");
        g_fileIndent(data->source);
        g_fileWrite(data->source, "%s->d = _d;\n", _this);
        s.metaprogram[CORTO_MEMBER](&s, &info, data);
        g_fileWrite(data->source, "break;\n");
        g_fileDedent(data->source);
    }

    g_fileWrite(data->source, "}\n");

    return 0;
}

/* Assign collection */
corto_int16 c_apiTypeAssignCollection(corto_type t, corto_string _this, c_apiWalk_t *data) {
    corto_id id, cVar, lvalue;
    corto_type elementType = corto_collection(t)->elementType;

    g_fullOid(data->g, t, id);

    strcpy(cVar, _this);

    switch(corto_collection(t)->kind) {
    case CORTO_SEQUENCE:
        strcat(cVar, "->buffer");
        g_fileWrite(data->source, "%s_resize(%s, length);\n", id, _this);
    case CORTO_ARRAY:
        g_fileWrite(data->source, "corto_uint32 i = 0;\n");
        g_fileWrite(data->source, "for (i = 0; i < length; i ++) {\n");
        g_fileIndent(data->source);
        sprintf(lvalue, "%s[i]", cVar);

        /* c_apiAssign expects value as parameter while elements[i] points to
         * the value directly. Compensate with a & */
        if (c_typeRequiresPtr(elementType)) {
            c_apiAssign(elementType, FALSE, 0, lvalue, "&elements[i]", data);
        } else {
            c_apiAssign(elementType, FALSE, 0, lvalue, "elements[i]", data);
        }
        g_fileDedent(data->source);
        g_fileWrite(data->source, "}\n");
        break;
    case CORTO_LIST:
        g_fileWrite(data->source, "corto_uint32 i = 0;\n");
        g_fileWrite(data->source, "%s_clear(*%s);\n", id, cVar);
        g_fileWrite(data->source, "for (i = 0; i < length; i ++) {\n");
        g_fileIndent(data->source);

        /* Append expects value as regular parameter while elements[i] points to
         * the value directly. Compensate with a & */
        if (c_typeRequiresPtr(elementType)) {
            g_fileWrite(data->source, "%s_append(*%s, &elements[i]);\n", id, cVar);
        } else {
            g_fileWrite(data->source, "%s_append(*%s, elements[i]);\n", id, cVar);
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
corto_int16 c_apiTypeInitAssign(corto_type t, corto_string _this, corto_member m, c_apiWalk_t *data) {
    corto_int16 result = 0;

    switch(t->kind) {
    case CORTO_VOID:
        if (t->reference) {
            result = c_apiTypeAssignVoid(t, _this, data);
        }
        break;
    case CORTO_ITERATOR:
        break;
    case CORTO_ANY:
        result = c_apiTypeAssignAny(t, _this, data);
        break;
    case CORTO_PRIMITIVE:
        result = c_apiTypeAssignPrimitive(t, _this, data);
        break;
    case CORTO_COMPOSITE:
        if (!m) {
            result = c_apiTypeAssignComposite(t, _this, data);
        } else {
            result = c_apiTypeAssignCompositeMember(m, _this, data);
        }
        break;
    case CORTO_COLLECTION:
        result = c_apiTypeAssignCollection(t, _this, data);
        break;
    }

    return result;
}

void c_apiLocalDefinition(corto_type t, c_apiWalk_t *data, char *func, char *id) {
    corto_id localId, buildingMacro;

    c_buildingMacro(data->g, buildingMacro);
    g_localOid(data->g, t, localId);

    if (strcmp(g_getAttribute(data->g, "bootstrap"), "true") && corto_parentof(t) != root_o) {
        g_fileWrite(data->header, "\n");
        g_fileWrite(data->header, "#if defined(%s) && !defined(__cplusplus)\n", buildingMacro);
        g_fileWrite(data->header, "#define %s%s %s%s\n", localId, func, id, func);
        g_fileWrite(data->header, "#endif\n");
    }
}

corto_int16 c_apiTypeCreateIntern(
    corto_type t,
    c_apiWalk_t *data,
    corto_string func,
    corto_bool scoped,
    corto_bool define)
{
    corto_id id, ret;
    g_generator g = data->g;
    corto_member member = NULL;
    corto_uint32 memberCount = 0;

    /* AND with define, because if function doesn't define the object the code
     * doesn't take into account individual members */
    corto_bool isUnion = corto_class_instanceof(corto_union_o, t) && define;

    g_fullOid(data->g, t, id);
    c_typeret(g, t, C_ByReference, ret);

    do {
        if (isUnion) {
            member = corto_interface(t)->members.buffer[memberCount];
            memberCount ++;
        }

        /* Collect arguments for casting macro */
        data->args = corto_ll_new();
        data->parameterCount = 0;

        /* Function declaration */
        c_apiLocalDefinition(t, data, func, id);

        c_writeExport(data->g, data->header);
        g_fileWrite(data->header, " %s _%s%s%s%s(",
          ret, id, func, member ? "_" : "", member ? corto_idof(member) : "");

        /* Function implementation */
        g_fileWrite(data->source, "%s _%s%s%s%s(",
          ret, id, func, member ? "_" : "", member ? corto_idof(member) : "");

        if (scoped) {
            g_fileWrite(data->header, "corto_object _parent, const char *_id");
            g_fileWrite(data->source, "corto_object _parent, const char *_id");
            c_apiCastMacroAddArg(data->args, "_parent", corto_object_o, FALSE);
            c_apiCastMacroAddArg(data->args, "_id", corto_type(corto_string_o), FALSE);
            data->parameterCount = 2;
        }

        if (define) {
            c_apiTypeInitArgs(t, member, data);
        }

        if (!data->parameterCount) {
            g_fileWrite(data->header, "void");
            g_fileWrite(data->source, "void");
        }

        g_fileWrite(data->header, ");\n");
        g_fileWrite(data->source, ") {\n");

        /* Write casting macro */
        c_apiCastMacro(id, func, member, data);
        c_apiCastAuto(t, scoped, func, member, data);
        c_apiCastMacroFreeArgs(data->args);
        data->args = NULL;

        g_fileIndent(data->source);
        g_fileWrite(data->source, "%s _this;\n", ret);
        if (scoped) {
            g_fileWrite(
              data->source,
              "_this = (%s)corto_declare(_parent, _id, %s_o);\n", ret, id);
        } else {
            g_fileWrite(
              data->source,
              "_this = (%s)corto_declare(NULL, NULL, %s_o);\n", ret, id);
        }

        g_fileWrite(data->source, "if (!_this) {\n");
        g_fileIndent(data->source);
        g_fileWrite(data->source, "return NULL;\n");
        g_fileDedent(data->source);
        g_fileWrite(data->source, "}\n");

        if (define) {
            g_fileWrite(data->source, "if (!corto_check_state(_this, CORTO_VALID)) {\n");
            g_fileIndent(data->source);

            /* Assignments */
            data->owned = TRUE;
            c_apiTypeInitAssign(t, "_this", member, data);

            if (t->kind != CORTO_VOID || t->reference) {
                /* Define object */
                g_fileWrite(data->source, "if (corto_define(_this)) {\n");
                g_fileIndent(data->source);
                g_fileWrite(data->source, "corto_release(_this);\n");
                g_fileWrite(data->source, "_this = NULL;\n");
                g_fileDedent(data->source);
                g_fileWrite(data->source, "}\n");
            }

            g_fileDedent(data->source);
            g_fileWrite(data->source, "}\n");
        }

        g_fileWrite(data->source, "return _this;\n");
        g_fileDedent(data->source);
        g_fileWrite(data->source, "}\n\n");

    } while (isUnion && (memberCount < corto_interface(t)->members.length));

    return 0;
}

corto_int16 c_apiTypeCreateChild(corto_type t, c_apiWalk_t *data) {
    return c_apiTypeCreateIntern(t, data, "_create", TRUE, TRUE);
}

corto_int16 c_apiTypeDeclareChild(corto_type t, c_apiWalk_t *data) {
    return c_apiTypeCreateIntern(t, data, "_declare", TRUE, FALSE);
}

corto_int16 c_apiTypeDefineIntern(corto_type t, c_apiWalk_t *data, corto_bool isUpdate, corto_bool doUpdate) {
    corto_id id, ptr;
    corto_string func = isUpdate ? doUpdate ? "_update" : "_assign" : "_define";
    corto_bool isSet = isUpdate ? doUpdate ? FALSE : TRUE : FALSE;
    g_generator g = data->g;
    corto_bool isUnion = corto_class_instanceof(corto_union_o, t);
    corto_member member = NULL;
    corto_uint32 memberCount = 0;

    do {
        if (isUnion) {
            member = corto_interface(t)->members.buffer[memberCount];
            memberCount ++;
        }

        data->args = corto_ll_new();
        data->parameterCount = 1;
        c_typeId(data->g, t, id);

        c_apiLocalDefinition(t, data, func, id);

        c_writeExport(data->g, data->header);

        if (isUpdate && !doUpdate) {
            g_fileWrite(data->header, " %s ", c_typeret(g, t, C_ByReference, ptr));
            g_fileWrite(data->source, "%s ", c_typeret(g, t, C_ByReference, ptr));
        } else {
            g_fileWrite(data->header, " corto_int16 ");
            g_fileWrite(data->source, "corto_int16 ");
        }

        g_fileWrite(data->header, "_%s%s%s%s(%s _this",
            id, func, member ? "_" : "", member ? corto_idof(member) : "", c_typeptr(g, t, ptr));
        g_fileWrite(data->source, "_%s%s%s%s(%s _this",
            id, func, member ? "_" : "", member ? corto_idof(member) : "", c_typeptr(g, t, ptr));

        if (strcmp(func, "Assign")) {
            c_apiCastMacroAddThis(data->args, "_this", t, FALSE, TRUE, g);
        } else {
            c_apiCastMacroAddThis(data->args, "_this", t, FALSE, FALSE, g);
        }

        c_apiTypeInitArgs(t, member, data);

        /* Write closing brackets for argumentlist in source and header */
        g_fileWrite(data->header, ");\n");
        g_fileWrite(data->source, ") {\n");
        g_fileIndent(data->source);
        g_fileWrite(data->source, "CORTO_UNUSED(_this);\n");

        /* Write cast macro */
        if (!isSet || isUnion) {
            c_apiCastMacro(id, func, member, data);
        } else {
            c_apiCastMacroSet(id, t, data);
        }
        c_apiCastMacroFreeArgs(data->args);
        data->args = NULL;

        if (isUpdate && doUpdate && (t->kind != CORTO_VOID || t->reference)) {
            g_fileWrite(data->source, "if (!corto_update_begin(_this)) {\n");
            g_fileIndent(data->source);

            g_fileWrite(data->source, "if ((corto_typeof(corto_typeof(_this)) == (corto_type)corto_target_o) && !corto_owned(_this)) {\n");
            g_fileIndent(data->source);
            corto_id thisVar;
            sprintf(thisVar, "((%s)CORTO_OFFSET(_this, ((corto_type)%s_o)->size))",
                c_typeret(g, t, C_ByReference, ptr), id);
            data->owned = FALSE;
            c_apiTypeInitAssign(t, thisVar, member, data);
            g_fileDedent(data->source);
            g_fileWrite(data->source, "} else {\n");
            g_fileIndent(data->source);
            data->owned = TRUE;
            c_apiTypeInitAssign(t, "_this", member, data);
            g_fileDedent(data->source);
            g_fileWrite(data->source, "}\n");
        } else {
            data->owned = TRUE;
            c_apiTypeInitAssign(t, "_this", member, data);
        }

        /* Define object */
        if (isUpdate && doUpdate) {
            if (t->kind != CORTO_VOID || t->reference) {
                g_fileWrite(data->source, "if (corto_update_end(_this)) {\n");
                g_fileIndent(data->source);
                g_fileWrite(data->source, "return -1;\n");
                g_fileDedent(data->source);
                g_fileWrite(data->source, "}\n");
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
        } else if (isUpdate && !doUpdate) {
            g_fileWrite(data->source, "return _this;\n");
        }

        g_fileDedent(data->source);
        g_fileWrite(data->source, "}\n\n");
    } while (isUnion && (memberCount < corto_interface(t)->members.length));

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

corto_int16 c_apiDelegate_init_callbackAuto(
    corto_delegate t,
    corto_bool instance,
    c_apiWalk_t *data)
{
    corto_id id;

    g_fullOid(data->g, t, id);

    if (!instance) {
        g_fileWrite(data->header, "#define %s_init_c_auto(d, callback) ", id);
        g_fileWrite(data->header, "%s d; %s_init_c(&d, callback)\n", id, id);
    } else {
        g_fileWrite(
            data->header,
            "#define %s_init_c_instance_auto(d, instance, callback)",
            id);
        g_fileWrite(
            data->header,
            "%s d; %s_init_c_instance(&d, instance, callback)\n", id, id);
    }

    return 0;
}

corto_int16 c_apiDelegateInitCallback(
    corto_delegate t,
    corto_bool instance,
    c_apiWalk_t *data)
{
    corto_id returnId, id, paramId, returnVarId;
    int i, firstComma = 0;

    if (c_apiDelegate_init_callbackAuto(t, instance, data)) {
        goto error;
    }

    g_fullOid(data->g, t, id);
    g_fullOid(data->g, t->returnType, returnId);
    c_varId(data->g, t->returnType, returnVarId);

    c_writeExport(data->g, data->header);
    if (!instance) {
        g_fileWrite(data->header, " corto_int16 %s_init_c(%s *d, ", id, id);
        g_fileWrite(data->source, "corto_int16 %s_init_c(%s *d, ", id, id);
    } else {
        g_fileWrite(
            data->header,
            " corto_int16 %s_init_c_instance(%s *d, corto_object instance, ",
            id, id);

        g_fileWrite(
            data->source,
            "corto_int16 %s_init_c_instance(%s *d, corto_object instance, ",
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
        g_fileWrite(data->source, "d->super.instance = instance;\n");
        g_fileWrite(data->source, "corto_claim(instance);\n");
    } else {
        g_fileWrite(data->source, "d->super.instance = NULL;\n");
    }
    g_fileWrite(data->source, "d->super.procedure = corto_declare(NULL, NULL, corto_function_o);\n");
    g_fileWrite(data->source, "d->super.procedure->kind = CORTO_PROCEDURE_CDECL;\n", id);
    g_fileWrite(data->source, "corto_set_ref(&d->super.procedure->returnType, %s);\n", returnVarId);
    g_fileWrite(
        data->source, "corto_function_parseParamString(d->super.procedure, \"(");

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
        data->source, "d->super.procedure->fptr = (corto_word)callback;\n");

    g_fileWrite(data->source, "corto_define(d->super.procedure);\n");
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

    g_fileWrite(data->header, "corto_int16 %s_call(%s *_delegate", id, id);
    g_fileWrite(data->source, "corto_int16 %s_call(%s *_delegate", id, id);

    if (hasReturn) {
        g_fileWrite(data->header, ", %s* _result", returnId);
        g_fileWrite(data->source, ", %s* _result", returnId);
    }

    corto_int32 i;
    for (i = 0; i < t->parameters.length; i++) {
        corto_parameter *p = &t->parameters.buffer[i];
        c_paramType(data->g, p, paramType);
        g_id(data->g, p->name, paramName);

        g_fileWrite(data->header, ", %s %s", paramType, paramName);
        g_fileWrite(data->source, ", %s %s", paramType, paramName);
    }

    g_fileWrite(data->header, ");\n");
    g_fileWrite(data->source, ") {\n");
    g_fileIndent(data->source);

    g_fileWrite(data->source, "if (_delegate->super.procedure) {\n");
    g_fileIndent(data->source);
    g_fileWrite(data->source, "if (_delegate->super.instance) {\n");
    g_fileIndent(data->source);
    if (hasReturn) {
        g_fileWrite(data->source, "corto_invoke(_delegate->super.procedure, _result, _delegate->super.instance");
    } else {
        g_fileWrite(data->source, "corto_invoke(_delegate->super.procedure, NULL, _delegate->super.instance");
    }
    for (i = 0; i < t->parameters.length; i++) {
        corto_parameter *p = &t->parameters.buffer[i];
        g_fullOid(data->g, p->type, paramType);
        g_id(data->g, p->name, paramName);
        g_fileWrite(data->source, ", %s", paramName);
    }
    g_fileWrite(data->source, ");\n");
    g_fileDedent(data->source);
    g_fileWrite(data->source, "} else {\n");
    g_fileIndent(data->source);
    if (hasReturn) {
        g_fileWrite(data->source, "corto_invoke(_delegate->super.procedure, _result");
    } else {
        g_fileWrite(data->source, "corto_invoke(_delegate->super.procedure, NULL");
    }
    for (i = 0; i < t->parameters.length; i++) {
        corto_parameter *p = &t->parameters.buffer[i];
        g_id(data->g, p->name, paramName);
        g_fileWrite(data->source, ", %s", paramName);
    }
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
