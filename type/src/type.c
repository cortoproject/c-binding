/*
 * c_type.c
 *
 *  Created on: Sep 17, 2012
 *      Author: sander
 */

#include "type.h"
#include <driver.gen.c.common>

typedef struct c_typeWalk_t {
    g_generator g;
    g_file header;
    bool prefixComma; /* For printing members and constants */
    bool declareAndDefine;
} c_typeWalk_t;

/* Enumeration constant */
static int16_t c_typeConstant(corto_walk_opt* s, corto_value* v, void* userData) {
    c_typeWalk_t* data;
    corto_id constantId;

    CORTO_UNUSED(s);

    data = userData;

    switch(corto_primitive(corto_parentof(v->is.constant.constant))->kind) {
    case CORTO_ENUM:
        if (data->prefixComma) {
            g_fileWrite(data->header, ",\n");
        } else {
            data->prefixComma = TRUE;
        }
        g_fileWrite(
            data->header,
            "%s = %d",
            c_constantId(data->g, v->is.constant.constant, constantId),
            *v->is.constant.constant);
        break;
    case CORTO_BITMASK:
        g_fileWrite(
            data->header,
            "#define %s (0x%x)\n",
            c_constantId(data->g, v->is.constant.constant, constantId),
            *v->is.constant.constant);
        break;
    default:
        ut_throw("c_typeConstant: invalid constant parent-type.");
        goto error;
        break;
    }

    return 0;
error:
    return -1;
}

/* Member */
static int16_t c_typeMember(corto_walk_opt* s, corto_value* v, void* userData) {
    c_typeWalk_t* data;
    corto_member m;
    corto_id specifier, postfix, memberId;

    CORTO_UNUSED(s);

    if (v->kind == CORTO_MEMBER) {
        data = userData;
        m = v->is.member.member;

        /* Get typespecifier */
        if (c_specifierId(data->g, m->type, specifier, NULL, postfix)) {
            goto error;
        }
        bool isPtr = (m->modifiers & CORTO_OPTIONAL) || (!m->type->reference && m->modifiers & CORTO_OBSERVABLE);
        if (m->id != (corto_uint32)-1) {
            g_fileWrite(data->header, "%s %s%s%s;\n",
              specifier,
              isPtr ? "*" : "",
              g_id(data->g, corto_idof(m), memberId),
              postfix);
        } else {
            g_fileWrite(data->header, "%s super%s;\n", specifier, postfix);
        }
    }

    return 0;
error:
    return -1;
}


/* Enumeration object */
static int16_t c_typePrimitiveEnum(corto_walk_opt* s, corto_value* v, void* userData) {
    c_typeWalk_t* data;
    corto_enum t;
    corto_id id;

    CORTO_UNUSED(s);

    data = userData;
    t = corto_enum(corto_value_typeof(v));

    /* Write enumeration */
    g_fileWrite(data->header, "typedef enum %s {\n", c_typeId(data->g, t, id));
    g_fileIndent(data->header);

    /* Write enumeration constants */
    if (corto_walk_constants(s, v, userData)) {
        goto error;
    }

    g_fileDedent(data->header);
    g_fileWrite(data->header, "\n");
    g_fileWrite(data->header, "} %s;\n", c_typeId(data->g, t, id));

    return 0;
error:
    return -1;
}

/* Bitmask object */
static int16_t c_typePrimitiveBitmask(corto_walk_opt* s, corto_value* v, void* userData) {
    c_typeWalk_t* data;
    corto_enum t;
    corto_id id;

    CORTO_UNUSED(s);

    data = userData;
    t = corto_enum(corto_value_typeof(v));

    g_fileWrite(data->header, "typedef uint32_t %s;\n", c_typeId(data->g, t, id));

    /* Write enumeration constants */
    g_fileIndent(data->header);
    if (corto_walk_constants(s, v, userData)) {
        goto error;
    }
    g_fileDedent(data->header);

    return 0;
error:
    return -1;
}

