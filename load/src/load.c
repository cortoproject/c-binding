/*
 * c_load.c
 *
 *  Created on: Sep 20, 2012
 *      Author: sander
 */

#include "corto/corto.h"
#include "corto/gen/c/common/common.h"

typedef struct c_typeWalk_t {
    g_generator g;
    g_file header;
    g_file source;
    corto_uint32 firstComma;
    corto_uint32 errorCount;
    corto_ll collections;
    corto_bool scoped; /* Switch between generating scoped and anonymous */
} c_typeWalk_t;

/* Resolve object */
static corto_char* c_loadResolve(corto_object o, corto_char* out, corto_char* src, corto_char* context, c_typeWalk_t *data) {
    if (g_mustParse(data->g, o) && corto_checkAttr(o, CORTO_ATTR_SCOPED)) {
        corto_id varId;
        c_varId(data->g, o, varId);
        sprintf(out, "(corto_claim(%s), %s)", varId, varId);
    } else if (corto_checkAttr(o, CORTO_ATTR_SCOPED) && corto_childof(root_o, o)) {
        corto_id id;
        char *escaped;
        corto_fullpath(id, o);

        escaped = c_escapeString(id);

        if (!(src || context)) {
            sprintf(out, "corto_resolve(NULL, \"%s\")", escaped);
        } else {
            if (!src) {
                src = "NULL";
            }
            if (!context) {
                context = "NULL";
            }
            sprintf(out, "corto_resolve(NULL, \"%s\")", escaped);
        }

        corto_dealloc(escaped);
    } else {
        corto_id ostr;
        char *escapedOstr;
        struct corto_serializer_s stringSer;
        corto_string_ser_t data;

        /* Serialize object string */
        stringSer = corto_string_ser(CORTO_LOCAL|CORTO_READONLY|CORTO_PRIVATE, CORTO_NOT, CORTO_SERIALIZER_TRACE_ON_FAIL);

        *ostr = '\0';
        data.compactNotation = TRUE;
        data.buffer = CORTO_BUFFER_INIT;
        data.buffer.buf = NULL;
        data.buffer.max = 0;
        data.prefixType = TRUE;
        data.enableColors = FALSE;
        if (corto_serialize(&stringSer, o, &data)) {
            goto error;
        }

        char *str = corto_buffer_str(&data.buffer);

        escapedOstr = c_escapeString(str);
        sprintf(out, "corto_resolve(NULL, \"%s\")", escapedOstr);
        corto_dealloc(escapedOstr);
        corto_dealloc(str);
    }

    return out;
error:
    return NULL;
}

/* Get element id, for lists and maps. */
static corto_char* c_loadElementId(corto_value* v, corto_char* out, corto_int32 offset) {
    corto_uint32 i;
    corto_value* ptr;

    i = 0;
    ptr = v;

    do {
        if (ptr->kind == CORTO_ELEMENT) {
            i++;
        }
    } while((ptr = ptr->parent));

    sprintf(out, "_e%d_", i + offset);

    return out;
}

/* This function translates from a value-object to a valid C-string identifying a
 * part of the object that is being serialized. */
