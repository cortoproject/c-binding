
#include <corto/corto.h>
#include "driver/gen/c/common/common.h"

static
uint32_t c_projectCountPackagesToLoad(
    g_generator g)
{
    uint32_t result = 0;
    if (g->imports && corto_ll_count(g->imports)) {
        corto_iter iter = corto_ll_iter(g->imports);
        while (corto_iter_hasNext(&iter)) {
            corto_object o = corto_iter_next(&iter);
            /* Filter out generated language packages */
            if (strcmp(corto_idof(o), "c")) {
                result ++;
            }
        }
    }
    return result;
}

/* Load dependencies */
static
void c_projectLoadPackages(
    g_generator g,
    g_file file)
{
    if (g->imports && c_projectCountPackagesToLoad(g)) {
        corto_id id;
        if (g_getPackage(g)) {
            corto_path(id, root_o, g_getPackage(g), "_");
        } else {
            strcpy(id, g_getProjectName(g));
        }
        g_fileWrite(file, "corto_log_push(\"load-deps:%s\");\n", id);
        corto_iter iter = corto_ll_iter(g->imports);
        while (corto_iter_hasNext(&iter)) {
            corto_object o = corto_iter_next(&iter);

            /* Filter out generated language packages */
            if (strcmp(corto_idof(o), "c") && strcmp(corto_idof(o), "cpp")) {
                g_fileWrite(
                    file,
                    "if (corto_use(\"%s\", 0, NULL)) {corto_log_pop(); corto_raise(); return -1;}\n",
                    corto_path(NULL, NULL, o, "/"));
            }
        }
        g_fileWrite(file, "corto_log_pop();\n");
    }
}

/* Load dependencies */
static
void c_projectLoadGeneratedPackages(
    g_generator g,
    g_file file)
{
    if (g->imports && c_projectCountPackagesToLoad(g)) {
        corto_id id;
        if (g_getPackage(g)) {
            corto_path(id, root_o, g_getPackage(g), "_");
        } else {
            strcpy(id, g_getProjectName(g));
        }
        g_fileWrite(file, "corto_log_push(\"load-gen-deps:%s\");\n", id);
        corto_iter iter = corto_ll_iter(g->imports);
        while (corto_iter_hasNext(&iter)) {
            corto_object o = corto_iter_next(&iter);

            /* Filter out generated language packages */
            if (!strcmp(corto_idof(o), "c") || !strcmp(corto_idof(o), "cpp")) {
                g_fileWrite(
                    file,
                    "if (corto_use(\"%s\", 0, NULL)) {corto_log_pop(); corto_raise(); return -1;}\n",
                    corto_path(NULL, NULL, o, "/"));
            } else {
                continue;
            }
        }
        g_fileWrite(file, "corto_log_pop();\n");
    }

    /* If a c4cpp package, load own generated cpp package */
    bool cpp = !strcmp(g_getAttribute(g, "c4cpp"), "true");
    bool local = !strcmp(g_getAttribute(g, "local"), "true");
    bool app = !strcmp(g_getAttribute(g, "app"), "true");

    if (cpp && g_getCurrent(g) && !local && !app) {
        corto_id id;
        if (g_getPackage(g)) {
            corto_path(id, NULL, g_getPackage(g), "/");
        } else {
            strcpy(id, g_getProjectName(g));
        }

        g_fileWrite(
            file,
            "if (corto_use(\"%s/cpp\", 0, NULL)) {corto_raise(); return -1;}\n",
            id);
    }
}

