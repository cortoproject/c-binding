/*
 * c_common.c
 *
 *  Created on: Dec 21, 2012
 *      Author: sander
 */

#include "corto/gen/c/common/common.h"
#include "ctype.h"

/* Escape language keywords */
static int c_typeKeywordEscape(corto_string inputName, corto_string buffer) {

    if( !strcmp(inputName, "alignas") ||
        !strcmp(inputName, "alignof") ||
        !strcmp(inputName, "and") ||
        !strcmp(inputName, "and_eq") ||
        !strcmp(inputName, "asm") ||
        !strcmp(inputName, "auto") ||
        !strcmp(inputName, "bitand") ||
        !strcmp(inputName, "bitor") ||
        !strcmp(inputName, "bool") ||
        !strcmp(inputName, "break") ||
        !strcmp(inputName, "case") ||
        !strcmp(inputName, "catch") ||
        !strcmp(inputName, "char") ||
        !strcmp(inputName, "char16_t") ||
        !strcmp(inputName, "char32_t") ||
        !strcmp(inputName, "class") ||
        !strcmp(inputName, "compl") ||
        !strcmp(inputName, "const") ||
        !strcmp(inputName, "constexpr") ||
        !strcmp(inputName, "const_cast") ||
        !strcmp(inputName, "continue") ||
        !strcmp(inputName, "decltype") ||
        !strcmp(inputName, "default") ||
        !strcmp(inputName, "delete") ||
        !strcmp(inputName, "do") ||
        !strcmp(inputName, "double") ||
        !strcmp(inputName, "dynamic_cast") ||
        !strcmp(inputName, "else") ||
        !strcmp(inputName, "enum") ||
        !strcmp(inputName, "explicit") ||
        !strcmp(inputName, "export") ||
        !strcmp(inputName, "extern") ||
        !strcmp(inputName, "false") ||
        !strcmp(inputName, "float") ||
        !strcmp(inputName, "for") ||
        !strcmp(inputName, "friend") ||
        !strcmp(inputName, "goto") ||
        !strcmp(inputName, "if") ||
        !strcmp(inputName, "inline") ||
        !strcmp(inputName, "int") ||
        !strcmp(inputName, "long") ||
        !strcmp(inputName, "mutable") ||
        !strcmp(inputName, "namespace") ||
        !strcmp(inputName, "new") ||
        !strcmp(inputName, "noexcept") ||
        !strcmp(inputName, "not") ||
        !strcmp(inputName, "not_eq") ||
        !strcmp(inputName, "nullptr") ||
        !strcmp(inputName, "operator") ||
        !strcmp(inputName, "or") ||
        !strcmp(inputName, "or_eq") ||
        !strcmp(inputName, "private") ||
        !strcmp(inputName, "protected") ||
        !strcmp(inputName, "public") ||
        !strcmp(inputName, "register") ||
        !strcmp(inputName, "reinterpret_cast") ||
        !strcmp(inputName, "return") ||
        !strcmp(inputName, "short") ||
        !strcmp(inputName, "signed") ||
        !strcmp(inputName, "sizeof") ||
        !strcmp(inputName, "static") ||
        !strcmp(inputName, "static_assert") ||
        !strcmp(inputName, "static_cast") ||
        !strcmp(inputName, "struct") ||
        !strcmp(inputName, "switch") ||
        !strcmp(inputName, "template") ||
        !strcmp(inputName, "this") ||
        !strcmp(inputName, "thread_local") ||
        !strcmp(inputName, "throw") ||
        !strcmp(inputName, "true") ||
        !strcmp(inputName, "try") ||
        !strcmp(inputName, "typedef") ||
        !strcmp(inputName, "typeid") ||
        !strcmp(inputName, "typename") ||
        !strcmp(inputName, "union") ||
        !strcmp(inputName, "using") ||
        !strcmp(inputName, "unsigned") ||
        !strcmp(inputName, "virtual") ||
        !strcmp(inputName, "void") ||
        !strcmp(inputName, "volatile") ||
        !strcmp(inputName, "wchar_t") ||
        !strcmp(inputName, "xor") ||
        !strcmp(inputName, "while")) {
        memmove(buffer + 1, inputName, strlen(inputName) + 1);
        *buffer = '_';
    } else {
        memmove(buffer, inputName, strlen(inputName) + 1);
    }
    return 0;
}

