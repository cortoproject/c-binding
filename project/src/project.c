
#include "corto/corto.h"
#include "corto/gen/c/common/common.h"

/* Load dependencies */
static void c_projectLoadPackages(g_file file) {
    corto_ll packages;

    if ((packages = corto_loadGetPackages())) {
        corto_iter iter = corto_llIter(packages);
        while (corto_iterHasNext(&iter)) {
            corto_string str = corto_iterNext(&iter);
            g_fileWrite(file, "if (corto_load(\"%s\", 0, NULL)) return -1;\n", str);
        }
    }
}

/* Generate file containing loader */
static corto_int16 c_projectGenerateMainFile(g_generator g) {
    corto_id filename;
    g_file file;
    corto_bool app = !strcmp(g_getAttribute(g, "app"), "true");
    corto_bool cpp = !strcmp(g_getAttribute(g, "c4cpp"), "true");
    corto_bool cppbinding = !strcmp(g_getAttribute(g, "lang"), "cpp");

    sprintf(filename, "_project.%s", cpp ? "cpp" : "c");

    file = g_hiddenFileOpen(g, filename);
    if(!file) {
        goto error;
    }

    g_fileWrite(file, "/* %s\n", filename);
    g_fileWrite(file, " *\n");
    g_fileWrite(file, " * This file is generated. Do not modify.\n");
    g_fileWrite(file, " */\n\n");

    corto_id header;

    if (!cppbinding) {
        g_fileWrite(file, "#include <%s>\n", c_mainheader(g, header));
    } else {
        g_fileWrite(file, "#include <%spp>\n", c_mainheader(g, header));
    }

    if (!app && g_getCurrent(g)) {
        c_includeFrom(g, file, g_getCurrent(g), "_project.h");
    }

    g_fileWrite(file, "\n");
    g_fileWrite(file, "int %sMain(int argc, char* argv[]);\n", g_getProjectName(g));
    g_fileWrite(file, "\n");

    if (g_getCurrent(g)) {
        g_fileWrite(file, "int %s_load(void);\n", g_getProjectName(g));
        g_fileWrite(file, "\n");
    }

    g_fileWrite(file, "#ifdef __cplusplus\n");
    g_fileWrite(file, "extern \"C\"\n");
    g_fileWrite(file, "#endif\n");
    if (g_getCurrent(g)) {
      c_writeExport(g, file);
      g_fileWrite(file, " ");
    }

    g_fileWrite(file, "int %s(int argc, char* argv[]) {\n", app ? "main" : "cortomain");
    g_fileIndent(file);
    if (app) g_fileWrite(file, "corto_start();\n");
    c_projectLoadPackages(file);
    if (g_getCurrent(g)) {
        g_fileWrite(file, "if (%s_load()) return -1;\n", g_getProjectName(g));
    }
    g_fileWrite(file, "if (%sMain(argc, argv)) return -1;\n", g_getProjectName(g));
    if (app) g_fileWrite(file, "corto_stop();\n");
    g_fileWrite(file, "return 0;\n");
    g_fileDedent(file);
    g_fileWrite(file, "}\n\n");

    return 0;
error:
    return -1;
}

/* Generate main header containing includes to dependencies */
static corto_int16 c_projectGenerateMainHeaderFile(g_generator g) {
    g_file file;
    corto_id upperName;
    corto_bool error = FALSE;
    corto_id filename;
    corto_bool cppbinding = !strcmp(g_getAttribute(g, "lang"), "cpp");
    corto_bool app = !strcmp(g_getAttribute(g, "app"), "true");
    corto_bool local = !strcmp(g_getAttribute(g, "local"), "true");

    if (!cppbinding || !app) {
        sprintf(filename, "%s.h", g_getProjectName(g));
    } else {
        sprintf(filename, "%s.hpp", g_getProjectName(g));
    }

    /* If a local package or application, prefix the macro. Otherwise there will
     * be issues including a package from an app/local project with the same
     * name. */
    upperName[0] = '\0';
    if (app || local) {
        strcpy(upperName, "_");
    }

    strcat(upperName, g_getName(g));
    corto_strupper(upperName);
    char *ptr = upperName, ch;
    while ((ch = *ptr)) {
        if (ch == '/') {
            *ptr = '_';
        }
        ptr++;
    }

    file = g_fileOpen(g, filename);
    if(!file) {
        goto error;
    }

    g_fileWrite(file, "/* %s\n", filename);
    g_fileWrite(file, " *\n");
    g_fileWrite(file, " * This file is generated. Do not modify.\n");
    g_fileWrite(file, " */\n\n");

    g_fileWrite(file, "#ifndef %s_H\n", upperName);
    g_fileWrite(file, "#define %s_H\n\n", upperName);

    c_includeFrom(g, file, corto_o, "corto.h");
    if (g_getCurrent(g)) {
        c_includeFrom(g, file, g_getCurrent(g), "_project.h");
        c_includeFrom(g, file, g_getCurrent(g), "_type.h");
    }
    g_fileWrite(file, "\n");

    if (g->imports) {
        corto_iter iter = corto_llIter(g->imports);
        while (corto_iterHasNext(&iter)) {
            corto_object import = corto_iterNext(&iter);
            corto_string str = corto_path(NULL, NULL, import, "/");
            corto_string package = corto_locate(str, CORTO_LOCATION_FULLNAME);
            if (!package) {
                corto_seterr("project configuration contains unresolved package '%s'", str);

                /* Don't break out of generation here, as this will mess up the
                 * file's code snippet */
                error = TRUE;
            } else {
                corto_string name = corto_locate(str, CORTO_LOCATION_NAME);
                g_fileWrite(file, "#include <%s/%s.%s>\n", package, name, cppbinding ? "hpp" : "h");
                corto_dealloc(name);
                corto_dealloc(package);
            }
        }
    }

    g_fileWrite(file, "/* $header()");

    /* Lookup the header snippet */
    corto_string snippet = g_fileLookupHeader(file, "");
    if (snippet) {
        g_fileWrite(file, snippet);
    } else {
        g_fileWrite(file, " */\n");
        g_fileWrite(file, "/* You can put your own definitions here! */\n");
        g_fileWrite(file, "/* ");
    }
    g_fileWrite(file, "$end */\n");

    g_fileWrite(file, "\n");

    g_fileWrite(file, "#ifdef __cplusplus\n");
    g_fileWrite(file, "extern \"C\" {\n");
    g_fileWrite(file, "#endif\n");

    g_fileWrite(file, "\n");

    g_fileWrite(file, "#ifdef __cplusplus\n");
    g_fileWrite(file, "}\n");
    g_fileWrite(file, "#endif\n");
    g_fileWrite(file, "#endif\n\n");

    g_fileClose(file);

    return error ? -1 : 0;
error:
    return -1;
}

