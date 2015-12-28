
#include "corto/corto.h"
#include "c_common.h"

/* Generate file containing component loader */
static corto_int16 c_projectGenerateMainFile(corto_generator g) {
    corto_id filename;
    g_file file;
    corto_bool isComponent = !strcmp(gen_getAttribute(g, "component"), "true");

    sprintf(filename, "_load.c");

    file = g_hiddenFileOpen(g, filename);
    if(!file) {
        goto error;
    }

    g_fileWrite(file, "/* %s\n", filename);
    g_fileWrite(file, " *\n");
    g_fileWrite(file, " * This file is generated. Do not modify.\n");
    g_fileWrite(file, " */\n\n");

    if (g_getCurrent(g)) {
        c_include(file, g_getCurrent(g));
        c_writeExport(g, file);
        g_fileWrite(file, "int cortomain(int argc, char* argv[]) {\n");
        g_fileIndent(file);
        g_fileWrite(file, "int %s_load(void);\n", g_getName(g));
        g_fileWrite(file, "if (%s_load()) return -1;\n", g_getName(g));
        g_fileWrite(file, "int %sMain(int argc, char* argv[]);\n", g_getName(g));
        g_fileWrite(file, "if (%sMain(argc, argv)) return -1;\n", g_getName(g));
        g_fileWrite(file, "return 0;\n");
        g_fileDedent(file);
        g_fileWrite(file, "}\n\n");
    } else {
        c_includeFrom(file, corto_o, "corto.h");
        if (isComponent) {
            g_fileWrite(file, "int cortomain(int argc, char* argv[]) {\n");
        } else {
            g_fileWrite(file, "int main(int argc, char* argv[]) {\n");
            g_fileWrite(file, "corto_start();\n");
        }
        g_fileIndent(file);
        g_fileWrite(file, "int %sMain(int argc, char* argv[]);\n", g_getName(g));
        g_fileWrite(file, "if (%sMain(argc, argv)) return -1;\n", g_getName(g));
        if (!isComponent) {
            g_fileWrite(file, "corto_stop();\n");
        }
        g_fileWrite(file, "return 0;\n");
        g_fileDedent(file);
        g_fileWrite(file, "}\n\n");
    }

    return 0;
error:
    return -1;
}

/* Generate main header containing includes to dependencies */
static corto_int16 c_projectGenerateMainHeaderFile(corto_generator g) {
    corto_id filename;
    g_file file;
    corto_ll packages, components;
    corto_id upperName;
    strcpy(upperName, g_getName(g));
    corto_strupper(upperName);

    sprintf(filename, "%s.h", g_getName(g));

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

    c_includeFrom(file, corto_o, "corto.h");

    if ((packages = corto_loadGetPackages())) {
        corto_iter iter = corto_llIter(packages);
        while (corto_iterHasNext(&iter)) {
            corto_string str = corto_iterNext(&iter);
            corto_object package = corto_resolve(NULL, str);
            if (!package) {
                corto_error("corto: package.txt contains unresolved package '%s'", str);
                goto error;
            }
            c_include(file, package);
            corto_release(package);
        }
        corto_loadFreePackages(packages);
    }

    if ((components = corto_loadGetComponents())) {
        corto_iter iter = corto_llIter(components);
        while (corto_iterHasNext(&iter)) {
            corto_string str = corto_iterNext(&iter);
            g_fileWrite(file, "#include \"%s.h\"\n", str);
        }
        corto_loadFreeComponents(components);
    }

    g_fileWrite(file, "#ifdef __cplusplus\n");
    g_fileWrite(file, "extern \"C\" {\n");
    g_fileWrite(file, "#endif\n");

    g_fileWrite(file, "\n");
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
    g_fileWrite(file, "}\n");
    g_fileWrite(file, "#endif\n");
    g_fileWrite(file, "#endif\n\n");

    g_fileClose(file);

    return 0;
error:
    return -1;
}

typedef struct c_projectCleanInclude_t {
    corto_generator g;
    g_file file;
} c_projectCleanInclude_t;

static int c_projectCleanInclude(corto_object o, void *userData) {
    c_projectCleanInclude_t *data = userData;

    if (corto_instanceof(corto_type(corto_interface_o), o) || corto_instanceof(corto_type(corto_package_o), o)) {
        corto_id id;
        g_fileWrite(
          data->file, "CLOBBER.include(\"include/%s\")\n",
          c_filename(id, o, "h"));
    }

    return 1;
}

/* Generate dependency makefile for project */
static corto_int16 c_projectGenerateDepMakefile(corto_generator g) {
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

/* Generate dependency makefile for project */
static corto_int16 c_projectGeneratePackageFile(corto_generator g) {
    corto_file file = NULL;

    g_resolveImports(g);
    if (g->imports) {
        file = corto_fileAppend(".corto/packages.txt");
        corto_iter iter = corto_llIter(g->imports);

        while (corto_iterHasNext(&iter)) {
            corto_object import = corto_iterNext(&iter);
            corto_id id;
            corto_fullpath(id, import);
            if (!corto_loadRequiresPackage(id)) {
                fprintf(corto_fileGet(file), "%s\n", id);
            }
        }

        corto_fileClose(file);
    }

    return 0;
}

/* Generate interface header with macro's for exporting */
static corto_int16 c_genInterfaceHeader(corto_generator g) {
    corto_id interfaceHeaderName;
    sprintf(interfaceHeaderName, "_interface.h");
    g_file interfaceHeader = g_fileOpen(g, interfaceHeaderName);

    if (!interfaceHeader) {
        goto error;
    } else {
        corto_id upperName;
        strcpy(upperName, g_getName(g));
        corto_strupper(upperName);

        g_fileWrite(interfaceHeader, "/* %s\n", interfaceHeaderName);
        g_fileWrite(interfaceHeader, " *\n");
        g_fileWrite(interfaceHeader, " * This file contains generated code. Do not modify!\n");
        g_fileWrite(interfaceHeader, " */\n\n");

        g_fileWrite(interfaceHeader, "#if BUILDING_%s && defined _MSC_VER\n", upperName);
        g_fileWrite(interfaceHeader, "#define %s_DLL_EXPORTED __declspec(dllexport)\n", upperName);
        g_fileWrite(interfaceHeader, "#elif BUILDING_%s\n", upperName);
        g_fileWrite(interfaceHeader, "#define %s_EXPORT __attribute__((__visibility__(\"default\")))\n", upperName);
        g_fileWrite(interfaceHeader, "#elif defined _MSC_VER\n");
        g_fileWrite(interfaceHeader, "#define %s_EXPORT __declspec(dllimport)\n", upperName);
        g_fileWrite(interfaceHeader, "#else\n");
        g_fileWrite(interfaceHeader, "#define %s_EXPORT\n", upperName);
        g_fileWrite(interfaceHeader, "#endif\n\n");
    }

    return 0;
error:
    return -1;
}

/* Generator main */
corto_int16 corto_genMain(corto_generator g) {

    /* Create source and include directories */
    corto_mkdir("include");
    corto_mkdir("src");
    corto_mkdir(".corto");

    if(c_projectGenerateMainFile(g)) {
        goto error;
    }

    if (c_genInterfaceHeader(g)) {
        goto error;
    }

    if (g->objects) {
        if(c_projectGenerateDepMakefile(g)) {
            goto error;
        }
        if(c_projectGeneratePackageFile(g)) {
            goto error;
        }
    } else {
        if(c_projectGenerateMainHeaderFile(g)) {
            goto error;
        }
    }

    return 0;
error:
    return -1;
}