corto_string corto_genId(corto_string str, corto_id id) {
    corto_char *ptr, ch, *idptr;

    ptr = str;
    idptr = id;

    /* Strip scope-operator for rootscope */
    if (*ptr) {
        if (*ptr == '/') {
            ptr += 1;
        }

        /* Replace '::' with '_' */
        while((ch = *ptr)) {
            switch(ch) {
            case '/':
                /* no break */
            case ',':
            case '(':
            case ' ':
            case '{':
                *idptr = '_';
                idptr++;
                break;
            case ')':
            case '}':
                break;
            default:
                *idptr = ch;
                idptr++;
                break;
            }
            ptr++;
        }
    }
    *idptr = '\0';

    /* Check for C-keywords */
    c_typeKeywordEscape(id, id);

    return id;
}

/* Get string representing the base-platform type */
corto_char* c_primitiveId(g_generator g, corto_primitive t, corto_char* buff) {
    corto_bool appendWidth, appendT;

    appendWidth = FALSE;
    appendT = FALSE;

    switch(t->kind) {
    case CORTO_BOOLEAN:
    case CORTO_CHARACTER:
        switch(t->width) {
        case CORTO_WIDTH_8:
            strcpy(buff, "char");
            break;
        case CORTO_WIDTH_16:
            strcpy(buff, "wchar");
            appendT = TRUE;
            break;
        default: {
            corto_seterr("unsupported width for primitive type '%s'.",
                corto_fullpath(NULL, t));
            goto error;
            break;
        }
        }
        break;
    case CORTO_BINARY:
    case CORTO_UINTEGER:
        strcpy(buff, "uint");
        appendWidth = TRUE;
        appendT = TRUE;
        break;
    case CORTO_INTEGER:
        strcpy(buff, "int");
        appendWidth = TRUE;
        appendT = TRUE;
        break;
    case CORTO_FLOAT:
        switch(t->width) {
        case CORTO_WIDTH_32:
            strcpy(buff, "float");
            break;
        case CORTO_WIDTH_64:
            strcpy(buff, "double");
            break;
        default: {
            corto_seterr("unsupported width for floating point type '%s'",
                corto_fullpath(NULL, t));
            goto error;
            break;
        }
        }
        break;
    case CORTO_ENUM:
    case CORTO_BITMASK:
        corto_seterr(
          "use c_specifierId instead of c_primitiveId for enums and bitmasks");
        goto error;
        g_fullOid(g, t, buff);
        break;
    case CORTO_TEXT:
        strcpy(buff, "char*");
        break;
    }

    /* Append width */
    if (appendWidth) {
        switch(t->width) {
        case CORTO_WIDTH_8:
            strcat(buff, "8");
            break;
        case CORTO_WIDTH_16:
            strcat(buff, "16");
            break;
        case CORTO_WIDTH_32:
            strcat(buff, "32");
            break;
        case CORTO_WIDTH_64:
            strcat(buff, "64");
            break;
        case CORTO_WIDTH_WORD:
            strcat(buff, "ptr");
            break;
        }
    }

    /* Append _t */
    if (appendT) {
        strcat(buff, "_t");
    }

    return buff;
error:
    return NULL;
}

/* Convert string to upper. */
static corto_string c_typeToUpper(corto_string str, corto_id buffer) {
    corto_char *ptr, *bptr, ch;

    ptr = str;
    bptr = buffer;
    while((ch = *ptr)) {
        *bptr = toupper(ch);
        bptr++;
        ptr++;
    }
    *bptr = '\0';

    return buffer;
}