/* Void object */
static int16_t c_typeVoid(corto_walk_opt* s, corto_value* v, void* userData) {
    corto_type t;
    c_typeWalk_t* data;
    corto_id id;

    CORTO_UNUSED(s);

    t = corto_value_typeof(v);
    data = userData;

    g_fileWrite(data->header, "/* %s */\n", corto_fullpath(NULL, t));
    if (t->reference) {
        g_fileWrite(data->header, "typedef void *%s;\n", c_typeId(data->g, t, id));
    } else {
        g_fileWrite(data->header, "typedef void %s;\n", c_typeId(data->g, t, id));
    }

    return 0;
}

/* Void object */
static int16_t c_typeAny(corto_walk_opt* s, corto_value* v, void* userData) {
    corto_type t;
    c_typeWalk_t* data;
    corto_id id;

    CORTO_UNUSED(s);

    t = corto_value_typeof(v);
    data = userData;

    g_fileWrite(data->header, "/* %s */\n", corto_fullpath(NULL, t));
    c_typeId(data->g, t, id);
    g_fileWrite(data->header,
        "typedef struct %s {corto_type type; void *value; uint8_t owner;} %s;\n", id, id);

    return 0;
}

/* Primitive object */
static int16_t c_typePrimitive(corto_walk_opt* s, corto_value* v, void* userData) {
    corto_char buff[16];
    corto_type t;
    corto_id id;
    c_typeWalk_t* data;

    CORTO_UNUSED(s);

    data = userData;
    t = corto_value_typeof(v);

    /* Obtain platform type-name for primitive */
    switch(corto_primitive(t)->kind) {
    case CORTO_ENUM:
        g_fileWrite(
            data->header, "/* %s %s */\n",
            corto_fullpath(NULL, corto_typeof(t)),
            corto_path(NULL, root_o, t, "/"));
        if (c_typePrimitiveEnum(s, v, userData)) {
            goto error;
        }
        break;
    case CORTO_BITMASK:
        g_fileWrite(
            data->header, "/* %s %s */\n",
            corto_fullpath(NULL, corto_typeof(t)),
            corto_path(NULL, root_o, t, "/"));
        if (c_typePrimitiveBitmask(s, v, userData)) {
            goto error;
        }
        break;
    default:
        if (!c_primitiveId(data->g, corto_primitive(t), buff)) {
            goto error;
        }

        /* Write typedef */
        g_fileWrite(
            data->header, "/* %s %s */\n",
            corto_fullpath(NULL, corto_typeof(t)),
            corto_path(NULL, root_o, t, "/"));
        g_fileWrite(data->header, "typedef %s %s%s;\n",
            buff,
            t->reference ? "*" : "",
            c_typeId(data->g, t, id));
        break;
    }

    return 0;
error:
    return -1;
}

/* Struct object */
static int16_t c_typeStruct(corto_walk_opt* s, corto_value* v, void* userData) {
    c_typeWalk_t* data;
    corto_id id, type_id;
    corto_type t;

    data = userData;
    t = corto_value_typeof(v);

    g_fileWrite(
        data->header, "/* %s %s */\n",
        corto_fullpath(NULL, corto_typeof(t)),
        corto_path(NULL, root_o, t, "/"));

    if (data->declareAndDefine) {
        g_fileWrite(data->header, "typedef ");
    }

    c_typeId(data->g, t, type_id);

    /* Open struct */
    if (t->reference) {
        g_fileWrite(data->header, "struct %s_s {\n", type_id);
    } else {
        g_fileWrite(data->header, "struct %s {\n", type_id);
    }

    /* Serialize members */
    g_fileIndent(data->header);

    /* Write base */
    if (corto_interface(t)->base && corto_type(corto_interface(t)->base)->alignment) {
        if (corto_type(corto_interface(t)->base)->reference) {
            g_fileWrite(data->header, "struct %s_s super;\n", c_typeId(data->g, corto_interface(t)->base, id));
        } else {
            g_fileWrite(data->header, "%s super;\n", c_typeId(data->g, corto_interface(t)->base, id));
        }
    }

    if (corto_walk_members(s, v, userData)) {
        goto error;
    }
    g_fileDedent(data->header);

    /* Close struct */
    g_fileWrite(data->header, "}");

    if (data->declareAndDefine) {
        g_fileWrite(data->header, " ");
        if (t->reference) {
            g_fileWrite(data->header, "*");
        }
        g_fileWrite(data->header, "%s", type_id);
    }
    g_fileWrite(data->header, ";\n");

    return 0;
error:
    return -1;
}