static corto_char* c_loadMemberId(c_typeWalk_t* data, corto_value* v, corto_char* out, corto_bool addMemberOperator) {
    corto_value* stack[CORTO_MAX_INHERITANCE_DEPTH];
    corto_uint32 count;
    corto_value *ptr;
    corto_object o;
    corto_type thisType;
    corto_bool objectDeref, derefMemberOperator;

    *out = '\0';

    /* Build serializer-stack */
    ptr = v;
    count = 0;
    while ((ptr->kind != CORTO_OBJECT) && (ptr->kind != CORTO_BASE)) {
        stack[count] = ptr;
        ptr = ptr->parent;
        count++;
    }
    stack[count] = ptr;

    /* Print object */
    o = corto_value_getObject(v);

    /* If object is a collection or primtive, dereference object pointer */
    corto_type t = corto_typeof(o);
    objectDeref =
        (t->kind == CORTO_PRIMITIVE) ||
        ((t->kind == CORTO_COLLECTION) &&
         (corto_collection(t)->kind != CORTO_ARRAY));

    derefMemberOperator = !(objectDeref || ((t->kind == CORTO_COLLECTION) &&
                              (corto_collection(t)->kind == CORTO_ARRAY)));

    /* Start with a dereference */
    if (objectDeref) {
        strcpy(out, "(*");
    }

    /* If the first found object-value in the value-stack is not of the type of the object,
     * cast it. This happens when using inheritance. */
    thisType = corto_value_getType(ptr);
    if (corto_type(thisType) != corto_typeof(o)) {
        corto_id id, parentId, objectId;
        /* Use standard C-style cast to prevent assertType macro, and more
         * importantly, accidentally using symbols from nested dependencies */
        sprintf(id, "((%s%s)%s)",
                g_fullOid(data->g, thisType, parentId),
                thisType->reference ? "" : "*",
                c_varId(data->g, corto_value_getObject(v), objectId));
        strcat(out, id);
    } else {
        corto_id objectId;
        strcat(out, c_varId(data->g, corto_value_getObject(v), objectId));
    }

    /* End bracket used for dereferencing object */
    if (objectDeref) {
        strcat(out, ")");
    }

    /* Walk serializer stack */
    while (count) {
        count--;

        switch (stack[count]->kind) {

        /* Member */
        case CORTO_MEMBER:
            /* When previous object is a reference, use -> operator. */
            if (derefMemberOperator) {
                strcat(out, "->");
                derefMemberOperator = FALSE;
            } else {
                strcat(out, ".");
            }

            /* Reference member using it's name. */
            corto_id memberId;
            strcat(out, g_id(data->g, corto_idof(stack[count]->is.member.t), memberId));
            break;

        /* Element */
        case CORTO_ELEMENT: {
            corto_collection t;
            corto_char arrayIndex[24];

            t = corto_collection(corto_value_getType(stack[count+1]));

            switch (t->kind) {

            /* Array element, use array operator. */
            case CORTO_ARRAY:
                sprintf(arrayIndex, "[%d]", stack[count]->is.element.t.index);
                strcat(out, arrayIndex);
                break;

            /* Sequence element, use buffer-array */
            case CORTO_SEQUENCE:
                if (derefMemberOperator) {
                    strcat(out, "->");
                } else {
                    strcat(out, ".");
                }
                sprintf(arrayIndex, "buffer[%d]", stack[count]->is.element.t.index);
                strcat(out, arrayIndex);
                break;

            /* Use elementId's for non-array collections. */
            default: {
                corto_char elementId[9]; /* One-million nested collections should be adequate in most cases. */

                if ((corto_value_getType(stack[count])->kind == CORTO_COLLECTION) && (corto_collection(corto_value_getType(stack[count]))->kind == CORTO_ARRAY)) {
                    sprintf(out, "(*%s)", c_loadElementId(stack[count], elementId, 0));
                } else {
                    sprintf(out, "%s", c_loadElementId(stack[count], elementId, 0));
                    derefMemberOperator = TRUE;
                }
                break;
            }
            }
            break;
        }

        /* CORTO_OBJECT and CORTO_CONSTANT will not be encountered in this loop. */
        default:
            corto_assert(0, "invalid valueKind at this place.");
            break;
        }
    }

    if (addMemberOperator) {
        if (derefMemberOperator) {
            strcat(out, "->");
        } else {
            strcat(out, ".");
        }
    }

    return out;
}

/* Walk types */
static int c_loadDeclareWalk(corto_object o, void* userData) {
    c_typeWalk_t* data;
    corto_id specifier, objectId;
    corto_type t;
    corto_object parent;

    data = userData;
    t = corto_typeof(o);

    if (!g_mustParse(data->g, o)) {
        return 1;
    }

    if (!corto_checkAttr(o, CORTO_ATTR_SCOPED) && !corto_instanceof(corto_type_o, o)) {
        return 1;
    }

    if (corto_checkAttr(o, CORTO_ATTR_SCOPED)) {
        parent = corto_parentof(o);
        if (parent && (parent != root_o) && (!g_mustParse(data->g, parent))) {
            c_loadDeclareWalk(corto_parentof(o), userData);
        }
    }

    /* Get C typespecifier */
    c_typeret(data->g, t, C_ByReference, specifier);

    c_varId(data->g, o, objectId);

    c_writeExport(data->g, data->header);

    /* Declare objects in headerfile and define in sourcefile */
    g_fileWrite(data->header, " extern %s %s;\n", specifier, objectId);
    g_fileWrite(data->source, "%s %s;\n", specifier, objectId);

    return 1;
}