static corto_char c_lastLetter(corto_string str) {
    char *ptr, ch = '\0';

    ptr = str + (strlen(str) - 1);

    while ((ptr >= str) && (ch = *ptr)) {
        if ((ch >= 'a') && (ch <= 'z')) {
            break;
        } else if ((ch >= 'A') && (ch <= 'Z')) {
            break;
        }
        ptr --;
    }

    return ptr >= str ? ch : '\0';
}

corto_bool c_typeHasCaps(corto_string str) {
    char *ptr = str, ch;

    for (;(ch = *ptr); ptr ++) {
        if (isupper(ch)) {
            return TRUE;
        }
    }

    return FALSE;
}

/* Translate constant to C-language id */
corto_char* c_constantId(g_generator g, corto_constant* c, corto_char* buffer) {
    corto_string prefixOrig;
    corto_enum e = corto_parentof(c);
    corto_string name = corto_idof(e->constants.buffer[0]);
    corto_id prefix;
    corto_char ch;

    prefixOrig = g_getPrefix(g, c);
    if (!prefixOrig) {
        prefixOrig = corto_idof(g_getCurrent(g));
    }

    strcpy(prefix, prefixOrig);

    /* If the prefix contains capitals, leave names unaltered */
    if (!c_typeHasCaps(prefix)) {
        /* Find last letter */
        ch = c_lastLetter(name);

        if (isupper(name[0])) {
            if (isupper(ch)) { /* All caps */
                c_typeToUpper(prefixOrig, prefix);
            } else { /* Initial caps */
                prefix[0] = toupper(prefix[0]);
            }
        }
    }

    sprintf(buffer, "%s_%s", prefix, corto_idof(c));

    return buffer;
}

/* Parse type into C-specifier */
corto_int16 c_specifierId(
    g_generator g,
    corto_type t,
    corto_char* specifier,
    corto_bool* prefix,
    corto_char* postfix)
{
    if (postfix) {
        *postfix = '\0';
    }

    /* If type is not a reference, objects that are defined with it need to add a prefix. This
     * won't be used for members or nested type-specifiers. */
    if (prefix) {
        if (!t->reference &&
            !(t->kind == CORTO_COLLECTION &&
            (corto_collection(t)->kind == CORTO_ARRAY)))
        {
            *prefix = TRUE;
        } else {
            *prefix = FALSE;
        }
    }

    if (corto_instanceof(corto_native_type_o, t)) {
        strcpy(specifier, corto_native_type(t)->name);
        return 0;
    }

    /* Check if object is scoped */
    if (corto_checkAttr(t, CORTO_ATTR_SCOPED) && corto_childof(root_o, t)) {
        g_fullOid(g, t, specifier);
    } else {
        switch(corto_type(t)->kind) {
        case CORTO_COLLECTION: {
            corto_id _specifier, _postfix;
            corto_type elementType = corto_collection(t)->elementType;
            switch(corto_collection(t)->kind) {
            case CORTO_ARRAY:
                /* Get specifier of elementType */
                if (c_specifierId(g, corto_collection(t)->elementType, _specifier, NULL, _postfix)) {
                    goto error;
                }
                if ((elementType->kind == CORTO_COLLECTION) && (corto_collection(elementType)->kind == CORTO_ARRAY)) {
                    sprintf(specifier, "%s_%d", _specifier, corto_collection(t)->max);
                } else {
                    sprintf(specifier, "%sArray%d", _specifier, corto_collection(t)->max);
                }
                break;
            case CORTO_SEQUENCE:
                /* Get specifier of elementType */
                if (c_specifierId(g, corto_collection(t)->elementType, _specifier, NULL, _postfix)) {
                    goto error;
                }
                if ((elementType->kind == CORTO_COLLECTION) && (corto_collection(elementType)->kind == CORTO_SEQUENCE)) {
                    sprintf(specifier, "%s_%d", _specifier, corto_collection(t)->max);
                } else {
                    if (corto_collection(t)->max) {
                        sprintf(specifier, "%sSeq%d", _specifier, corto_collection(t)->max);
                    } else {
                        sprintf(specifier, "%sSeq", _specifier);
                    }
                }
                break;
            case CORTO_LIST:
                if (c_specifierId(g, corto_collection(t)->elementType, _specifier, NULL, _postfix)) {
                    goto error;
                }
                if ((elementType->kind == CORTO_COLLECTION) && (corto_collection(elementType)->kind == CORTO_LIST)) {
                    sprintf(specifier, "%s_%d", _specifier, corto_collection(t)->max);
                } else {
                    if (corto_collection(t)->max) {
                        sprintf(specifier, "%sList%d", _specifier, corto_collection(t)->max);
                    } else {
                        sprintf(specifier, "%sList", _specifier);
                    }
                }
                break;
            case CORTO_MAP:
                strcpy(specifier, "corto_rbtree");
                break;
            }
            break;
        }
        default: {
            static corto_ll anonymousTypes = NULL;
            corto_uint32 count = 0;
            if (!anonymousTypes) {
                anonymousTypes = corto_llNew();
            }
            corto_iter it = corto_llIter(anonymousTypes);
            while (corto_iterHasNext(&it)) {
                corto_object o = corto_iterNext(&it);
                if (o == t) {
                    break;
                }
                count ++;
            }
            if (count == corto_llSize(anonymousTypes)) {
                corto_llAppend(anonymousTypes, t);
            }

            corto_object cur = g_getCurrent(g);
            if (corto_instanceof(corto_package_o, cur)) {
                corto_id packageId;
                g_fullOid(g, cur, packageId);
                sprintf(specifier, "anonymous_%s_%u", packageId, count);
            } else {
                sprintf(specifier, "anonymous_%u", count);
            }
            break;
        }
        }
    }

    return 0;
error:
    return -1;
}