/* Union object */
static int16_t c_typeUnion(corto_walk_opt* s, corto_value* v, void* userData) {
    c_typeWalk_t* data;
    corto_id id;
    corto_type t;

    data = userData;
    t = corto_value_typeof(v);

    if (data->declareAndDefine) {
        g_fileWrite(data->header, "typedef ");
    }

    /* Open struct */
    g_fileWrite(data->header, "struct %s {\n", c_typeId(data->g, t, id));
    g_fileIndent(data->header);

    /* Write discriminator */
    g_fileWrite(data->header, "%s d;\n", c_typeId(data->g, corto_union(t)->discriminator, id));

    /* Open union */
    g_fileWrite(data->header, "union {\n");
    g_fileIndent(data->header);

    if (corto_walk_cases(s, v, userData)) {
        goto error;
    }

    /* Close union */
    g_fileDedent(data->header);
    g_fileWrite(data->header, "} is;\n");

    /* Close struct */
    g_fileDedent(data->header);
    g_fileWrite(data->header, "}");

    if (data->declareAndDefine) {
        g_fileWrite(data->header, " %s", c_typeId(data->g, t, id));
    }
    g_fileWrite(data->header, ";\n");

    return 0;
error:
    return -1;
}

/* Abstract object */
static int16_t c_typeAbstract(corto_walk_opt* s, corto_value* v, void* userData) {
    CORTO_UNUSED(s);
    CORTO_UNUSED(v);
    CORTO_UNUSED(userData);

    return 0;
}

/* Composite object */
static int16_t c_typeComposite(corto_walk_opt* s, corto_value* v, void* userData) {
    corto_type t;

    t = corto_value_typeof(v);
    switch(corto_interface(t)->kind) {
    case CORTO_UNION:
        if (c_typeUnion(s, v, userData)) {
            goto error;
        }
        break;
    case CORTO_INTERFACE:
        if (c_typeAbstract(s, v, userData)) {
            goto error;
        }
        break;
    case CORTO_PROCEDURE:
    case CORTO_CLASS:
    case CORTO_STRUCT:
    case CORTO_DELEGATE:
        if (c_typeStruct(s, v, userData)) {
            goto error;
        }
        break;
    }

    return 0;
error:
    return -1;
}

/* Array object */
static int16_t c_typeArray(corto_walk_opt* s, corto_value* v, void* userData) {
    corto_type t;
    c_typeWalk_t* data;
    corto_id id, id3, postfix, postfix2;

    CORTO_UNUSED(s);
    CORTO_UNUSED(v);

    data = userData;
    t = corto_value_typeof(v);
    c_specifierId(data->g, corto_type(t), id, NULL, postfix);
    c_specifierId(data->g, corto_type(corto_collection(t)->element_type), id3, NULL, postfix2);
    g_fileWrite(data->header, "typedef %s %s[%d];\n",
            id3,
            id,
            corto_collection(t)->max);

    return 0;
}

/* Sequence object */
static
int16_t c_typeSequence(
    corto_walk_opt* s,
    corto_value* v,
    void* userData)
{
    corto_type t;
    c_typeWalk_t* data;
    corto_id id, id3, postfix, postfix2;

    CORTO_UNUSED(s);
    CORTO_UNUSED(v);

    data = userData;
    t = corto_value_typeof(v);
    c_specifierId(data->g, corto_type(t), id, NULL, postfix);
    c_specifierId(data->g, corto_type(corto_collection(t)->element_type), id3, NULL, postfix2);

    int len = strlen(id);
    bool protect = false;

    if (!strcmp(&id[len - 3], "Seq")) {
        protect = true;
    }

    if (protect) {
        g_fileWrite(data->header, "#ifndef %s_DEFINED\n", id);
        g_fileWrite(data->header, "#define %s_DEFINED\n", id);
    }

    g_fileWrite(data->header, "typedef struct %s {uint32_t length; %s *buffer;} %s;\n",
        id, id3, id);

    if (protect) {
        g_fileWrite(data->header, "#endif\n");
    }

    return 0;
}