/* Open generator headerfile */
static g_file c_loadHeaderFileOpen(g_generator g) {
    g_file result;
    corto_id headerFileName, path;

    /* Create file */
    sprintf(headerFileName, "_load.h");
    result = g_fileOpen(g, headerFileName);

    corto_path(path, root_o, g_getCurrent(g), "_");
    corto_strupper(path);

    /* Print standard comments and includes */
    g_fileWrite(result, "/* %s\n", headerFileName);
    g_fileWrite(result, " *\n");
    g_fileWrite(result, " * Loads objects in object store.\n");
    g_fileWrite(result, " * This file contains generated code. Do not modify!\n");
    g_fileWrite(result, " */\n\n");
    g_fileWrite(result, "#ifndef %s_LOAD_H\n", path);
    g_fileWrite(result, "#define %s_LOAD_H\n\n", path);
    c_includeFrom(g, result, corto_o, "corto.h");
    c_includeFrom(g, result, g_getCurrent(g), "_project.h");

    /* Include _type.h from dependencies, just in case there are objects of a
     * type defined in another package */
    c_includeDependencies(g, result, "_type.h");

    g_fileWrite(result, "\n");
    g_fileWrite(result, "#ifdef __cplusplus\n");
    g_fileWrite(result, "extern \"C\" {\n");
    g_fileWrite(result, "#endif\n\n");

    return result;
}

/* Close headerfile */
static void c_loadHeaderFileClose(g_generator g, g_file file) {
    CORTO_UNUSED(g);

    /* Print standard comments and includes */
    g_fileWrite(file, "\n");
    g_fileWrite(file, "#ifdef __cplusplus\n");
    g_fileWrite(file, "}\n");
    g_fileWrite(file, "#endif\n");
    g_fileWrite(file, "#endif\n\n");
}

/* Open generator sourcefile */
static g_file c_loadSourceFileOpen(g_generator g) {
    g_file result;
    corto_id fileName;
    corto_bool cpp = !strcmp(g_getAttribute(g, "c4cpp"), "true");

    /* Create file */
    sprintf(fileName, "_load.%s", cpp ? "cpp" : "c");
    result = g_hiddenFileOpen(g, fileName);
    if (!result) {
        goto error;
    }

    /* Print standard comments and includes */
    g_fileWrite(result, "/* %s\n", fileName);
    g_fileWrite(result, " *\n");
    g_fileWrite(result, " * Loads objects in object store.\n");
    g_fileWrite(result, " * This file contains generated code. Do not modify!\n");
    g_fileWrite(result, " */\n\n");

    c_include(g, result, g_getCurrent(g));

    return result;
error:
    return NULL;
}

/* Write starting comment of variable definitions */
static void c_sourceWriteVarDefStart(g_file file) {
    g_fileWrite(file, "/* Variable definitions */\n");
}

/* Write start of load-routine */
static void c_sourceWriteLoadStart(g_generator g, g_file file) {
    g_fileWrite(file, "\n");
    g_fileWrite(file, "/* Load objects in object store. */\n");
    g_fileWrite(file, "int %s_load(void) {\n", g_getProjectName(g));
    g_fileIndent(file);
    g_fileWrite(file, "corto_object _a_; /* Used for resolving anonymous objects */\n");
    g_fileWrite(file, "corto_object _e_; /* Used for resolving extern objects */\n");
    g_fileWrite(file, "(void)_e_;\n");
    g_fileWrite(file, "_a_ = NULL;\n\n");
    g_fileWrite(file, "corto_attr prevAttr = corto_getAttr();\n\n");
}

