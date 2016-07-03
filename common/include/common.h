/*
 * c_common.h
 *
 *  Created on: Dec 21, 2012
 *      Author: sander
 */

#ifndef C_COMMON_H_
#define C_COMMON_H_

#include "corto/corto.h"

#ifdef __cplusplus
extern "c" {
#endif

/* Translate id to C-id */
corto_string corto_genId(corto_string str, corto_id id);

/* Return string with C type for primitive */
corto_char* c_primitiveId(corto_primitive t, corto_char* buff);

/* Parse type into C-specifier */
corto_int16 c_specifierId(corto_generator g, corto_type t, corto_char* specifier, corto_bool* prefix, corto_char* postfix);

/* Translate constant to C-language id */
corto_char* c_constantId(corto_generator g, corto_constant* c, corto_char* buffer);

/* Translate string to c-compatible string (do escaping of ") */
corto_char* c_escapeString(corto_string str, corto_id id);

/* Returns TRUE if procedure has this parameter */
corto_bool c_procedureHasThis(corto_function o);

/* Translate parameter name to C (take into account special $ parameters) */
corto_string c_paramName(corto_string name, corto_string buffer);

/* Does type require * when passed as parameter */
corto_bool c_paramRequiresPtr(corto_parameter *p);

/* Does type require * */
corto_bool c_typeRequiresPtr(corto_type t);

/* Return string with typename[*] */
corto_string c_typeptr(corto_generator g, corto_type t, corto_id id);

/* Return string with typename[*] */
corto_string c_typeval(corto_generator g, corto_type t, corto_id id);

/* Return string with typename[*] when type is used as returntype */
corto_string c_typeret(corto_generator g, corto_type t, corto_id id);

/* Generate USING name */
corto_char* c_usingName(corto_generator g, corto_object o, corto_id id);

/* Generate USING macro */
corto_char* c_usingConstant(corto_generator g, corto_id id);

/* Write library export for symbol */
void c_writeExport(corto_generator g, g_file file);

/* Generic routine to create filename for object */
char* c_filename(corto_generator g, char *fileName, corto_object o, char *ext);

/* Create include statement from object for specified file */
void c_includeFrom(corto_generator g, g_file file, corto_object o, corto_string include, ...);

/* Create include statement for object */
void c_include(corto_generator g, g_file file, corto_object o);

/* Get filename of mainheader */
char* c_mainheader(corto_generator g, corto_id buffer);

/* Find all instances of a specified class */
corto_ll c_findType(corto_generator g, corto_class type);

/* Obtain ID for meta object */
corto_char* c_varId(corto_generator g, corto_object o, corto_char* out);

char* c_functionName(corto_generator g, corto_function o, corto_id id);

/* Write this parameter to file */
void c_paramThis(
    corto_generator g,
    g_file file,
    corto_bool cpp,
    corto_type parentType);

int c_paramWalk(corto_object f, int(*action)(corto_parameter*, void*), void *userData);

/* Print function declaration to file */
corto_int16 c_decl(corto_generator g, g_file file, corto_function f, corto_bool isWrapper, corto_bool cpp);

#ifdef __cplusplus
}
#endif

#endif /* C_COMMON_H_ */