corto_char* _c_typeId(g_generator g, corto_type t, corto_char *specifier) {
    corto_id postfix;
    c_specifierId(g, t, specifier, NULL, postfix);
    return specifier;
}

corto_char* c_escapeString(corto_string str) {
    size_t length = stresc(NULL, 0, str);
    corto_string out = corto_alloc(length + 1);
    stresc(out, length + 1, str);
    return out;
}

corto_bool c_procedureHasThis(corto_function o) {
    corto_procedure t = corto_procedure(corto_typeof(o));
    return t->hasThis;
}

corto_string c_paramName(corto_string name, corto_string buffer) {

    if (*name == '$') {
        if (!strcmp(name, "$__line")) {
            strcpy(buffer, name + 1);
        } else if (!strcmp(name, "$__file")) {
            strcpy(buffer, name + 1);
        } else {
            sprintf(buffer, "str_%s", name + 1);
        }
    } else {
        corto_id id;
        corto_genId(name, id);
        strcpy(buffer, id);
    }
    return buffer;
}

corto_bool c_paramRequiresPtr(corto_parameter *p) {
    return (!p->type->reference &&
           (p->passByReference || (p->type->kind == CORTO_COMPOSITE))) ||
           p->inout == CORTO_OUT ||
           p->inout == CORTO_INOUT;
}

corto_bool c_typeRequiresPtr(corto_type t) {
    return (!t->reference &&
           (t->kind == CORTO_COMPOSITE));
}

corto_string c_typeptr(g_generator g, corto_type t, corto_id id) {
    corto_id postfix;
    c_specifierId(g, t, id, NULL, postfix);
    if (!t->reference &&
        !(t->kind == CORTO_COLLECTION &&
        (corto_collection(t)->kind == CORTO_ARRAY)))
    {
        strcat(id, "*");
    }
    return id;
}

corto_string c_typeret(g_generator g, corto_type t, c_refKind ref, corto_id id) {
    if ((t->kind == CORTO_COLLECTION &&
        (corto_collection(t)->kind == CORTO_ARRAY)))
    {
        g_fullOid(g, corto_collection(t)->elementType, id);
        strcat(id, "*");
    } else {
        corto_id postfix;
        c_specifierId(g, t, id, NULL, postfix);
        if (!t->reference && (ref == C_ByReference))
        {
            strcat(id, "*");
        }
    }

    return id;
}

