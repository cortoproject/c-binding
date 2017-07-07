/*
 * c_common.h
 *
 *  Created on: Dec 21, 2012
 *      Author: sander
 */

#ifndef C_COMMON_H_
#define C_COMMON_H_

#include "corto/corto.h"
#include "corto/g/g.h"

#ifdef __cplusplus
extern "c" {
#endif

/* Translate id to C-id */
char* corto_genId(char* str, corto_id id);

/* Return string with C type for primitive */
char* c_primitiveId(g_generator g, corto_primitive t, char* buff);

/* Parse type into C-specifier */
int16_t c_specifierId(g_generator g, corto_type t, char* specifier, bool* prefix, char* postfix);

/* Get simple type dd */
#define c_typeId(g, t, specifier) _c_typeId(g, corto_type(t), specifier)
char* _c_typeId(g_generator g, corto_type t, char *specifier);

/* Translate constant to C-language id */
char* c_constantId(g_generator g, corto_constant* c, char* buffer);

/* Translate string to c-compatible string (do escaping of ") */
char* c_escapeString(char* str);

/* Returns TRUE if procedure has this parameter */
bool c_procedureHasThis(corto_function o);

/* Translate parameter name to C (take into account special $ parameters) */
char* c_paramName(char* name, char* buffer);

/* Does type require * when passed as parameter */
bool c_paramRequiresPtr(corto_parameter *p);

/* Does type require * */
bool c_typeRequiresPtr(corto_type t);

/* Return string with typename[*] */
char* c_typeptr(g_generator g, corto_type t, corto_id id);

/* Return string with typename[*] */
char* c_typeval(g_generator g, corto_type t, corto_id id);

/* Return string with typename[*] when type is used as returntype */
typedef enum c_refKind {
    C_ByValue,
    C_ByReference
} c_refKind;
char* c_typeret(g_generator g, corto_type t, c_refKind ref, corto_id id);

/* Generate USING name */
char* c_usingName(g_generator g, corto_object o, corto_id id);

/* Generate USING macro */
char* c_usingConstant(g_generator g, corto_id id);

/* Generate BUILDING macro */
char* c_buildingMacro(g_generator g, corto_id buffer);

/* Write library export for symbol */
void c_writeExport(g_generator g, g_file file);

/* Get package for object */
corto_object c_findPackage(g_generator g, corto_object o);

/* Generic routine to create filename for object */
char* c_filename(g_generator g, char *fileName, corto_object o, char *ext);

/* Create include statement from object for specified file */
void c_includeFrom(g_generator g, g_file file, corto_object o, char* include, ...);
void c_includeFrom_toBuffer(g_generator g, corto_buffer *buffer, corto_object o, char* include, ...);

/* Create include statement for object */
void c_include(g_generator g, g_file file, corto_object o);
void c_include_toBuffer(g_generator g, corto_buffer *buffer, corto_object o);

/* Get filename of mainheader */
char* c_mainheader(g_generator g, corto_id buffer);

/* Get filename of mainheader for implementation files */
char* c_mainheaderImpl(g_generator g, corto_id buffer);

/* Find all instances of a specified class */
corto_ll c_findType(g_generator g, corto_class type);

/* Obtain ID for meta object */
char* c_varId(g_generator g, corto_object o, char* out);

char* c_functionName(g_generator g, corto_function o, corto_id id);

/* Write this parameter to file */
void c_paramThis(
    g_generator g,
    corto_buffer *buffer,
    bool cpp,
    corto_type parentType);

int c_paramWalk(corto_object f, int(*action)(corto_parameter*, void*), void *userData);

/* Create function declaration in buffer */
int16_t c_decl(g_generator g, corto_buffer *buffer, corto_function f, bool isWrapper, bool cpp, bool impl);

/* Include dependencies */
void c_includeDependencies(g_generator g, g_file file, char* header);

#ifdef __cplusplus
}
#endif

#endif /* C_COMMON_H_ */