/* Write end of load-routine */
static void c_sourceWriteLoadEnd(g_file file, c_typeWalk_t *data) {
    g_fileWrite(file, "if (_a_) {\n");
    g_fileIndent(file);
    g_fileWrite(file, "corto_release(_a_);\n");
    g_fileDedent(file);
    g_fileWrite(file, "}\n\n");
    g_fileWrite(file, "return 0;\n");
    if (data->errorCount) {
        g_fileDedent(file);
        g_fileWrite(file, "error:\n");
        g_fileIndent(file);
        g_fileWrite(file, "if (_a_) {\n");
        g_fileIndent(file);
        g_fileWrite(file, "corto_release(_a_);\n");
        g_fileDedent(file);
        g_fileWrite(file, "}\n\n");
        g_fileWrite(file, "corto_setAttr(prevAttr);\n");
        g_fileWrite(file, "return -1;\n");
    }
    g_fileDedent(file);
    g_fileWrite(file, "}\n");
}

/* Print variable start */
static void c_varPrintStart(corto_value* v, c_typeWalk_t* data) {
    corto_id memberId;
    corto_type t;

    t = corto_value_getType(v);

    /* Only write an identifier if the object is a primitive type, or a reference. */
    if ((t->kind == CORTO_PRIMITIVE) || (t->reference && !(v->kind == CORTO_OBJECT))) {
        /* Print memberId if object is member */
        g_fileWrite(data->source, "%s = ",
                c_loadMemberId(data, v, memberId, FALSE));
    }
}

/* Print variable end */
static void c_varPrintEnd(corto_value* v, c_typeWalk_t* data) {
    corto_type t;

    /* Get member object */
    t = corto_value_getType(v);
    if ((t->kind == CORTO_PRIMITIVE) || (t->reference && !(v->kind == CORTO_OBJECT))) {
        /* Print end of member-assignment */
        g_fileWrite(data->source, ";\n");
    }
}

/* c_initPrimitive */
static corto_int16 c_initPrimitive(corto_serializer s, corto_value* v, void* userData) {
    void* ptr;
    corto_type t;
    corto_string str;
    c_typeWalk_t* data;
    CORTO_UNUSED(s);

    ptr = corto_value_getPtr(v);
    t = corto_value_getType(v);
    data = userData;
    str = NULL;

    c_varPrintStart(v, userData);

    /* Treat booleans separately, the default convert translates booleans to 'true' and 'false' while
     * the language mapping of C TRUE and FALSE is. */
    if (corto_primitive(t)->kind == CORTO_BOOLEAN) {
        if (*(corto_bool*)ptr) {
            str = corto_strdup("TRUE");
        } else {
            str = corto_strdup("FALSE");
        }
    } else if (corto_primitive(t)->kind == CORTO_ENUM) {
        corto_id enumId;

        /* Convert constant-name to language id */
        str = corto_strdup(c_constantId(data->g, corto_enum_constant(corto_enum(t), *(corto_uint32*)ptr), enumId));
    } else if (corto_primitive(t)->kind == CORTO_BITMASK) {
        str = corto_alloc(11);
        sprintf(str, "0x%x", *(corto_uint32*)ptr);
    } else if (corto_primitive(t)->kind == CORTO_TEXT) {
        corto_string v = *(corto_string*)ptr;
        if (v) {
            size_t n = stresc(NULL, 0, v);
            str = malloc(strlen("corto_strdup(\"\")") + n + 1);
            strcpy(str, "corto_strdup(\"");
            stresc(str + strlen("corto_strdup(\""), n + 1, v);
            strcat(str, "\")");
        } else {
            str = corto_strdup("NULL");
        }
    } else if (corto_primitive(t)->kind == CORTO_CHARACTER) {
        corto_char v = *(corto_char*)ptr;
        if (v) {
            char buff[3];
            chresc(buff, v, '\'')[0] = '\0';
            str = malloc(strlen(buff) + 1 + 2);
            sprintf(str, "'%s'", buff);
        } else {
            str = corto_strdup("'\\0'");
        }
    } else {
        /* Convert primitive value to string using built-in conversion */
        if (corto_convert(corto_primitive(t), ptr, corto_primitive(corto_string_o), &str)) {
            goto error;
        }

        /* Capitalize NAN */
        if ((corto_primitive(t)->kind == CORTO_FLOAT) && !strcmp(str, "nan")) {
            corto_setstr(&str, "NAN");
        }
    }

    g_fileWrite(data->source, "%s", str);
    corto_dealloc(str);

    c_varPrintEnd(v, userData);

    return 0;
error:
    return -1;
}

