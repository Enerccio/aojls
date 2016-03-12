#pragma once

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#ifndef AOJLS_OBJECT_START_ALLOC_SIZE
#define AOJLS_OBJECT_START_ALLOC_SIZE 16
#endif

#ifndef AOJLS_ARRAY_START_ALLOC_SIZE
#define AOJLS_ARRAY_START_ALLOC_SIZE 16
#endif

typedef enum {
	JS_OBJECT, JS_ARRAY, JS_NUMBER, JS_STRING, JS_BOOL, JS_NULL, INVALID
} json_type_t;

typedef struct _aojls_alloc_node _aojls_alloc_node_t;
typedef struct aojls_ctx aojls_ctx_t;
typedef struct json_value json_value_t;
typedef struct json_object json_object;
typedef struct json_array json_array;
typedef struct json_string json_string;
typedef struct json_number json_number;
typedef struct json_boolean json_boolean;
typedef struct json_null json_null;

/* Value */

json_type_t json_get_type(json_value_t* value);

json_object* json_as_object(json_value_t* value);
json_array* json_as_array(json_value_t* value);
double json_as_number(json_value_t* value, bool* correct_type);
char* json_as_string(json_value_t* value);
bool json_as_bool(json_value_t* value, bool* correct_type);
bool json_is_null(json_value_t* value);

/* Object */

json_object* json_make_object(aojls_ctx_t* ctx);

json_object* json_object_add(json_object* o, char* key, json_value_t* value);
json_object* json_object_nadd(json_object* o, char* key, size_t len, json_value_t* value);

size_t json_object_numkeys(json_object* o);
char* json_object_get_key(json_object* o, size_t i);

json_value_t* json_object_get_object_as_value(json_object* o, char* key);
json_object* json_object_get_object(json_object* o, char* key);
json_array* json_object_get_array(json_object* o, char* key);
double json_object_get_double(json_object* o, char* key, bool* valid);
double json_object_get_double_default(json_object* o, char* key, double defval);
char* json_object_get_string(json_object* o, char* key);
char* json_object_get_string_default(json_object* o, char* key, char* defval);
bool json_object_get_bool(json_object* o, char* key, bool* valid);
bool json_object_get_bool_default(json_object* o, char* key, bool defval);
bool json_object_is_null(json_object* o, char* key);

/* Array */

json_array* json_make_array(aojls_ctx_t* ctx);
json_array* json_array_add(json_array* a, json_value_t* value);

size_t json_array_size(json_array* a);

json_value_t* json_array_get(json_array* a, size_t i);
json_object* json_array_get_object(json_array* a, size_t i);
json_array* json_array_get_array(json_array* a, size_t i);
double json_array_get_double(json_array* a, size_t i, bool* valid);
double json_array_get_double_default(json_array* a, size_t i, double defval);
char* json_array_get_string(json_array* a, size_t i);
char* json_array_get_string_default(json_array* a, size_t i, char* defval);
bool json_array_get_bool(json_array* a, size_t i, bool* valid);
bool json_array_get_bool_default(json_array* a, size_t i, bool defval);
bool json_array_is_null(json_array* a, size_t i);

/* Primitives */

json_string* json_from_string(aojls_ctx_t* ctx, char* string);
json_number* json_from_number(aojls_ctx_t* ctx, double number);
json_boolean* json_from_boolean(aojls_ctx_t* ctx, bool b);
json_null* json_make_null(aojls_ctx_t* ctx);

/* Context */

aojls_ctx_t* json_make_context();
bool json_context_error_happened(aojls_ctx_t* ctx);
void json_free_context(aojls_ctx_t* ctx);

/* Serialization */

typedef bool(*writer_function_t)(const char* buffer, size_t len, void* writer_data);

typedef struct {
	bool pretty;
	size_t offset_per_level;
	char* eol;

	writer_function_t writer;
	void* writer_data;

	bool success;
} aojls_serialization_prefs;

char* aojls_serialize(json_value_t* value, aojls_serialization_prefs* prefs);

/* Deserialization */
