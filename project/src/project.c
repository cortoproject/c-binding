
#include "corto/corto.h"
#include "corto/gen/c/common/common.h"

/* Load dependencies */
static void c_projectLoadPackages(g_generator g, g_file file) {

    if (g->imports) {
        corto_iter iter = corto_ll_iter(g->imports);
        while (corto_iter_hasNext(&iter)) {
            corto_object o = corto_iter_next(&iter);
            g_fileWrite(
                file, 
                "if (corto_load(\"%s\", 0, NULL)) return -1;\n", 
                corto_path(NULL, NULL, o, "/"));
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
    if (app) g_fileWrite(file, "corto_start(argv[0]);\n");
    c_projectLoadPackages(g, file);
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

/* Generate interface header with macro's for exporting */
static corto_int16 c_genInterfaceHeader(g_generator g) {
    corto_id interfaceHeaderName;

    sprintf(interfaceHeaderName, "_project.h");

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

    if (c_genInterfaceHeader(g)) {
        if (!corto_lasterr()) {
            corto_seterr("failed to create interface header");
        }
        goto error;
    }

    return 0;
error:
    return -1;
}