typedef struct c_projectCleanInclude_t {
    g_generator g;
    g_file file;
} c_projectCleanInclude_t;

static int c_projectCleanInclude(corto_object o, void *userData) {
    c_projectCleanInclude_t *data = userData;

    if ((o != g_getCurrent(data->g)) &&
        (corto_instanceof(corto_type(corto_interface_o), o) ||
        corto_instanceof(corto_type(corto_package_o), o))) {
        corto_id id;
        g_fileWrite(
          data->file, "CLOBBER.include(\"include/%s\")\n",
          c_filename(data->g, id, o, "h"));
    }

    return 1;
}

/* Generate dependency makefile for project */
static corto_int16 c_projectGenerateDepMakefile(g_generator g) {
    g_file file;
    c_projectCleanInclude_t walkData;

    file = g_hiddenFileOpen(g, "dep.rb");
    if(!file) {
        goto error;
    }

    g_fileWrite(file, "require 'rake/clean'\n");

    g_fileWrite(file, "\n");
    g_fileWrite(file, "# Clobber generated header files\n");
    walkData.file = file;
    walkData.g = g;
    g_fileWrite(file, "\n");
    g_walkRecursive(g, c_projectCleanInclude, &walkData);
    g_fileWrite(file, "CLOBBER.include(\".corto/dep.rb\")\n");

    return 0;
error:
    return -1;
}

/* Generate interface header with macro's for exporting */
static corto_int16 c_genInterfaceHeader(g_generator g) {
    corto_id interfaceHeaderName;

    if (g_getCurrent(g)) {
        sprintf(interfaceHeaderName, "_project.h");
    } else {
        sprintf(interfaceHeaderName, "%s/_project.h", g_getName(g));
    }

    g_file interfaceHeader = g_fileOpen(g, interfaceHeaderName);

    if (!interfaceHeader) {
        goto error;
    } else {
        corto_id upperName;
        strcpy(upperName, g_getName(g));
        corto_strupper(upperName);

        corto_id upperFullName;
        if (g_getCurrent(g)) {
            corto_path(upperFullName, root_o, g_getCurrent(g), "_");
        } else {
            strcpy(upperFullName, g_getName(g));
            char *ptr, ch;
            for (ptr = upperFullName; (ch = *ptr); ptr++) {
                if (ch == '/') *ptr = '_';
            }
        }
        corto_strupper(upperFullName);

        g_fileWrite(interfaceHeader, "/* %s\n", interfaceHeaderName);
        g_fileWrite(interfaceHeader, " *\n");
        g_fileWrite(interfaceHeader, " * This file contains generated code. Do not modify!\n");
        g_fileWrite(interfaceHeader, " */\n\n");

        g_fileWrite(interfaceHeader, "#if BUILDING_%s && defined _MSC_VER\n", upperFullName);
        g_fileWrite(interfaceHeader, "#define ");
        c_writeExport(g, interfaceHeader);
        g_fileWrite(interfaceHeader, " __declspec(dllexport)\n", upperFullName);
        g_fileWrite(interfaceHeader, "#elif BUILDING_%s\n", upperFullName);
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
corto_int16 corto_genMain(g_generator g) {
    corto_bool cppbinding = !strcmp(g_getAttribute(g, "lang"), "cpp");

    /* Create source and include directories */
    corto_mkdir("include");
    corto_mkdir("src");
    corto_mkdir(".corto");

    if (c_projectGenerateMainFile(g)) {
        if (!corto_lasterr()) {
            corto_seterr("failed to create main sourcefile");
        }
        goto error;
    }

    if (g->objects) {
        if (c_genInterfaceHeader(g)) {
            if (!corto_lasterr()) {
                corto_seterr("failed to create interface header");
            }
            goto error;
        }
        if(c_projectGenerateDepMakefile(g)) {
            if (!corto_lasterr()) {
                corto_seterr("failed to create dependency rakefile");
            }
            goto error;
        }
    }

    /* The mainheader is generated by the project generator when there are no
     * type definitions for this project. */
    if (!g_getCurrent(g) || cppbinding) {
        if(c_projectGenerateMainHeaderFile(g)) {
            if (!corto_lasterr()) {
                corto_seterr("failed to create main header");
            }
            goto error;
        }
    }

    return 0;
error:
    return -1;
}