/* Generate file containing loader */
static
int16_t c_projectGenerateMainFile(
    g_generator g)
{
    corto_id filename;
    g_file file;
    bool app = !strcmp(g_getAttribute(g, "app"), "true");
    bool cpp = !strcmp(g_getAttribute(g, "c4cpp"), "true");
    bool local = !strcmp(g_getAttribute(g, "local"), "true");
    bool cppbinding = !strcmp(g_getAttribute(g, "lang"), "cpp");

    sprintf(filename, "_project.%s", cpp ? "cpp" : "c");

    file = g_hiddenFileOpen(g, filename);
    if(!file) {
        goto error;
    }

    g_fileWrite(file, "/* %s\n", filename);
    g_fileWrite(file, " *\n");
    g_fileWrite(file, " * This file contains the project entry function. Here, we\n");
    g_fileWrite(file, " * start/stop corto and load all dependencies, before and after\n");
    g_fileWrite(file, " * invoking the main function of your project.\n");
    g_fileWrite(file, " */\n\n");

    corto_id header;

    if (!cppbinding) {
        g_fileWrite(file, "#include <%s>\n", c_mainheader(g, header));
    } else {
        g_fileWrite(file, "#include <%spp>\n", c_mainheader(g, header));
    }

    if (!app && g_getPackage(g)) {
        c_includeFrom(g, file, g_getPackage(g), "_project.h");
    }

    if (g_getCurrent(g)) {
        g_fileWrite(file, "int %s_load(void);\n",
            corto_path(NULL, root_o, g_getPackage(g), "_"));
        if ((app | local) && cpp) {
            g_fileWrite(file, "int %s_cppinit(void);\n",
                corto_path(NULL, root_o, g_getPackage(g), "_"));
        }
        g_fileWrite(file, "\n");
    }

    g_fileWrite(file, "#ifdef __cplusplus\n");
    g_fileWrite(file, "extern \"C\"\n");
    g_fileWrite(file, "#endif\n");
    if (g_getCurrent(g)) {
        c_writeExport(g, file);
        g_fileWrite(file, "\n");
    }

    g_fileWrite(file, "int %s(int argc, char* argv[]) {\n", app ? "main" : "cortoinit");
    g_fileIndent(file);
    if (app) g_fileWrite(
        file,
        "corto_start(\"%s\"); /* Pass application name for logging framework */\n",
        g_getName(g));

    /* Load dependencies before loading main package */
    c_projectLoadPackages(g, file);

    g_fileWrite(file, "int ret = 0;\n");
    if (g_getCurrent(g)) {
        corto_id id;
        corto_path(id, root_o, g_getPackage(g), "_");
        g_fileWrite(file, "corto_log_push(\"load-model:%s\");\n", id);
        g_fileWrite(file, "ret = %s_load();\n", id);
        g_fileWrite(file, "corto_log_pop();\n");
    }

    /* If a C++ application that has a package, initialize C++ variables */
    if (app && cpp && g_getCurrent(g)) {
        g_fileWrite(file, "%s_cppinit();\n",
            corto_path(NULL, root_o, g_getPackage(g), "_"));
    }

    /* Load generated language packages after loading main package */
    c_projectLoadGeneratedPackages(g, file);

    if (app) {
        /* Load configuration only for application projects */
        g_fileWrite(file, "corto_load_config();\n");
        g_fileWrite(file, "if (!ret) {\n");
        g_fileIndent(file);
        g_fileWrite(file, "int cortomain(int argc, char *argv[]);\n");
        g_fileWrite(file, "ret = cortomain(argc, argv);\n");
        g_fileDedent(file);
        g_fileWrite(file, "}\n");
        g_fileWrite(file, "char *keep_alive = corto_getenv(\"CORTO_KEEP_ALIVE\");\n");
        g_fileWrite(file, "if (keep_alive && !stricmp(keep_alive, \"true\")) {\n");
        g_fileIndent(file);
        g_fileWrite(file, "corto_info(\"Keeping process alive, press CTRL-C to exit\");\n");
        g_fileWrite(file, "while (true) { corto_sleep(1, 0); }\n");
        g_fileDedent(file);
        g_fileWrite(file, "}\n");
        g_fileWrite(file, "corto_stop();\n");
    }
    g_fileWrite(file, "return ret;\n");
    g_fileDedent(file);
    g_fileWrite(file, "}\n\n");

    return 0;
error:
    return -1;
}

/* Generate interface header with macro's for exporting */
static
int16_t c_genInterfaceHeader(
    g_generator g)
{
    corto_id interfaceHeaderName;

    sprintf(interfaceHeaderName, "_project.h");

    g_file interfaceHeader = g_fileOpen(g, interfaceHeaderName);

    if (!interfaceHeader) {
        goto error;
    } else {
        corto_id upperName;
        strcpy(upperName, g_getName(g));
        strupper(upperName);

        corto_id upperFullName, buildingMacro;
        if (g_getCurrent(g)) {
            corto_path(upperFullName, root_o, g_getCurrent(g), "_");
        } else {
            strcpy(upperFullName, g_getName(g));
            char *ptr, ch;
            for (ptr = upperFullName; (ch = *ptr); ptr++) {
                if (ch == '/') *ptr = '_';
            }
        }
        strupper(upperFullName);

        c_buildingMacro(g, buildingMacro);

        g_fileWrite(interfaceHeader, "/* %s\n", interfaceHeaderName);
        g_fileWrite(interfaceHeader, " * This file is generated. Do not modify the contents of this file.\n");
        g_fileWrite(interfaceHeader, " */\n\n");

        g_fileWrite(interfaceHeader, "#if %s && defined _MSC_VER\n", buildingMacro);
        g_fileWrite(interfaceHeader, "#define ");
        c_writeExport(g, interfaceHeader);
        g_fileWrite(interfaceHeader, " __declspec(dllexport)\n", upperFullName);
        g_fileWrite(interfaceHeader, "#elif %s\n", buildingMacro);
        g_fileWrite(interfaceHeader, "#define ");
        c_writeExport(g, interfaceHeader);
        g_fileWrite(interfaceHeader, " __attribute__((__visibility__(\"default\")))\n");
        g_fileWrite(interfaceHeader, "#elif defined _MSC_VER\n");
        g_fileWrite(interfaceHeader, "#define ");
        c_writeExport(g, interfaceHeader);
        g_fileWrite(interfaceHeader, " __declspec(dllimport)\n");
        g_fileWrite(interfaceHeader, "#else\n");
        g_fileWrite(interfaceHeader, "#define ");
        c_writeExport(g, interfaceHeader);
        g_fileWrite(interfaceHeader, "\n");
        g_fileWrite(interfaceHeader, "#endif\n\n");
    }

    return 0;
error:
    return -1;
}

/* Generator main */
int genmain(g_generator g) {

    /* Create source and include directories */
    corto_mkdir("include");
    corto_mkdir("src");

    if (c_projectGenerateMainFile(g)) {
        if (!corto_raised()) {
            corto_throw("failed to create main sourcefile");
        }
        goto error;
    }

    if (c_genInterfaceHeader(g)) {
        if (!corto_raised()) {
            corto_throw("failed to create interface header");
        }
        goto error;
    }

    return 0;
error:
    return -1;
}
