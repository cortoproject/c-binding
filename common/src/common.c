/*
 * c_common.c
 *
 *  Created on: Dec 21, 2012
 *      Author: sander
 */

#include "driver/gen/c/common/common.h"
#include "ctype.h"

/* Escape language keywords */
static int c_typeKeywordEscape(corto_string inputName, corto_string buffer) {

    if( !strcmp(inputName, "alignas") ||
        !strcmp(inputName, "alignof") ||
        !strcmp(inputName, "and") ||
        !strcmp(inputName, "and_eq") ||
        !strcmp(inputName, "asm") ||
        !strcmp(inputName, "assert") ||
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
        !strcmp(inputName, "while"))
    {
        memmove(buffer + 1, inputName, strlen(inputName) + 1);
        *buffer = '_';
    } else {
        memmove(buffer, inputName, strlen(inputName) + 1);
    }
    return 0;
}

corto_string corto_genId(corto_string str, corto_id id) {
    char *ptr, ch, *idptr;

    ptr = str;
    idptr = id;

    /* Strip scope-operator for rootscope */
    if (*ptr) {
        if (*ptr == '/') {
            ptr += 1;
        }

        /* Replace '/' with '_' */
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
char* c_primitiveId(g_generator g, corto_primitive t, char* buff) {
    bool appendWidth, appendT;

    appendWidth = FALSE;
    appendT = FALSE;

    switch(t->kind) {
    case CORTO_BOOLEAN:
        strcpy(buff, "bool");
        break;
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
            corto_throw("unsupported width for primitive type '%s'.",
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
            corto_throw("unsupported width for floating point type '%s'",
                corto_fullpath(NULL, t));
            goto error;
            break;
        }
        }
        break;
    case CORTO_ENUM:
    case CORTO_BITMASK:
        corto_throw(
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
    char *ptr, *bptr, ch;

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

static char c_lastLetter(corto_string str) {
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

bool c_typeHasCaps(corto_string str) {
    char *ptr = str, ch;

    for (;(ch = *ptr); ptr ++) {
        if (isupper(ch)) {
            return TRUE;
        }
    }

    return FALSE;
}

/* Translate constant to C-language id */
char* c_constantId_intern(
    g_generator g,
    corto_constant* c,
    char* buffer,
    int mode) /* 0 = default, 1 = short */
{
    corto_enum e = corto_parentof(c);
    char *ptr, ch;
    corto_object from = root_o;
    corto_object parent = g_getCurrent(g);

    /* Determine caps strategy from first constant in enum */
    corto_string name = corto_idof(e->constants.buffer[0]);

    if (mode == 1) {
        from = corto_parentof(g_getCurrent(g));
    }

    /* Create full path for container of enumeration */
    if (!corto_isbuiltin(c)) {
        if (from != parent) {
            corto_path(buffer, from, parent, "/");
        } else {
            buffer[0] = '\0';
        }
    } else {
        if (parent == corto_secure_o) {
            strcpy(buffer, "corto_secure");
        } else if (parent == corto_native_o) {
            strcpy(buffer, "corto_native");
        } else {
            strcpy(buffer, "corto");
        }
    }

    /* If the prefix contains capitals, leave names unaltered */
    if (strlen(buffer)) {
        if (!c_typeHasCaps(buffer)) {
            /* Find last letter */
            ch = c_lastLetter(name);

            if (isupper(name[0])) {
                if (isupper(ch)) {
                    /* All caps */
                    strupper(buffer);
                } else {
                    /* Initial caps */
                    buffer[0] = toupper(buffer[0]);
                    for (ptr = buffer; (ch = *ptr); ptr ++) {
                        if (ch == '/') {
                            ptr[1] = toupper(ptr[1]);
                        }
                    }
                }
            }
        }

        for (ptr = buffer; (ch = *ptr); ptr ++) {
            if (ch == '/') {
                *ptr = '_';
            }
        }

        strcat(buffer, "_");
    }

    strcat(buffer, corto_idof(c));

    return buffer;
}

char* c_constantId(
    g_generator g,
    corto_constant* c,
    char* buffer)
{
    return c_constantId_intern(g, c, buffer, 0);
}

char* c_shortConstantId(
    g_generator g,
    corto_constant* c,
    char* buffer)
{
    return c_constantId_intern(g, c, buffer, 1);
}

/* Parse type into C-specifier */
static
int16_t c_specifierId_intern(
    g_generator g,
    corto_type t,
    char* specifier,
    bool* prefix,
    char* postfix,
    int mode) /* 0 = default, 1 = local, 2 = short */
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
    if (corto_check_attr(t, CORTO_ATTR_NAMED) && corto_childof(root_o, t)) {
        if (t->kind == CORTO_PRIMITIVE &&
            corto_primitive(t)->kind != CORTO_ENUM &&
            corto_primitive(t)->kind != CORTO_BITMASK &&
            corto_primitive(t)->kind != CORTO_TEXT)
        {
            if (!c_primitiveId(g, corto_primitive(t), specifier)) {
                goto error;
            }
        } else if (t->kind == CORTO_VOID && !t->reference) {
            strcpy(specifier, "void");
        } else {
            if (mode == 0) {
                g_fullOid(g, t, specifier);
            } else if (mode == 1) {
                g_shortOid(g, t, specifier);
            }
        }
    } else {
        switch(corto_type(t)->kind) {
        case CORTO_COLLECTION: {
            corto_id _specifier, _postfix;
            corto_type elementType = corto_collection(t)->elementType;

            /* Get specifier of elementType */
            if (elementType->kind == CORTO_PRIMITIVE && corto_check_attr(elementType, CORTO_ATTR_NAMED)) {
                if (mode == 0) {
                    g_fullOid(g, elementType, _specifier);
                } else if (mode == 1) {
                    g_shortOid(g, elementType, _specifier);
                }
            } else if (c_specifierId_intern(g, elementType, _specifier, NULL, _postfix, mode)) {
                goto error;
            }

            switch(corto_collection(t)->kind) {
            case CORTO_ARRAY:
                if ((elementType->kind == CORTO_COLLECTION) && (corto_collection(elementType)->kind == CORTO_ARRAY)) {
                    sprintf(specifier, "%s_%d", _specifier, corto_collection(t)->max);
                } else {
                    sprintf(specifier, "%sArray%d", _specifier, corto_collection(t)->max);
                }
                break;
            case CORTO_SEQUENCE:
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
                if ((elementType->kind == CORTO_COLLECTION) && (corto_collection(elementType)->kind == CORTO_LIST)) {
                    sprintf(specifier, "%s_%d", _specifier, corto_collection(t)->max);
                } else {
                    if (corto_collection(t)->max) {
                        sprintf(specifier, "%sMap%d", _specifier, corto_collection(t)->max);
                    } else {
                        sprintf(specifier, "%sMap", _specifier);
                    }
                }
            break;
                break;
            }
            break;
        }
        default: {
            if (mode == 0) {
                g_fullOid(g, t, specifier);
            } else if (mode == 1) {
                g_shortOid(g, t, specifier);
            }
            break;
        }
        }
    }

    return 0;
error:
    return -1;
}

/* Parse type into C-specifier */
corto_int16 c_specifierId(
    g_generator g,
    corto_type t,
    char* specifier,
    bool* prefix,
    char* postfix)
{
    return c_specifierId_intern(g, t, specifier, prefix, postfix, 0);
}

char* _c_typeId(g_generator g, corto_type t, char *specifier) {
    corto_id postfix;

    if (!corto_check_attr(t, CORTO_ATTR_NAMED)) {
        c_specifierId(g, t, specifier, NULL, postfix);
    } else {
        g_fullOid(g, t, specifier);
    }

    return specifier;
}

char* c_escapeString(corto_string str) {
    size_t length = stresc(NULL, 0, str);
    corto_string out = corto_alloc(length + 1);
    stresc(out, length + 1, str);
    return out;
}

bool c_procedureHasThis(corto_function o) {
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

/* Translate parameter type to C */
char* c_paramType(g_generator g, corto_parameter *p, char* buffer) {
    if (p->type->kind == CORTO_PRIMITIVE &&
        corto_primitive(p->type)->kind == CORTO_TEXT)
    {
        if (p->inout == CORTO_IN) {
            strcpy(buffer, "const char *");
        } else {
            strcpy(buffer, "char *");
        }
    }
    else
    {
        corto_id type, postfix;
        c_specifierId(g, p->type, type, NULL, postfix);
        strcpy(buffer, type);
        if (c_paramRequiresPtr(p)) {
            strcat(buffer, " *");
        }
    }
    return buffer;
}

/* Translate parameter type to C */
char* c_impl_paramType(g_generator g, corto_parameter *p, char* buffer) {
    if (p->type->kind == CORTO_PRIMITIVE &&
        corto_primitive(p->type)->kind == CORTO_TEXT)
    {
        return c_paramType(g, p, buffer);
    }
    else
    {
        corto_id type, postfix;
        c_specifierId_intern(g, p->type, type, NULL, postfix, 1);
        strcpy(buffer, type);
        if (c_paramRequiresPtr(p)) {
            strcat(buffer, " *");
        }
    }
    return buffer;
}

bool c_paramRequiresPtr(corto_parameter *p) {
    return (!p->type->reference &&
           (p->passByReference || (p->type->kind == CORTO_COMPOSITE))) ||
           p->inout == CORTO_OUT ||
           p->inout == CORTO_INOUT;
}

bool c_typeRequiresPtr(corto_type t) {
    return (!t->reference &&
           (t->kind == CORTO_COMPOSITE));
}

corto_string c_typeptr(g_generator g, corto_type t, bool impl, corto_id id) {
    corto_id postfix;
    c_specifierId_intern(g, t, id, NULL, postfix, impl);
    if (!t->reference &&
        !(t->kind == CORTO_COLLECTION &&
        (corto_collection(t)->kind == CORTO_ARRAY)))
    {
        strcat(id, "*");
    }
    return id;
}

corto_string c_typeret(g_generator g, corto_type t, c_refKind ref, bool impl, corto_id id) {
    if ((t->kind == CORTO_COLLECTION &&
        (corto_collection(t)->kind == CORTO_ARRAY)))
    {
        if (!impl) {
            g_fullOid(g, corto_collection(t)->elementType, id);
        } else {
            g_shortOid(g, corto_collection(t)->elementType, id);
        }
        strcat(id, "*");
    } else {
        corto_id postfix;
        if (ref != C_Cast) {
            c_specifierId_intern(g, t, id, NULL, postfix, impl);
        } else {
            corto_id fullId;
            if (t->kind == CORTO_PRIMITIVE) {
                g_fullOid(g, t, fullId);
            } else if (t->kind == CORTO_VOID && !t->reference) {
                strcpy(fullId, "corto_void");
            } else {
                c_specifierId_intern(g, t, fullId, NULL, postfix, impl);
            }
            strcpy(id, "_type_");
            strcat(id, fullId);
        }
        if (!t->reference && (ref == C_ByReference))
        {
            strcat(id, "*");
        }
    }

    return id;
}

corto_string c_typeval(g_generator g, corto_type t, bool impl, corto_id id) {
    corto_id postfix;
    c_specifierId_intern(g, t, id, NULL, postfix, impl);
    if (!t->reference && (t->kind == CORTO_COMPOSITE)) {
        strcat(id, "*");
    }
    return id;
}

char* c_usingName(g_generator g, corto_object o, corto_id id) {
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

char* c_usingConstant(g_generator g, corto_id id) {
    corto_id buff;
    strcpy(id, "USING_");
    char *ptr = &id[6];
    corto_object o = g_getCurrent(g);
    corto_path(buff, root_o, o, "_");
    c_typeToUpper(buff, ptr);
    return id;
}

/* Generate BUILDING macro */
char* c_buildingMacro(g_generator g, corto_id buffer) {
    corto_id buff;
    char *ptr = buffer;

    if (strcmp(g_getAttribute(g, "bootstrap"), "true") && g_getPackage(g)) {
        corto_path(buff, root_o, g_getPackage(g), "_");
    } else {
        strcpy(buff, g_getName(g));
        char *ptr, ch;
        for (ptr = buff; (ch = *ptr); ptr++) {
            if (ch == '/') *ptr = '_';
        }
    }

    strupper(buff);
    strcpy(ptr, buff);
    strcat(ptr, "_IMPL");
    return buffer;
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
    strupper(upperName);
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
            corto_iter it = corto_ll_iter(g->imports);
            while (corto_iter_hasNext(&it)) {
                corto_object o = corto_iter_next(&it);
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
    bool cpp = !strcmp(g_getAttribute(g, "c4cpp"), "true");

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

void c_includeFrom_toBufferv(
    g_generator g,
    corto_buffer *buffer,
    corto_object o,
    corto_string include,
    va_list list)
{
    corto_id path, filebuff;
    corto_object package = NULL;

    if (o) {
        package = c_findPackage(g, o);
    }

    vsprintf(filebuff, include, list);

    /* If an app or local project and the object to include is from this project
     * there is no need to prefix the header with a path */
    if ((!strcmp(g_getAttribute(g, "local"), "true") ||
         !strcmp(g_getAttribute(g, "app"), "true")) &&
        (g_getPackage(g) == package))
    {
        /* Never include headers by just their names, as this could potentially
         * conflict with system headers, for example when a user creates a class
         * called "time". */
        corto_buffer_append(buffer, "#include <include/%s>\n", filebuff);
    } else {
        if (package) {
            corto_path(path, root_o, package, "/");
        } else {
            strcpy(path, g_getName(g));
        }
        corto_buffer_append(buffer, "#include <%s/%s>\n",
          path,
          filebuff);
    }
}

void c_includeFrom_toBuffer(
    g_generator g,
    corto_buffer *buffer,
    corto_object o,
    corto_string include,
    ...)
{
    va_list args;
    va_start(args, include);
    c_includeFrom_toBufferv(g, buffer, o, include, args);
    va_end(args);
}

void c_includeFrom(
    g_generator g,
    g_file file,
    corto_object o,
    corto_string include,
    ...)
{
    corto_buffer buffer = CORTO_BUFFER_INIT;

    va_list args;
    va_start(args, include);
    c_includeFrom_toBuffer(
        g,
        &buffer,
        o,
        include,
        args);
    va_end(args);

    char *str = corto_buffer_str(&buffer);
    g_fileWrite(file, str);
    free(str);
}

int c_includeDependencies(g_generator g, g_file result, corto_string header) {
    int count = 0;

    /* Add include files of managed dependencies with models */
    corto_iter it = corto_ll_iter(g->imports);
    while (corto_iter_hasNext(&it)) {
        corto_object import = corto_iter_next(&it);
        if (import != corto_o) {
            corto_id import_id;
            corto_path(import_id, root_o, import, "/");
            const char *include = corto_locate(
                import_id, NULL, CORTO_LOCATE_INCLUDE);

            if (header) {
                if (corto_file_test(strarg("%s/%s", include, header))) {
                    g_fileWrite(result, "#include <%s/%s>\n", import_id, header);
                    count ++;
                }
            } else {
                char *import_name = strrchr(import_id, '/');
                if (import_name) {
                    import_name ++;
                } else {
                    import_name = import_id;
                }
                if (corto_file_test(strarg("%s/%s.h", include, import_name))) {
                    g_fileWrite(result, "#include <%s/%s.h>\n", import_id, import_name);
                    count ++;
                }
            }
        }
    }
    return count;
}

void c_include_toBuffer(g_generator g, corto_buffer *buffer, corto_object o) {
    corto_id name;
    corto_object package = c_findPackage(g, o);

    corto_assert (package != NULL, "can't include '%s' from non-package scopes",
        corto_fullpath(NULL, o));

    c_includeFrom_toBuffer(g, buffer, package, c_filename(g, name, o, "h"));
}

void c_include(g_generator g, g_file file, corto_object o) {
    corto_id name;
    corto_object package = c_findPackage(g, o);

    corto_assert (package != NULL, "can't include '%s' from non-package scopes",
        corto_fullpath(NULL, o));

    c_includeFrom(g, file, package, c_filename(g, name, o, "h"));
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
    if (corto_check_attr(o, CORTO_ATTR_NAMED)) {
        return 1;
    } else {
        if (corto_instanceof(corto_collection_o, o) && corto_instanceof(corto_collection_o, userData)) {
            return c_compareCollections(
                corto_collection(o),
                corto_collection(userData)) != CORTO_EQ;
        } else if (corto_instanceof(corto_iterator_o, o) && corto_instanceof(corto_iterator_o, userData)) {
            return corto_iterator(o)->elementType ==
                corto_iterator(userData)->elementType;
        } else {
            return 1;
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
        if (!corto_ll_count(data->result) || corto_ll_walk(data->result, c_checkDuplicates, o)) {
            corto_ll_append(data->result, o);
        }
    }

    return 1;
}

corto_ll c_findType(g_generator g, corto_class t) {
    c_findTypeWalk_t walkData;

    walkData.result = corto_ll_new();
    walkData.t = t;

    if (corto_genDepWalk(g, c_findTypeWalk, NULL, &walkData)) {
        corto_throw("failed to resolve instances of '%s'",
            corto_fullpath(NULL, t));
        goto error;
    }

    return walkData.result;
error:
    return NULL;
}

static
char* c_varId_intern(
    g_generator g,
    corto_object o,
    char* out,
    int mode) /* 0 = default, 1 = local, 2 = short */
{
    if (o != root_o) {
        /* Using fully scoped name for package variables allows using
         * packages with the same name */
        if (corto_instanceof(corto_package_o, o)) {
            corto_path(out, root_o, o, "_");
        } else {
            corto_id postfix;
            if (corto_instanceof(corto_type_o, o) && (!corto_check_attr(o, CORTO_ATTR_NAMED) || !corto_childof(root_o, o))) {
                c_specifierId_intern(g, o, out, NULL, postfix, mode);
            } else {
                if (mode == 0) {
                    g_fullOid(g, o, out);
                } else if (mode == 1) {
                    g_shortOid(g, o, out);
                } else if (mode == 2) {
                    g_shortOid(g, o, out);
                }
            }
        }
        strcat(out, "_o");
    } else {
        strcpy(out, "root_o");
    }

    return out;
}

char* c_varId(g_generator g, corto_object o, char* out) {
    return c_varId_intern(g, o, out, 0);
}

char* c_impl_varId(g_generator g, corto_object o, char* out) {
    return c_varId_intern(g, o, out, 1);
}

char* c_short_varId(g_generator g, corto_object o, char* out) {
    return c_varId_intern(g, o, out, 2);
}

char* c_functionName(g_generator g, corto_function o, corto_id id) {
    g_fullOid(g, o, id);
    if (o->overridable) {
        strcat(id, "_v");
    }
    return id;
}

char* c_functionLocalName(g_generator g, corto_function o, corto_id id) {
    g_shortOid(g, o, id);
    if (o->overridable) {
        strcat(id, "_v");
    }
    return id;
}

typedef struct c_paramWalk_t {
    g_generator g;
    bool firstComma;
    bool impl;
    corto_buffer *buffer;
} c_paramWalk_t;

/* Generate parameters for method */
static
int c_param(
    corto_parameter *o,
    void *userData)
{
    c_paramWalk_t* data;
    corto_id type, name;

    data = userData;

    /* Write comma */
    if (data->firstComma) {
        corto_buffer_appendstr(data->buffer, ",\n    ");
    } else {
        corto_buffer_appendstr(data->buffer, "\n    ");
    }

    if (!data->impl) {
        c_paramType(data->g, o, type);
    } else {
        c_impl_paramType(data->g, o, type);
    }

    corto_buffer_append(data->buffer, "%s", type);
    if (type[strlen(type) - 1] != '*') {
        corto_buffer_appendstr(data->buffer, " ");
    }

    /* Write to source */
    corto_buffer_append(data->buffer, "%s", c_paramName(o->name, name));

    data->firstComma++;

    return 1;
}

/* Add this to parameter-list */
void c_paramThis(
    g_generator g,
    corto_buffer *buffer,
    bool cpp,
    bool impl,
    corto_type parentType)
{
    corto_id classId;

    g_fullOid(g, parentType, classId);

    /* If 'cpp' is enabled, the code will be compiled with a C++ compiler.
     * Therefore, avoid using the 'this' keyword */
    if (!cpp) {
        corto_buffer_append(buffer,
            "\n    %s this",
            c_typeptr(g, parentType, impl, classId));
    } else {
        corto_buffer_append(buffer,
            "\n    %s _this",
            c_typeptr(g, parentType, impl, classId));
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
    corto_buffer *buffer,
    corto_function o,
    bool isWrapper,
    bool cpp,
    bool impl)
{
    corto_id functionName, returnSpec;
    corto_type returnType;
    c_paramWalk_t walkData;

    walkData.g = g;
    walkData.firstComma = 0;
    walkData.buffer = buffer;
    walkData.impl = impl;

    /* Generate function-return type string */
    returnType = ((corto_function)o)->returnType;
    if (returnType) {
        c_typeret(g, returnType, C_ByValue, impl, returnSpec);
    } else {
        strcpy(returnSpec, "void");
    }

    if (impl) {
        c_short_id(g, functionName, o);
    } else {
        c_id(g, functionName, o);
    }
    if (isWrapper && o->overridable) {
        /* Remove _v */
        functionName[strlen(functionName) - 2] = '\0';
    }

    /* Start of function */
    if (impl) {
        corto_buffer_append(buffer, "%s %s(", returnSpec, functionName);
    } else {
        corto_buffer_append(buffer, "%s _%s(", returnSpec, functionName);
    }

    /* Add 'this' parameter to methods */
    if (c_procedureHasThis(o)) {
        corto_type thisType = corto_procedure(corto_typeof(o))->thisType;
        if (!thisType || thisType->reference) {
            c_paramThis(g, buffer, cpp, impl, corto_parentof(o));
        } else {
            if (!cpp) {
                corto_buffer_appendstr(buffer, "corto_any this");
            } else {
                corto_buffer_appendstr(buffer, "corto_any _this");
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
        corto_buffer_appendstr(buffer, "void");
    }

    /* Begin of function */
    corto_buffer_appendstr(buffer, ")");

    return 0;
error:
    return -1;
}

/* Get filename of mainheader */
char* c_mainheader(g_generator g, corto_id header) {
    bool app = !strcmp(g_getAttribute(g, "app"), "true");
    bool local = !strcmp(g_getAttribute(g, "local"), "true");

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

/* Get filename of mainheader */
char* c_mainheaderImpl(g_generator g, corto_id header) {

    if (g_getCurrent(g)) {
        sprintf(header, "src/%s.h", g_getProjectName(g));
    } else {
        c_mainheader(g, header);
    }

    return header;
}

/* Open generated header file */
g_file c_headerOpen(
    g_generator g,
    const char *name)
{
    bool bootstrap = !strcmp(g_getAttribute(g, "bootstrap"), "true");
    corto_id filename, path, upper_name;
    sprintf(filename, "_%s.h", name);

    if (bootstrap) {
        corto_path(path, root_o, g_getCurrent(g), "_");
    } else {
        corto_path(path, root_o, g_getPackage(g), "_");
    }
    strupper(path);

    strcpy(upper_name, name);
    strupper(upper_name);

    g_file result = g_fileOpen(g, filename);

    /* Print standard comments and includes */
    g_fileWrite(result, "/* %s\n", filename);
    g_fileWrite(result, " * This file is generated. Do not modify its contents.\n");
    g_fileWrite(result, " */\n\n");
    g_fileWrite(result, "#ifndef %s_%s_H\n", path, upper_name);
    g_fileWrite(result, "#define %s_%s_H\n\n", path, upper_name);

    return result;
}

/* Close generated header file */
void c_headerClose(
    g_file file)
{
    g_fileWrite(file, "#endif\n\n");
    g_fileClose(file);
}

/* Get identifier for language object */
static
char* c_id_intern(
    g_generator g,
    corto_id id,
    corto_object o,
    int mode) /* 0 = default, 1 = local, 2 = short */
{
    corto_object parent = corto_parentof(o);
    if (corto_instanceof(corto_constant_o, o) && parent && corto_instanceof(corto_enum_o, parent)) {
        switch (mode) {
        case 0: return c_constantId(g, o, id);
        case 1: return c_shortConstantId(g, o, id); /* short constants not supported */
        default: return NULL;
        }
    } else {
        if (corto_instanceof(corto_type_o, o) && !corto_check_attr(o, CORTO_ATTR_NAMED)) {
            corto_id postfix;
            c_specifierId_intern(g, o, id, NULL, postfix, mode);
        } else {
            switch(mode) {
            case 0: g_fullOid(g, o, id); break;
            case 1: g_shortOid(g, o, id); break;
            default: return NULL;
            }
        }
        if (corto_instanceof(corto_function_o, o) &&
            ((corto_function)o)->overridable)
        {
            strcat(id, "_v");
        }
        return id;
    }
}

char* c_id(
    g_generator g,
    corto_id id,
    corto_object o)
{
    return c_id_intern(g, id, o, 0);
}

/* Get short identifier for language object */
char* c_short_id(
    g_generator g,
    corto_id id,
    corto_object o)
{
    return c_id_intern(g, id, o, 1);
}

/* Function builds a scope-stack from root to module */
static void
cpp_scopeStack(
    corto_object module,
    corto_object* stack /* corto_object[SD_MAX_SCOPE_DEPTH] */)
{
    corto_uint32 count;
    corto_object ptr;

    corto_assert(module != NULL, "NULL passed for module to sd_utilModuleStack");

    /* Count scope depth */
    ptr = module;
    count = 1; /* For self */
    while((ptr = corto_parentof(ptr))) {
        count++;
    }

    if(count > CORTO_MAX_SCOPE_DEPTH) {
        corto_error("cpp_scopeStack: unsupported scope-depth (depth=%d, max=%d).", count, CORTO_MAX_SCOPE_DEPTH);
    }
    corto_assert(count <= CORTO_MAX_SCOPE_DEPTH, "MAX_SCOPE_DEPTH overflow.");

    /* Fill module stack */
    ptr = module;
    while(count) {
        stack[count-1] = ptr;
        ptr = corto_parentof(ptr);
        count--;
    }

    /* ptr should be NULL */
    corto_assert(!ptr, "ptr is NULL.");
}

/* Find first common module in two module-stacks */
static corto_object
cpp_commonScope(
    corto_object from,
    corto_object to,
    corto_object* fromStack,
    corto_object* toStack,
    corto_uint32* i_out)
{
    corto_object fromPtr, toPtr;
    corto_uint32 i;

    /* fromPtr and toPtr will initially point to base */
    i = 0;
    do {
        fromPtr = fromStack[i];
        toPtr = toStack[i];
        i++;
    }while((fromPtr != from) && (toPtr != to) && (fromStack[i] == toStack[i]));

    /* Common module is now stored in fromPtr and toPtr. */

    if(i_out) {
        *i_out = i;
    }

    return fromPtr;
}

/* Check whether a type translates to a native construct or may act as a C++ namespace */
bool
cpp_nativeType(corto_object o) {
	bool result = FALSE;

	if(corto_class_instanceof(corto_type_o, o)) {
		switch(corto_type(o)->kind) {
		case CORTO_VOID:
		    if(corto_type(o)->reference) {
		        result = TRUE;
		    }
			break;
		default:
			result = TRUE;
			break;
		}
	}

	return result;
}

/* Open a scope */
void
cpp_openScope(
    g_file file,
    bool namespaceOnly,
    corto_object to)
{
    corto_object from;

    if (to) {

        /* Do not open namespaces for non-void type-scopes */
        if (namespaceOnly) {
            while(cpp_nativeType(to)) {
            	to = corto_parentof(to);
            }
        }

        /* If context->module is NULL, start from root */
        from = g_fileScopeGet(file);
        if(!from) {
            from = root_o;
        }

        /* If from and to are not equal, find shortest path between modules. */
        if(from != to) {
            corto_object fromStack[CORTO_MAX_SCOPE_DEPTH], toStack[CORTO_MAX_SCOPE_DEPTH];
            corto_object fromPtr, toPtr;
            corto_uint32 i;

            /* Find common module. First build up a scope-stack for the two modules which
             * are ordered base -> <module>. Then walk through these stacks to find the
             * last common module. */
            cpp_scopeStack(from, fromStack);
            cpp_scopeStack(to, toStack);
            fromPtr = toPtr = cpp_commonScope(from, to, fromStack, toStack, &i);

            /* Walk down from module 'from' to 'toPtr' */
            fromPtr = from;
            while(fromPtr != toPtr) {
                g_fileWrite(file, "}\n");
                fromPtr = corto_parentof(fromPtr);
            }

            /* Walk from toPtr to 'to' */
            while(toPtr != to) {
                corto_id id;
                toPtr = toStack[i];
                g_fileWrite(file, "namespace %s {\n", g_oid(g_fileGetGenerator(file), toPtr, id));
                i++;
            }

            /* Update context->module */
            g_fileScopeSet(file, to);
        }
    }
}

void cpp_closeScope(g_file file) {
    corto_object ptr;

    if((ptr = g_fileScopeGet(file))) {
        while((ptr = corto_parentof(ptr))) {
            g_fileWrite(file, "}\n");
        }

        g_fileScopeSet(file, NULL);
    }
}