/* c_initReference */
static corto_int16 c_initReference(corto_serializer s, corto_value* v, void* userData) {
    corto_object *optr, o;
    c_typeWalk_t* data;
    CORTO_UNUSED(s);

    data = userData;
    optr = corto_value_getPtr(v);

    c_varPrintStart(v, userData);

    if ((o = *optr)) {
        corto_id id, src, context, typeId, postfix;
        c_varId(data->g, corto_value_getObject(v), src);
        c_specifierId(data->g, corto_value_getType(v), typeId, NULL, postfix);
        corto_strving(v, context, 256);
        g_fileWrite(data->source, "%s(%s)",
          typeId,
          c_loadResolve(o, id, src, context, data));
    } else {
        g_fileWrite(data->source, "NULL");
    }

    c_varPrintEnd(v, userData);

    return 0;
}

/* c_initElement */
static corto_int16 c_initElement(corto_serializer s, corto_value* v, void* userData) {
    c_typeWalk_t* data = userData;
    corto_collection t = corto_collection(corto_value_getType(v->parent));
    corto_bool requiresAlloc = corto_collection_requiresAlloc(t->elementType);

    /* Allocate space for element */
    switch (t->kind) {
    case CORTO_LIST:
    case CORTO_MAP: {
        corto_id elementId, specifier, postfix;
        g_fileWrite(data->source, "\n");

        if (requiresAlloc) {
            c_specifierId(data->g, t->elementType, specifier, NULL, postfix);
            g_fileWrite(data->source, "%s = corto_alloc(sizeof(%s%s));\n",
                c_loadElementId(v, elementId, 0), specifier, postfix);
        }
        break;
    }
    default:
        break;
    }

    /* Serialize value */
    if (corto_serializeValue(s, v, data)) {
        goto error;
    }

    switch (t->kind) {
    case CORTO_LIST: {
        corto_id parentId, elementId;
        g_fileWrite(data->source, "corto_llAppend(%s, %s%s);\n",
                c_loadMemberId(data, v->parent, parentId, FALSE),
                requiresAlloc ? "" : "(void*)(intptr_t)", c_loadElementId(v, elementId, 0));
        break;
    }
    case CORTO_MAP: /*{
        corto_id parentId, elementId;
        g_fileWrite(data->source, "corto_rbtreeSet(%s, %s)",
                c_loadMemberId(data->g, v->parent, parentId),
                c_loadElementId(v, elementId, 0));
        break;
    }*/
    default:
        break;
    }

    return 0;
error:
    return -1;
}