/* List object */
static int16_t c_typeList(corto_walk_opt* s, corto_value* v, void* userData) {
    corto_type t;
    c_typeWalk_t* data;
    corto_id id, postfix;

    CORTO_UNUSED(s);
    CORTO_UNUSED(v);

    data = userData;
    t = corto_value_typeof(v);
    c_specifierId(data->g, corto_type(t), id, NULL, postfix);

    int len = strlen(id);
    bool protect = false;

    if (!strcmp(&id[len - 4], "List")) {
        protect = true;
    }

    if (protect) {
        g_fileWrite(data->header, "#ifndef %s_DEFINED\n", id);
        g_fileWrite(data->header, "#define %s_DEFINED\n", id);
    }

    g_fileWrite(data->header, "typedef ut_ll %s;\n",
            id);

    if (protect) {
        g_fileWrite(data->header, "#endif\n");
    }

    return 0;
}

/* Map object */
static int16_t c_typeMap(corto_walk_opt* s, corto_value* v, void* userData) {
    corto_type t;
    c_typeWalk_t* data;
    corto_id id, postfix;

    CORTO_UNUSED(s);
    CORTO_UNUSED(v);

    data = userData;
    t = corto_value_typeof(v);
    c_specifierId(data->g, corto_type(t), id, NULL, postfix);

    int len = strlen(id);
    bool protect = false;

    if (!strcmp(&id[len - 3], "Map")) {
        protect = true;
    }

    if (protect) {
        g_fileWrite(data->header, "#ifndef %s_DEFINED\n", id);
        g_fileWrite(data->header, "#define %s_DEFINED\n", id);
    }

    g_fileWrite(data->header, "typedef ut_rb %s;\n", id);

    if (protect) {
        g_fileWrite(data->header, "#endif\n");
    }

    return 0;
}

/* Collection object */
static int16_t c_typeCollection(corto_walk_opt* s, corto_value* v, void* userData) {
    corto_type t;

    t = corto_value_typeof(v);
    switch(corto_collection(t)->kind) {
    case CORTO_ARRAY:
        if (c_typeArray(s, v, userData)) {
            goto error;
        }
        break;
    case CORTO_SEQUENCE:
        if (c_typeSequence(s, v, userData)) {
            goto error;
        }
        break;
    case CORTO_LIST:
        if (c_typeList(s, v, userData)) {
            goto error;
        }
        break;
    case CORTO_MAP:
        if (c_typeMap(s, v, userData)) {
            goto error;
        }
        break;
    }

    return 0;
error:
    return -1;
}

/* Iterator object */
static int16_t c_typeIterator(corto_walk_opt* s, corto_value* v, void* userData) {
    corto_type t;

    CORTO_UNUSED(s);
    CORTO_UNUSED(v);

    c_typeWalk_t* data;
    corto_id id, postfix;

    data = userData;
    t = corto_value_typeof(v);
    c_specifierId(data->g, corto_type(t), id, NULL, postfix);

    int len = strlen(id);
    bool protect = false;

    if (!strcmp(&id[len - 4], "Iter")) {
        protect = true;
    }

    if (protect) {
        g_fileWrite(data->header, "#ifndef %s_DEFINED\n", id);
        g_fileWrite(data->header, "#define %s_DEFINED\n", id);
    }

    g_fileWrite(data->header, "typedef ut_iter %s;\n", id);

    if (protect) {
        g_fileWrite(data->header, "#endif\n");
    }

    return 0;
}