corto_string c_typeval(g_generator g, corto_type t, corto_id id) {
    corto_id postfix;
    c_specifierId(g, t, id, NULL, postfix);
    if (!t->reference && (t->kind == CORTO_COMPOSITE)) {
        strcat(id, "*");
    }
    return id;
}

corto_char* c_usingName(g_generator g, corto_object o, corto_id id) {
    corto_id buff;
    char *ptr;

    g_fullOid(g, o, buff);
    ptr = strchr(buff, '_');
    if (ptr) {
        strcpy(id, ptr + 1);
    } else {
        strcpy(id, buff);
    }

    return id;
}

corto_char* c_usingConstant(g_generator g, corto_id id) {
    corto_id buff;
    strcpy(id, "USING_");
    char *ptr = &id[6];
    corto_object o = g_getCurrent(g);
    corto_path(buff, root_o, o, "_");
    c_typeToUpper(buff, ptr);
    return id;
}

void c_writeExport(g_generator g, g_file file) {
    corto_id upperName;
    if (!strcmp(g_getAttribute(g, "bootstrap"), "true") || !g_getCurrent(g)) {
        strcpy(upperName, g_getName(g));
        char *ptr, ch;
        for (ptr = upperName; (ch = *ptr); ptr++) {
            if (ch == '/') *ptr = '_';
        }

    } else {
        corto_path(upperName, root_o, g_getCurrent(g), "_");
    }
    corto_strupper(upperName);
    g_fileWrite(file, "%s_EXPORT", upperName);
}

corto_object c_findPackage(g_generator g, corto_object o) {
    corto_object package = o;
    corto_object ptr;

    if (o != g_getCurrent(g)) {
        while (package && !corto_instanceof(corto_package_o, package)) {
            package = corto_parentof(package);
        }
    }

    /* If package is in scope of current package and not an import, use 
     * current package */
    if (g_getCurrent(g)) {
        ptr = NULL;
        if (g->imports) {
            corto_iter it = corto_llIter(g->imports);
            while (corto_iterHasNext(&it)) {
                corto_object o = corto_iterNext(&it);
                if (o == package) {
                    ptr = package;
                }
            }
        }

        if (!ptr) {
            ptr = package;
            while (ptr && (ptr != g_getCurrent(g))) {
                ptr = corto_parentof(ptr);
                if (ptr == g_getCurrent(g)) {
                    package = ptr;
                }
            }
        }
    }

    return package;
}

char* c_filename(
    g_generator g,
    char *fileName,
    corto_object o,
    char *ext)
{
    corto_id path;
    corto_object package = c_findPackage(g, o);
    corto_bool cpp = !strcmp(g_getAttribute(g, "c4cpp"), "true");

    if (cpp && !strcmp(ext, "c")) {
        ext = "cpp";
    }

    if (o != package) {
        corto_path(path, package, o, "_");
        sprintf(fileName, "%s.%s", path, ext);
    } else {
        sprintf(fileName, "%s.%s", corto_idof(package), ext);
    }

    return fileName;
}

void c_includeFrom(
    g_generator g,
    g_file file,
    corto_object o,
    corto_string include,
    ...)
{
    corto_id path, filebuff;
    va_list list;
    corto_object package = NULL;

    if (o) {
        package = c_findPackage(g, o);
    }

    va_start(list, include);
    vsprintf(filebuff, include, list);
    va_end(list);

    /* If an app or local project and the object to include is from this project
     * there is no need to prefix the header with a path */
    if ((!strcmp(g_getAttribute(g, "local"), "true") ||
         !strcmp(g_getAttribute(g, "app"), "true")) &&
        (g_getCurrent(g) == package))
    {
        /* Never include headers by just their names, as this could potentially
         * conflict with system headers, for example when a user creates a class
         * called "time". */
        g_fileWrite(file, "#include <include/%s>\n", filebuff);
    } else {
        if (package) {
            corto_path(path, root_o, package, "/");
        } else {
            strcpy(path, g_getName(g));
        }
        g_fileWrite(file, "#include <%s/%s>\n",
          path,
          filebuff);
    }
}