/* c_initCollection */
static corto_int16 c_initCollection(corto_serializer s, corto_value* v, void* userData) {
    corto_collection t;
    c_typeWalk_t* data;
    corto_id memberId;
    int result;
    void* ptr;
    corto_uint32 size = 0;

    ptr = corto_value_getPtr(v);

    t = corto_collection(corto_value_getType(v));
    data = userData;

    switch (t->kind) {
    case CORTO_ARRAY:
        size = t->max;
        break;
    case CORTO_SEQUENCE: {
        corto_uint32 length;
        corto_id specifier, postfix;

        length = *(corto_uint32*)ptr;
        size = length;

        /* Set length of sequence */
        g_fileWrite(data->source, "%slength = %d;\n",
                c_loadMemberId(data, v, memberId, TRUE),
                length);

        /* Get type-specifier */
        c_specifierId(data->g, t->elementType, specifier, NULL, postfix);

        /* Allocate buffer */
        if (!length) {
            g_fileWrite(data->source, "%sbuffer = NULL;\n",
                    c_loadMemberId(data, v, memberId, TRUE));
        } else if (length == 1) {
            g_fileWrite(data->source, "%sbuffer = (%s*)corto_alloc(sizeof(%s%s));\n",
                    c_loadMemberId(data, v, memberId, TRUE),
                    specifier, specifier, postfix);
        } else {
            g_fileWrite(data->source, "%sbuffer = (%s*)corto_alloc(sizeof(%s%s) * %d);\n",
                    c_loadMemberId(data, v, memberId, TRUE),
                    specifier, specifier, postfix,
                    length);
        }
        break;
    }
    case CORTO_LIST:
        /* Lists are created by initializer */
        size = corto_llSize(*(corto_ll*)ptr);
        break;
    case CORTO_MAP: {
        corto_id keyId;
        /* Create map object */
        if (*(corto_rbtree*)ptr) {
            g_fileWrite(data->source, "%s = corto_rbtreeNew(%s);\n",
                    c_loadMemberId(data, v, memberId, FALSE), g_fullOid(data->g, corto_rbtreeKeyType(*(corto_rbtree*)ptr), keyId));
        } else {
            g_fileWrite(data->source, "%s = NULL;\n", c_loadMemberId(data, v, memberId, FALSE));
        }
        size = corto_rbtreeSize(*(corto_rbtree*)ptr);
        break;
    }
    }

    /* For the non-array types, allocate a member variable, if size of collection is not zero. */
    if (size) {
        switch (t->kind) {
        case CORTO_LIST:
        case CORTO_MAP: {
            corto_id elementId, elementTypeId;
            g_fileWrite(data->source, "{\n");
            g_fileIndent(data->source);

            g_fileWrite(
                data->source,
                "%s%s %s;\n",
                g_fullOid(data->g, t->elementType, elementTypeId),
                corto_collection_requiresAlloc(t->elementType) ? "*" : "",
                c_loadElementId(v, elementId, 1));

            break;
        }
        default:
            break;
        }
    }

    /* Serialize elements */
    result = corto_serializeElements(s, v, userData);

    if (size) {
        switch (t->kind) {
        case CORTO_LIST:
        case CORTO_MAP: {
            g_fileDedent(data->source);
            g_fileWrite(data->source, "}\n");
            break;
        }
        default:
            break;
        }
    }

    return result;
}

/* Write forward-declaration to interface function, return name. */
static int c_loadCFunction(corto_function o, c_typeWalk_t* data, corto_id name) {

    /* Print name */
    g_fullOid(data->g, o, name);
    if (c_procedureHasThis(o)) {
        if (o->overridable) {
            strcat(name, "_v");
        }
    }

    return 0;
}

/* Create serializer that initializes object values */
static struct corto_serializer_s c_initSerializer(void) {
    struct corto_serializer_s s;

    corto_serializerInit(&s);

    s.access = CORTO_LOCAL;
    s.accessKind = CORTO_NOT;
    s.aliasAction = CORTO_SERIALIZER_ALIAS_IGNORE;
    s.optionalAction = CORTO_SERIALIZER_OPTIONAL_ALWAYS;
    s.traceKind = CORTO_SERIALIZER_TRACE_ON_FAIL;
    s.program[CORTO_PRIMITIVE] = c_initPrimitive;
    s.program[CORTO_COLLECTION] = c_initCollection;
    s.metaprogram[CORTO_ELEMENT] = c_initElement;
    s.reference = c_initReference;

    return s;
}