/* Type object */
static int16_t c_typeObject(corto_walk_opt* s, corto_value* v, void* userData) {
    c_typeWalk_t* data;
    corto_type t;
    int16_t result;

    data = userData;
    t = corto_type(corto_value_typeof(v));

    /* No type needs to be generated if type is a native type */
    if (corto_instanceof(corto_native_type_o, t)) {
        return 0;
    }

    /* Reset prefixComma */
    data->prefixComma = FALSE;

    /* Forward to specific type-functions */
    switch(t->kind) {
    case CORTO_VOID:
        result = c_typeVoid(s, v, userData);
        break;
    case CORTO_ANY:
        result = c_typeAny(s, v, userData);
        break;
    case CORTO_PRIMITIVE:
        result = c_typePrimitive(s, v, userData);
        break;
    case CORTO_COMPOSITE:
        result = c_typeComposite(s, v, userData);
        break;
    case CORTO_COLLECTION:
        result = c_typeCollection(s, v, userData);
        break;
    case CORTO_ITERATOR:
        result = c_typeIterator(s, v, userData);
        break;
    default:
        ut_throw("c_typeObject: typeKind '%s' not handled by code-generator.", corto_idof(corto_enum_constant_from_value(corto_typeKind_o, t->kind)));
        goto error;
    }

    g_fileWrite(data->header, "\n");

    return result;
error:
    return -1;
}

/* Metawalk-serializer for types */
static
corto_walk_opt c_typeSerializer(void) {
    corto_walk_opt s;

    /* Initialize serializer */
    corto_walk_init(&s);
    s.metaprogram[CORTO_OBJECT] = c_typeObject;
    s.metaprogram[CORTO_BASE] = c_typeMember;
    s.metaprogram[CORTO_MEMBER] = c_typeMember;
    s.metaprogram[CORTO_CONSTANT] = c_typeConstant;
    s.access = CORTO_GLOBAL;
    s.accessKind = CORTO_XOR;
    s.aliasAction = CORTO_WALK_ALIAS_IGNORE;
    s.optionalAction = CORTO_WALK_OPTIONAL_ALWAYS;
    s.traceKind = CORTO_WALK_TRACE_ON_FAIL;

    return s;
}

/* Print native type typedefs */
static
int c_typeNativeDefWalk(
    corto_object o,
    void* userData)
{
    c_typeWalk_t* data;

    data = userData;

    if (corto_instanceof(corto_native_type_o, o)) {
        char *nativeName = corto_native_type(o)->name;
        bool isPtr = nativeName[strlen(nativeName) - 1] == '*';
        corto_id id;
        strcpy(id, nativeName);
        if (isPtr) {
            id[strlen(id) - 1] = '\0';
        }

        if (corto_native_type(o)->is_ptr) {
            g_fileWrite(data->header, "#define %s void\n", id);
        } else {
            g_fileWrite(data->header, "#define %s void*\n", id);
        }
    }

    return 1;
}

static
int c_nativeCollect(
    corto_object o,
    void *userData)
{
    if (corto_instanceof(corto_native_type_o, o)) {
        ut_ll_append(userData, o);
    }
    return 0;
}

/* Open headerfile, write standard header. */
static
g_file c_typeHeaderFileOpen(
    g_generator g)
{
    g_file result;
    bool bootstrap = !strcmp(g_getAttribute(g, "bootstrap"), "true");

    /* Create file */
    result = c_headerOpen(g, "type");
    if (!result) {
        goto error;
    }

    /* Don't include this file when generating for the bootstrap */
    if (bootstrap) {
        if (g_getCurrent(g) == corto_lang_o) {
            g_fileWrite(result, "<bake.util>\n");
        } else {
            c_includeFrom(g, result, corto_lang_o, "_type.h");
        }
    } else {
        if (c_includeDependencies(g, result, "_type.h")) {
            g_fileWrite(result, "\n");
        }
    }

    g_fileWrite(result, "#ifdef __cplusplus\n");
    g_fileWrite(result, "extern \"C\" {\n");
    g_fileWrite(result, "#endif\n");

    return result;
error:
    return NULL;
}

