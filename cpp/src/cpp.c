
#include <driver.gen.c.cpp>

typedef struct c_binding_t {
    g_generator g;
    g_file header;
    g_file source;
} c_binding_t;

static
int c_binding_cppTypedefs(
    corto_object o,
    void *userData)
{
    c_binding_t* data = userData;

    if (corto_instanceof(corto_type_o, o) &&
        corto_check_attr(o, CORTO_ATTR_NAMED) &&
        corto_childof(root_o, o) &&
        o != g_getCurrent(data->g))
    {
        corto_id id, cpp_id, meta_id;
        c_id(data->g, id, o);
        c_typeId(data->g, corto_typeof(o), meta_id);
        corto_path(cpp_id, g_getCurrent(data->g), o, "_");
        g_fileWrite(data->header, "extern %s %s_o;\n", meta_id, cpp_id);
        if (!corto_instanceof(corto_native_type_o, o)) {
            c_escape_keyword(cpp_id, cpp_id);
            g_fileWrite(data->header, "typedef ::%s %s;\n", id, cpp_id);
        }
    }

    return 1;
}

static
int c_binding_typeSafeHelper(
    corto_object o,
    void *userData)
{
    c_binding_t* data = userData;

    if (corto_instanceof(corto_type_o, o) &&
        corto_check_attr(o, CORTO_ATTR_NAMED) &&
        corto_childof(root_o, o) &&
        o != g_getCurrent(data->g))
    {
        corto_id cpp_id, id, ptr_id, meta_id;
        c_typeptr(data->g, o, false, ptr_id);
        c_typeId(data->g, corto_typeof(o), meta_id);
        c_id(data->g, id, o);
        corto_path(cpp_id, g_getCurrent(data->g), o, "_");

        g_fileWrite(data->header, "class %s_t {\n", cpp_id);
        g_fileWrite(data->header, "public:\n");
        g_fileIndent(data->header);
        if (!corto_instanceof(corto_native_type_o, o)) {
            g_fileWrite(data->header, "typedef ::%s _ref;\n", ptr_id);
        } else {
            g_fileWrite(data->header, "typedef %s _ref;\n", ptr_id);
        }
        g_fileWrite(data->header, "static %s _o;\n", meta_id);
        g_fileDedent(data->header);
        g_fileWrite(data->header, "};\n");
    }

    return 1;
}

static
char* c_binding_cppId(
    g_generator g,
    char *buffer,
    corto_object o)
{
    corto_object stack[CORTO_MAX_SCOPE_DEPTH];
    int32_t i;
    cpp_scopeStack(o, stack);
    buffer[0] = '\0';
    for (i = 1; stack[i] != NULL; i ++) {
        corto_id id;
        if (i > 1) strcat(buffer, "::");
        strcat(buffer, g_oid(g, stack[i], id));
    }
    return buffer;
}

static
int c_binding_cppDeclareVariables(
    corto_object o,
    void *userData)
{
    c_binding_t* data = userData;

    if (corto_instanceof(corto_type_o, o) &&
        corto_check_attr(o, CORTO_ATTR_NAMED) &&
        corto_childof(root_o, o) &&
        o != g_getCurrent(data->g))
    {
        corto_id cpp_id, cpp_prefix, ptr_id, meta_id;
        c_typeptr(data->g, o, false, ptr_id);
        corto_path(cpp_id, g_getCurrent(data->g), o, "_");
        c_typeId(data->g, corto_typeof(o), meta_id);

        c_binding_cppId(data->g, cpp_prefix, g_getCurrent(data->g));
        g_fileWrite(data->source, "%s %s::%s_t::_o;\n", meta_id, cpp_prefix, cpp_id);
        g_fileWrite(data->source, "%s %s::%s_o;\n", meta_id, cpp_prefix, cpp_id);
    }

    return 1;
}

static
int c_binding_cppAssignVariables(
    corto_object o,
    void *userData)
{
    c_binding_t* data = userData;

    if (corto_instanceof(corto_type_o, o) &&
        corto_check_attr(o, CORTO_ATTR_NAMED) &&
        corto_childof(root_o, o) &&
        o != g_getCurrent(data->g))
    {
        corto_id cpp_id, cpp_prefix, id, ptr_id, fullId, meta_id;
        c_typeptr(data->g, o, false, ptr_id);
        c_id(data->g, id, o);
        corto_fullpath(fullId, o);
        corto_path(cpp_id, g_getCurrent(data->g), o, "_");
        c_typeId(data->g, corto_typeof(o), meta_id);
        c_binding_cppId(data->g, cpp_prefix, g_getCurrent(data->g));

        g_fileWrite(data->source,
            "%s::%s_o = static_cast<%s>(corto_resolve(NULL, \"%s\"));\n",
            cpp_prefix,
            cpp_id,
            meta_id,
            fullId);

        g_fileWrite(data->source, "%s::%s_t::_o = %s::%s_o;\n",
            cpp_prefix, cpp_id, cpp_prefix, cpp_id);
    }

    return 1;
}