/* Declare object */
static int c_loadDeclare(corto_object o, void* userData) {
    c_typeWalk_t* data;
    corto_id varId, parentId, typeId, typeCast;
    char *escapedName = NULL;

    data = userData;

    if (!corto_checkAttr(o, CORTO_ATTR_SCOPED) && !corto_instanceof(corto_type_o, o)) {
        return 1;
    }

    c_varId(data->g, o, varId);
    c_typeId(data->g, corto_typeof(o), typeCast);

    if (o == g_getCurrent(data->g) && corto_instanceof(corto_package_o, o)) {
         g_fileWrite(data->source, "prevAttr = corto_setAttr(CORTO_ATTR_PERSISTENT);\n");
    }

    if (!corto_checkAttr(o, CORTO_ATTR_SCOPED) || !corto_childof(root_o, o)) {
        g_fileWrite(data->source, "%s = %s(corto_declare(", varId, typeCast);
    } else {
        if (!g_mustParse(data->g, o)) {
            return 1;
        }

        if (g_mustParse(data->g, corto_parentof(o))) {
            c_varId(data->g, corto_parentof(o), parentId);
        } else {
            g_fileWrite(data->source, "_e_ = corto_lookup(NULL, \"%s\");\n",
                corto_fullpath(NULL, corto_parentof(o)));
            strcpy(parentId, "_e_");

            g_fileWrite(data->source, "if (!_e_) {\n");
            g_fileIndent(data->source);
            g_fileWrite(data->source, "corto_error(\"failed to lookup '%s'\");\n", corto_fullpath(NULL, corto_parentof(o)));
            g_fileWrite(data->source, "goto error;\n");
            g_fileDedent(data->source);
            g_fileWrite(data->source, "}\n");
        }

        escapedName = c_escapeString(corto_idof(o));
        g_fileWrite(data->source, "%s = %s(corto_declareChild(%s, \"%s\", ",
            varId,
            typeCast,
            parentId,
            escapedName);
        corto_dealloc(escapedName);
    }

    /* Declaration */
    if (!corto_checkAttr(corto_typeof(o), CORTO_ATTR_SCOPED)) {
        g_fileWrite(data->source, "(_a_ ? corto_release(_a_) : 0, _a_ = %s)));\n",
            c_varId(data->g, corto_typeof(o), typeId));
    } else {
        g_fileWrite(data->source, "%s));\n",
            c_varId(data->g, corto_typeof(o), typeId));
    }

    if (corto_checkAttr(o, CORTO_ATTR_SCOPED) && corto_parentof(o) && !g_mustParse(data->g, corto_parentof(o))) {
        g_fileWrite(data->source, "corto_release(_e_);\n");
    }

    /* Error checking */
    g_fileWrite(data->source, "if (!%s) {\n", varId);
    g_fileIndent(data->source);
    g_fileWrite(data->source, "corto_error(\"%s_load: failed to declare '%s' (%%s)\", corto_lasterr());\n",
            g_getProjectName(data->g),
            varId);
    g_fileWrite(data->source, "goto error;\n");
    data->errorCount++;
    g_fileDedent(data->source);

    g_fileWrite(data->source, "}\n");

    if (o == g_getCurrent(data->g) && corto_instanceof(corto_package_o, o)) {
        g_fileWrite(data->source, "corto_setAttr(prevAttr);\n");
    }

    g_fileWrite(data->source, "\n");

    return 1;
}

/* Forward declare procedures */
static corto_int16 c_loadFwdDeclProcedure(corto_function f, c_typeWalk_t* data) {
    g_fileWrite(data->source, "\n#ifdef __cplusplus\n");
    g_fileWrite(data->source, "extern \"C\"\n");
    g_fileWrite(data->source, "#endif\n");
    c_decl(data->g, data->source, f, FALSE, TRUE);
    g_fileWrite(data->source, ";\n");
    return 0;
}
static int c_walkProcedures(corto_object o, void *userData) {
    if (corto_instanceof(corto_function_o, o)) {
        c_loadFwdDeclProcedure(o, userData);
    }
    return 1;
}