/* Close headerfile */
static void c_typeHeaderFileClose(g_file file) {

    /* Print standard comments and includes */
    g_fileWrite(file, "#ifdef __cplusplus\n");
    g_fileWrite(file, "}\n");
    g_fileWrite(file, "#endif\n");
    c_headerClose(file);
}

static int c_typeDeclare(corto_object o, void* userData) {
    corto_id id, postfix;
    corto_type t;
    c_typeWalk_t* data;

    data = userData;
    t = o;
    c_specifierId(data->g, t, id, NULL, postfix);

    g_fileWrite(
        data->header, "/* %s %s */\n",
        corto_fullpath(NULL, corto_typeof(o)),
        corto_path(NULL, root_o, o, "/"));

    switch(t->kind) {
    case CORTO_COMPOSITE:
        switch(corto_interface(t)->kind) {
        case CORTO_DELEGATE:
        case CORTO_STRUCT:
        case CORTO_UNION:
        case CORTO_CLASS:
        case CORTO_PROCEDURE:
            if (t->reference) {
                g_fileWrite(data->header, "typedef struct %s_s *%s;\n\n", id, id);
            } else {
                g_fileWrite(data->header, "typedef struct %s %s;\n\n", id, id);
            }
            break;
        case CORTO_INTERFACE:
            g_fileWrite(data->header, "typedef void *%s;\n", id);
            break;
        }
        break;
    default:
        ut_throw("c_typeDeclare: only composite types can be forward declared.");
        goto error;
        break;
    }

    return 0;
error:
    return -1;
}

static int c_typeDefine(corto_object o, void* userData) {
    corto_walk_opt s;
    int result;

    result = 0;

    /* Get type-serializer */
    s = c_typeSerializer();

    /* Do metawalk on type */
    result = corto_metawalk(&s, corto_type(o), userData);

    return result;
}

static int c_typeDeclareDefine(corto_object o, void* userData) {
    corto_walk_opt s;
    int result;
    c_typeWalk_t *data = userData;

    result = 0;

    /* Get type-serializer */
    s = c_typeSerializer();

    /* Do metawalk on type */
    data->declareAndDefine = true;
    result = corto_metawalk(&s, corto_type(o), userData);
    data->declareAndDefine = false;

    return result;
}

/* Generator main */
int16_t genmain(g_generator g) {
    c_typeWalk_t walkData;

    /* Prepare walkdata, open headerfile */
    walkData.header = c_typeHeaderFileOpen(g);
    if (!walkData.header) {
        goto error;
    }

    walkData.g = g;
    walkData.prefixComma = FALSE;

    /* Collect native types */
    ut_ll native_types = ut_ll_new();
    if (corto_genTypeDepWalk(g, NULL, c_nativeCollect, NULL, native_types)) {
        goto error;
    }

    /* Define native types as void* if _type.h is used by itself */
    if (ut_ll_count(native_types)) {
        corto_id path;
        corto_path(path, root_o, g_getCurrent(g), "_");
        strupper(path);
        g_fileWrite(walkData.header, "\n");
        g_fileWrite(walkData.header, "/* -- Native types -- */\n");
        g_fileWrite(walkData.header, "#ifndef %s_H\n", path);
        if (!ut_ll_walk(native_types, c_typeNativeDefWalk, &walkData)) {
            goto error;
        }
        g_fileWrite(walkData.header, "#endif\n");
    }
    ut_ll_free(native_types);

    g_fileWrite(walkData.header, "\n");
    g_fileWrite(walkData.header, "/* -- Type definitions -- */\n\n");

    /* Walk objects */
    if (corto_genTypeDepWalk(g, c_typeDeclare, c_typeDefine, c_typeDeclareDefine, &walkData)) {
        goto error;
    }

    g_fileWrite(walkData.header, "\n");

    /* Close headerfile */
    c_typeHeaderFileClose(walkData.header);

    return 0;
error:
    return -1;
}