int genmain(g_generator g) {
    corto_bool local = !strcmp(g_getAttribute(g, "local"), "true");
    corto_bool app = !strcmp(g_getAttribute(g, "app"), "true");
    bool cpp = !strcmp(g_getAttribute(g, "c4cpp"), "true");
    bool bootstrap = !strcmp(g_getAttribute(g, "bootstrap"), "true");

    c_binding_t walkdata = {0};
    walkdata.g = g;

    char *cwd = ut_strdup(ut_cwd());

    if ((app || local) && !cpp) {
        ut_trace("skip generating C++ for a local non-C++ project");
        return 0;
    }

    /* Create project files */
    if (!local && !app) {
        if (!ut_file_test("cpp/project.json")) {
            corto_int8 ret, sig;
            corto_id cmd;
            sprintf(
                cmd,
                "corto create package %s.cpp --use-cpp --nobuild --silent -o cpp",
                g_getName(g));

            sig = ut_proc_cmd(cmd, &ret);
            if (sig || ret) {
                ut_throw("failed to setup project for '%s/c'", g_getName(g));
                goto error;
            }

            /* Overwrite rakefile */
            g_file rakefile = g_fileOpen(g, "cpp/project.json");
            if (!rakefile) {
                ut_throw("failed to open cpp/project.json");
                goto error;
            }
            g_fileWrite(rakefile, "{\n");
            g_fileWrite(rakefile, "    \"id\": \"%s.cpp\",\n", g_getName(g));
            g_fileWrite(rakefile, "    \"type\": \"package\",\n");
            g_fileWrite(rakefile, "    \"value\": {\n");
            g_fileWrite(rakefile, "        \"use\": [\"corto\"],\n");
            g_fileWrite(rakefile, "        \"language\": \"c++\"\n");
            g_fileWrite(rakefile, "    }\n");
            g_fileWrite(rakefile, "}\n");
            g_fileClose(rakefile);
        }

        ut_chdir("cpp");

        walkdata.header = g_fileOpen(g, "cpp.h");
        walkdata.source = g_fileOpen(g, "main.cpp");
    } else {
        walkdata.header = g_fileOpen(g, "_cpp.h");
        walkdata.source = g_hiddenFileOpen(g, "_cpp.cpp");
    }

    if (!walkdata.header) {
        goto error;
    }

    if (!walkdata.source) {
        goto error;
    }

    /* Obtain path for macro */
    corto_id path;
    corto_path(path, root_o, g_getCurrent(g), "_");
    strupper(path);

    /* Write standard comments and includes */
    g_fileWrite(walkdata.header, "/*\n");
    g_fileWrite(walkdata.header, " * Extensions for C++.\n");
    g_fileWrite(walkdata.header, " * This file contains generated code. Do not modify!\n");
    g_fileWrite(walkdata.header, " */\n\n");
    g_fileWrite(walkdata.header, "#ifndef %s__CPPAPI_H\n", path);
    g_fileWrite(walkdata.header, "#define %s__CPPAPI_H\n\n", path);
    g_fileWrite(walkdata.header, "#include <corto>\n");

    corto_object pkg;
    if (bootstrap) {
        pkg = g_getCurrent(g);
    } else {
        pkg = g_getPackage(g);
    }

    g_fileWrite(walkdata.header, "#include <%s.dir/_type.h>\n", g_getName(g));
    if (bootstrap || corto_isbuiltin(pkg)) {
    } else {
        g_fileWrite(walkdata.header, "#include \"bake_config.h\"\n");
        g_fileWrite(walkdata.header, "\n");
    }

    /* #8 Typedef types in namespace for convenience in C++ applications */
    g_fileWrite(walkdata.header, "\n/* -- C++ namespaced typedefs and objects -- */\n");
    g_fileWrite(walkdata.header, "#ifdef __cplusplus\n");
    cpp_openScope(walkdata.header, true, g_getCurrent(g));
    if (!g_walkAll(g, c_binding_cppTypedefs, &walkdata)) {
        goto error;
    }
    cpp_closeScope(walkdata.header);
    g_fileWrite(walkdata.header, "#endif\n");

    /* #9 Objects that allow for compile-time checked corto API */
    g_fileWrite(walkdata.header, "\n/* -- Helpers that enable compile-time type safety -- */\n");
    g_fileWrite(walkdata.header, "#ifdef __cplusplus\n");
    cpp_openScope(walkdata.header, true, g_getCurrent(g));
    if (!g_walkAll(g, c_binding_typeSafeHelper, &walkdata)) {
        goto error;
    }
    cpp_closeScope(walkdata.header);
    g_fileWrite(walkdata.header, "#endif\n");
    g_fileWrite(walkdata.header, "#endif\n\n");

    /* Write includes for source file */
    g_fileWrite(walkdata.source, "/*\n");
    g_fileWrite(walkdata.source, " * Extensions for C++.\n");
    g_fileWrite(walkdata.source, " * This file contains generated code. Do not modify!\n");
    g_fileWrite(walkdata.source, " */\n\n");
    if (!local && !app) {
        g_fileWrite(walkdata.source, "#include <%s.cpp>\n", g_getName(g));
    } else {
        c_includeFrom(g, walkdata.source, pkg, "_cpp.h");
    }
    g_fileWrite(walkdata.source, "\n");

    if (!g_walkAll(g, c_binding_cppDeclareVariables, &walkdata)) {
        goto error;
    }

    /* Write cortomain or cpp init, depending on whether project is an app */
    g_fileWrite(walkdata.source, "\n");
    if (!app && !local) {
        g_fileWrite(walkdata.source, "extern \"C\"\n");
        g_fileWrite(
            walkdata.source, "int cortomain(int argc, char* argv[]) {\n");
    } else {
        g_fileWrite(
            walkdata.source, "int %s_cppinit() {\n",
            corto_path(NULL, root_o, g_getPackage(g), "_"));
    }
    g_fileIndent(walkdata.source);
    if (!g_walkAll(g, c_binding_cppAssignVariables, &walkdata)) {
        goto error;
    }
    g_fileWrite(walkdata.source, "return 0;\n");
    g_fileDedent(walkdata.source);
    g_fileWrite(walkdata.source, "}\n");

    if (!local) {
        ut_chdir(cwd);
        corto_dealloc(cwd);
    }

    return 0;
error:
    return -1;
}