/* Define object */
static int c_loadDefine(corto_object o, void* userData) {
    struct corto_serializer_s s;
    c_typeWalk_t* data = userData;

    if (!g_mustParse(data->g, o)) {
        return 1;
    }

    if (!corto_checkAttr(o, CORTO_ATTR_SCOPED) && !corto_instanceof(corto_type_o, o)) {
        return 1;
    }

    corto_id varId, postfix;
    char *escapedId = NULL;

    c_varId(data->g, o, varId);

    g_fileWrite(data->source, "if (!corto_checkState(%s, CORTO_DEFINED)) {\n", varId);
    g_fileIndent(data->source);

    /* Serialize object if object is not a primitive */
    s = c_initSerializer();
    corto_serialize(&s, o, userData);

    /* If object is a procedure, set function implementation */
    if (corto_class_instanceof(corto_procedure_o, corto_typeof(o))) {
        corto_id name;
        g_fileWrite(data->source, "\n");
        if (!corto_function(o)->impl) {
            g_fileWrite(data->source, "corto_function(%s)->kind = CORTO_PROCEDURE_CDECL;\n", varId);
            c_loadCFunction(o, data, name);
            g_fileWrite(data->source, "corto_function(%s)->fptr = (corto_word)_%s;\n", varId, name);
        }
    }

    /* Define object */
    g_fileWrite(data->source, "if (corto_define(%s)) {\n", varId);
    g_fileIndent(data->source);
    escapedId = c_escapeString(varId);
    g_fileWrite(data->source, "corto_error(\"%s_load: failed to define '%s' (%%s)\", corto_lasterr());\n",
            g_getProjectName(data->g),
            escapedId);
    corto_dealloc(escapedId);
    g_fileWrite(data->source, "goto error;\n");
    data->errorCount++;
    g_fileDedent(data->source);
    g_fileWrite(data->source, "}\n");
    g_fileDedent(data->source);
    g_fileWrite(data->source, "}\n\n");

    /* Do size validation - this makes porting to other platforms easier */
    if (corto_instanceof(corto_type(corto_type_o), o)) {
        corto_id typeId;
        c_specifierId(data->g, o, typeId, NULL, postfix);

        if ((corto_type(o)->kind == CORTO_COMPOSITE) && corto_type(o)->reference && (corto_interface(o)->kind != CORTO_INTERFACE)) {
            g_fileWrite(data->source, "if (corto_type(%s)->size != sizeof(struct %s_s)) {\n",
                varId,
                typeId);
            g_fileIndent(data->source);
            g_fileWrite(data->source,
                "corto_error(\"%s_load: calculated size '%%d' of type '%s' doesn't match C-type size '%%d'\", corto_type(%s)->size, sizeof(struct %s_s));\n",
                g_getProjectName(data->g),
                varId,
                varId,
                typeId);
            g_fileDedent(data->source);
            g_fileWrite(data->source, "}\n\n");
        } else if (corto_type(o)->reference || (corto_type(o)->kind != CORTO_VOID)) {
            g_fileWrite(data->source, "if (corto_type(%s)->size != sizeof(%s)) {\n",
                varId,
                typeId);
            g_fileIndent(data->source);
            g_fileWrite(data->source,
                "corto_error(\"%s_load: calculated size '%%d' of type '%s' doesn't match C-type size '%%d'\", corto_type(%s)->size, sizeof(%s));\n",
                g_getProjectName(data->g),
                varId,
                varId,
                typeId);
            g_fileDedent(data->source);
            g_fileWrite(data->source, "}\n\n");
        }
    }

    return 1;
}

/* Entry point for generator */
int corto_genMain(g_generator g) {
    c_typeWalk_t walkData;

    /* Default prefixes for corto namespaces */
    g_parse(g, corto_o, FALSE, FALSE, "");
    g_parse(g, corto_lang_o, FALSE, FALSE, "corto");
    g_parse(g, corto_core_o, FALSE, FALSE, "corto");
    g_parse(g, corto_native_o, FALSE, FALSE, "corto_native");
    g_parse(g, corto_secure_o, FALSE, FALSE, "corto_secure");

    /* Prepare walkData, create header- and sourcefile */
    walkData.g = g;
    walkData.header = c_loadHeaderFileOpen(g);
    if (!walkData.header) {
        goto error;
    }
    walkData.source = c_loadSourceFileOpen(g);
    if (!walkData.source) {
        goto error;
    }
    walkData.collections = c_findType(g, corto_collection_o);
    walkData.errorCount = 0;

    g_fileWrite(walkData.source, "\n/* Forward declarations */\n");
    if (!g_walkRecursive(g, c_walkProcedures, &walkData)) {
        goto error;
    }

    g_fileWrite(walkData.source, "\n");


    /* Write comment indicating definitions in sourcefile */
    c_sourceWriteVarDefStart(walkData.source);

    /* Walk objects, declare variables in header and sourcefile */
    if (corto_genDepWalk(g, c_loadDeclareWalk, NULL, &walkData)) {
        goto error;
    }

    /* Create load-routine */
    c_sourceWriteLoadStart(g, walkData.source);

    /* Walk objects in dependency order */
    if (corto_genDepWalk(g, c_loadDeclare, c_loadDefine, &walkData)) {
        corto_trace("generation of load-routine failed while resolving dependencies.");
        goto error;
    }

    /* Close load-routine */
    c_sourceWriteLoadEnd(walkData.source, &walkData);

    /* Close headerfile */
    c_loadHeaderFileClose(g, walkData.header);

    return 0;
error:
    return -1;
}