void c_include(g_generator g, g_file file, corto_object o) {
    corto_id name;
    corto_object package = c_findPackage(g, o);

    corto_assert (package != NULL, "can't include '%s' from non-package scopes",
        corto_fullpath(NULL, o));

    c_includeFrom(
      g,
      file,
      package,
      c_filename(g, name, o, "h"));
}

static corto_equalityKind c_compareCollections(corto_collection c1, corto_collection c2) {
    corto_equalityKind result = CORTO_EQ;
    if (c1->kind != c2->kind) {
        result = CORTO_NEQ;
    } else if (c1->elementType != c2->elementType) {
        result = CORTO_NEQ;
    } else if (c1->max != c2->max) {
        result = CORTO_NEQ ;
    } else if (c1->kind == CORTO_MAP) {
        if (corto_map(c1)->keyType != corto_map(c2)->keyType) {
            result = CORTO_NEQ;
        }
    }
    return result;
}

static int c_checkDuplicates(void* o, void* userData) {
    if (corto_checkAttr(o, CORTO_ATTR_SCOPED)) {
        return 1;
    } else {
        if (corto_instanceof(corto_collection_o, o)) {
            return c_compareCollections(
                corto_collection(o),
                corto_collection(userData)) != CORTO_EQ;
        } else if (corto_instanceof(corto_iterator_o, o)) {
            return corto_iterator(o)->elementType ==
                corto_iterator(userData)->elementType;
        }
    }

    return 0;
}

typedef struct c_findTypeWalk_t {
    corto_ll result;
    corto_class t;
} c_findTypeWalk_t;

static int c_findTypeWalk(corto_object o, void* userData) {
    c_findTypeWalk_t* data = userData;

    if (corto_instanceof(data->t, o)) {
        if (!corto_llSize(data->result) || corto_llWalk(data->result, c_checkDuplicates, o)) {
            corto_llAppend(data->result, o);
        }
    }

    return 0;
}

corto_ll c_findType(g_generator g, corto_class t) {
    c_findTypeWalk_t walkData;

    walkData.result = corto_llNew();
    walkData.t = t;

    if (corto_genDepWalk(g, c_findTypeWalk, NULL, &walkData)) {
        corto_seterr("generation of api-routines failed while resolving %s.",
            corto_fullpath(NULL, t));
        goto error;
    }

    return walkData.result;
error:
    return NULL;
}

corto_char* c_varId(g_generator g, corto_object o, corto_char* out) {

    if (o != root_o) {
        /* Using fully scoped name for package variables allows using
         * packages with the same name */
        if (corto_instanceof(corto_package_o, o)) {
            corto_path(out, root_o, o, "_");
        } else {
            corto_id postfix;
            if (corto_instanceof(corto_type_o, o) && (!corto_checkAttr(o, CORTO_ATTR_SCOPED) || !corto_childof(root_o, o))) {
                c_specifierId(g, o, out, NULL, postfix);
            } else {
                g_fullOid(g, o, out);
            }
        }
        strcat(out, "_o");
    } else {
        strcpy(out, "root_o");
    }

    return out;
}

char* c_functionName(g_generator g, corto_function o, corto_id id) {
    g_fullOid(g, o, id);
    if (o->overridable) {
        strcat(id, "_v");
    }
    return id;
}

typedef struct c_paramWalk_t {
    g_generator g;
    corto_bool firstComma;
    g_file file;
} c_paramWalk_t;

/* Generate parameters for method */
static int c_param(corto_parameter *o, void *userData) {
    c_paramWalk_t* data;
    corto_id specifier, postfix, name;

    data = userData;

    /* Write comma */
    if (data->firstComma) {
        g_fileWrite(data->file, ",\n    ");
    } else {
        g_fileWrite(data->file, "\n    ");
    }

    if (c_specifierId(data->g, o->type, specifier, NULL, postfix)) {
        goto error;
    }

    g_fileWrite(data->file, "%s ", specifier);

    if (c_paramRequiresPtr(o)) {
        g_fileWrite(data->file, "*");
    }

    /* Write to source */
    g_fileWrite(data->file, "%s%s", c_paramName(o->name, name), postfix);

    data->firstComma++;

    return 1;
error:
    return 0;
}

