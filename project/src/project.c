
#include "corto/corto.h"
#include "corto/gen/c/common/common.h"
#include <corto/fmt/json/json.h>

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
static corto_int16 c_projectGenerateMainFile(corto_generator g) {
    corto_id filename;
    g_file file;
    corto_bool app = !strcmp(gen_getAttribute(g, "app"), "true");
    corto_bool cpp = !strcmp(gen_getAttribute(g, "c4cpp"), "true");

    sprintf(filename, "_load.%s", cpp ? "cpp" : "c");

    file = g_hiddenFileOpen(g, filename);
    if(!file) {
        goto error;
    }

    g_fileWrite(file, "/* %s\n", filename);
    g_fileWrite(file, " *\n");
    g_fileWrite(file, " * This file is generated. Do not modify.\n");
    g_fileWrite(file, " */\n\n");

    corto_id header;
    g_fileWrite(file, "#include <%s>\n", c_mainheader(g, header));
    g_fileWrite(file, "\n");
    if (g_getCurrent(g)) {
        g_fileWrite(file, "int %s_load(void);\n", g_getProjectName(g));
    }
    g_fileWrite(file, "int %sMain(int argc, char* argv[]);\n", g_getProjectName(g));
    g_fileWrite(file, "\n");
    if (!app) {
        g_fileWrite(file, "#ifdef __cplusplus\n");
        g_fileWrite(file, "extern \"C\"\n");
        g_fileWrite(file, "#endif\n");
        if (g_getCurrent(g)) {
          c_writeExport(g, file);
          g_fileWrite(file, " ");
        }
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
static corto_int16 c_projectGenerateMainHeaderFile(corto_generator g) {
    g_file file;
    corto_ll packages;
    corto_id upperName;
    corto_bool error = FALSE;
    corto_id filename;
    sprintf(filename, "%s.h", g_getProjectName(g));
    corto_bool app = !strcmp(gen_getAttribute(g, "app"), "true");
    corto_bool local = !strcmp(gen_getAttribute(g, "local"), "true");

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
        g_fileWrite(file, "#include <_interface.h>\n");
    }
    g_fileWrite(file, "\n");

    if ((packages = corto_loadGetPackages())) {
        corto_iter iter = corto_llIter(packages);
        while (corto_iterHasNext(&iter)) {
            corto_string str = corto_iterNext(&iter);
            corto_string package = corto_locate(str, CORTO_LOCATION_FULLNAME);
            if (!package) {
                corto_seterr("packages.txt contains unresolved package '%s'", str);

                /* Don't break out of generation here, as this will mess up the
                 * file's code snippet */
                error = TRUE;
            } else {
                corto_string name = corto_locate(str, CORTO_LOCATION_NAME);
                g_fileWrite(file, "#include <%s/%s.h>\n", package, name);
                corto_dealloc(name);
                corto_dealloc(package);
            }
        }
        corto_loadFreePackages(packages);
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
    corto_generator g;
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

    if (g_getCurrent(g)) {
        sprintf(interfaceHeaderName, "_interface.h");
    } else {
        sprintf(interfaceHeaderName, "%s/_interface.h", g_getName(g));
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
        }
        corto_strupper(upperFullName);

        g_fileWrite(interfaceHeader, "/* %s\n", interfaceHeaderName);
        g_fileWrite(interfaceHeader, " *\n");
        g_fileWrite(interfaceHeader, " * This file contains generated code. Do not modify!\n");
        g_fileWrite(interfaceHeader, " */\n\n");

        g_fileWrite(interfaceHeader, "#if BUILDING_%s && defined _MSC_VER\n", upperFullName);
        g_fileWrite(interfaceHeader, "#define %s_DLL_EXPORTED __declspec(dllexport)\n", upperName);
        g_fileWrite(interfaceHeader, "#elif BUILDING_%s\n", upperFullName);
        g_fileWrite(interfaceHeader, "#define ");
        c_writeExport(g, interfaceHeader);
        g_fileWrite(interfaceHeader, " __attribute__((__visibility__(\"default\")))\n", upperName);
        g_fileWrite(interfaceHeader, "#elif defined _MSC_VER\n");
        g_fileWrite(interfaceHeader, "#define ");
        c_writeExport(g, interfaceHeader);
        g_fileWrite(interfaceHeader, " __declspec(dllimport)\n", upperName);
        g_fileWrite(interfaceHeader, "#else\n");
        g_fileWrite(interfaceHeader, "#define ");
        c_writeExport(g, interfaceHeader);
        g_fileWrite(interfaceHeader, "\n", upperName);
        g_fileWrite(interfaceHeader, "#endif\n\n");
    }

    return 0;
error:
    return -1;
}


static corto_int16 corto_printCortoListAsRubyArray(g_file f, const char* rubyName, corto_ll list)
{
    g_fileWrite(f, "%s = [\n", rubyName);
    g_fileIndent(f);
    {
        corto_stringlistForeach(list, elem) {
            g_fileWrite(f, "\"%s\",\n", elem);
        }
    }
    g_fileDedent(f);
    g_fileWrite(f, "]\n");
    return 0;
}

static corto_int16 corto_writeRakefile(corto_generator g, corto_package package)
{
    g_file rakefile = g_fileOpen(g, "rakefile");
    g_fileWrite(rakefile, "PACKAGE = '%s'\n", corto_fullpath(NULL, package));

    if (corto_llSize(package->lib)) {
        corto_printCortoListAsRubyArray(rakefile, "LIB", package->lib);
    }
    if (corto_llSize(package->libpath)) {
        corto_printCortoListAsRubyArray(rakefile, "LIBPATH", package->libpath);
    }
    if (corto_llSize(package->include)) {
        corto_printCortoListAsRubyArray(rakefile, "INCLUDE", package->include);
    }
    if (corto_llSize(package->link)) {
        corto_printCortoListAsRubyArray(rakefile, "LINK", package->link);
    }
    if (corto_llSize(package->dependencies)) {
        corto_printCortoListAsRubyArray(rakefile, "USE_PACKAGE", package->dependencies);
    }
    if (corto_llSize(package->cflags)) {
        corto_printCortoListAsRubyArray(rakefile, "CFLAGS", package->cflags);
    }

    g_fileWrite(rakefile, "require \"#{ENV['CORTO_BUILD']}/package\"\n");
    g_fileClose(rakefile);
    return 0;
}

/*
 * This method changes the id!
 */
static void corto_readPackageJson_separateParentAndName(char* id, char** parentName, char** name)
{
    char *ptr = strrchr(id, '/');
    if (ptr) {
        *name = ptr + 1;
        *parentName = id;
        *ptr = '\0';
    } else {
        *name = id;
        *parentName = NULL;
    }
}

static corto_object corto_readPackageJson_declarePackage(const char* parentName, const char* name)
{
    corto_object package = NULL;
    if (parentName) {
        corto_object parent = corto_resolve(root_o, (corto_string)parentName);
        if (!parent) {
            corto_seterr("Cannot find parent package %s", parentName);
            goto errorNoParent;
        }
        package = corto_packageDeclareChild(parent, (corto_string)name);
        corto_release(parent);
    } else {
        package = corto_packageDeclareChild(root_o, (corto_string)name);
    }
    return package;

errorNoParent:
    return NULL;
}

static corto_int16 corto_readPackageJson(corto_generator g)
{
    if (!corto_fileTest("package.json")) {
        goto warning;
    }

    char* fileContent = corto_fileLoad("package.json");
    if (!fileContent) {
        goto errorFileLoad;
    }

    JSON_Value* value = json_parse_string(fileContent);
    if (!value) {
        corto_seterr("Error parsing package.json");
        goto errorParsePackageJson;
    }

    JSON_Object* packageAsObject = json_value_get_object(value);
    if (!packageAsObject) {
        corto_seterr("Main object in package.json is not JSON object");
        goto errorPackageNotObject;
    }
    const char* id = json_object_get_string(packageAsObject, "id");
    if (!id) {
        corto_seterr("No \"id\" string field found");
        goto errorNoId;
    }

    char* idbuf = corto_strdup(id);
    if (!idbuf) {
        goto errorIdBuf;
    }
    char *name = NULL;
    char *parentName = NULL;
    corto_readPackageJson_separateParentAndName(idbuf, &parentName, &name);

    corto_package package = corto_readPackageJson_declarePackage(parentName, name);
    if (!package) {
        goto errorNoPackage;
    }

    JSON_Object* valueObject = json_object_get_object(packageAsObject, "value");
    if (!valueObject) {
        corto_seterr("No \"value\" object field found");
        goto errorGetValue;
    }

    char* valueStr = json_serialize_to_string(json_object_get_value(packageAsObject, "value"));
    if (!valueStr) {
        corto_seterr("Error serializing value to string");
        goto errorSerializeToString;
    }

    if (json_toCorto(package, valueStr)) {
        goto errorToCorto;
    }

    if (corto_writeRakefile(g, package)) {
        goto errorWriteRakefile;
    }

    corto_delete(package);
    corto_dealloc(valueStr);
    corto_dealloc(idbuf);
    corto_dealloc(value);
    corto_dealloc(fileContent);

    return 0;

errorWriteRakefile:
errorToCorto:
    corto_delete(package);

    corto_dealloc(valueStr);
errorSerializeToString:
errorGetValue:
errorNoPackage:
    corto_dealloc(idbuf);
errorIdBuf:
errorNoId:
errorPackageNotObject:
    json_value_free(value);
errorParsePackageJson:
    corto_dealloc(fileContent);
errorFileLoad:
    return -1;

warning:
    corto_warning("Warning: package.json is not found; consider upgrading to the new build system.");
    return 0;
}

static corto_int16 corto_genRakefile(corto_generator g)
{
    if (corto_readPackageJson(g)) {
        goto error;
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

    if (corto_genRakefile(g)) {
        goto error;
    }

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
        if(c_projectGeneratePackageFile(g)) {
            if (!corto_lasterr()) {
                corto_seterr("failed to create packages.txt");
            }
            goto error;
        }
    } else {
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