/* Add this to parameter-list */
void c_paramThis(
    g_generator g,
    g_file file,
    corto_bool cpp,
    corto_type parentType)
{
    corto_id classId;

    g_fullOid(g, parentType, classId);

    /* If 'cpp' is enabled, the code will be compiled with a C++ compiler.
     * Therefore, avoid using the 'this' keyword */
    if (!cpp) {
        g_fileWrite(file,
            "\n    %s this",
            c_typeptr(g, parentType, classId));
    } else {
        g_fileWrite(file,
            "\n    %s _this",
            c_typeptr(g, parentType, classId));
    }
}

/* Walk parameters of function */
int c_paramWalk(corto_object f, int(*action)(corto_parameter*, void*), void *userData) {
    corto_uint32 i;
    corto_parameterseq *params = NULL;

    if (corto_instanceof(corto_function_o, f)) {
        params = &corto_function(f)->parameters;
    } else if (corto_instanceof(corto_delegate_o, f)) {
        params = &corto_delegate(f)->parameters;
    }

    for (i = 0; i < params->length; i++) {
        if (!action(&(params->buffer[i]), userData)) {
            return 0;
        }
    }

    return 1;
}

corto_int16 c_decl(
    g_generator g,
    g_file file,
    corto_function o,
    corto_bool isWrapper,
    corto_bool cpp)
{
    corto_id fullname, functionName, signatureName, returnSpec;
    corto_type returnType;
    c_paramWalk_t walkData;

    walkData.g = g;
    walkData.firstComma = 0;
    walkData.file = file;

    /* Generate function-return type string */
    returnType = ((corto_function)o)->returnType;
    if (returnType) {
        c_typeret(g, returnType, C_ByValue, returnSpec);
    } else {
        strcpy(returnSpec, "void");
    }

    corto_fullpath(fullname, o);

    /* If the function is not a wrapper, append _v if the function is virtual */
    if (isWrapper) {
        g_fullOid(g, o, functionName);
    } else {
        c_functionName(g, o, functionName);
    }

    if (corto_function(o)->overloaded) {
        strcpy(signatureName, fullname + 1); /* Skip scope */
    } else {
        corto_signatureName(fullname + 1, signatureName);
    }

    /* Start of function */
    g_fileWrite(file, "%s _%s(", returnSpec, functionName);

    /* Add 'this' parameter to methods */
    if (c_procedureHasThis(o)) {
        corto_type thisType = corto_procedure(corto_typeof(o))->thisType;
        if (!thisType || thisType->reference) {
            c_paramThis(g, file, cpp, corto_parentof(o));
        } else {
            if (!cpp) {
                g_fileWrite(file, "corto_any this");
            } else {
                g_fileWrite(file, "corto_any _this");
            }
        }
        walkData.firstComma = 1;
    } else {
        walkData.firstComma = 0;
    }

    /* Walk parameters */
    if (!c_paramWalk(o, c_param, &walkData)) {
        goto error;
    }

    /* Append void if the argumentlist was empty */
    if (!walkData.firstComma) {
        g_fileWrite(file, "void");
    }

    /* Begin of function */
    g_fileWrite(file, ")");

    return 0;
error:
    return -1;
}

/* Get filename of mainheader */
char* c_mainheader(g_generator g, corto_id header) {
    corto_bool app = !strcmp(g_getAttribute(g, "app"), "true");
    corto_bool local = !strcmp(g_getAttribute(g, "local"), "true");

    if (!app && !local) {
        corto_id name;
        char *ptr, ch;
        strcpy(name, g_getName(g));
        for (ptr = name; (ch = *ptr); ptr++) {
            if (ch == ':') {
                *ptr = '/';
                ptr++;
                memmove(ptr, ptr + 1, strlen(ptr));
            }
        }
        sprintf(header, "%s/%s.h", name, g_getProjectName(g));
    } else {
        sprintf(header, "include/%s.h", g_getProjectName(g));
    }

    return header;
}
