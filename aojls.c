#include "aojls.h"

// private struct implementations

struct _aojls_alloc_node {
	struct _aojls_alloc_node* next;
};

struct json_value{
	_aojls_alloc_node_t anode;
	json_type_t         type;
	aojls_ctx_t*		ctx;
};

struct json_object {
	json_value_t   self;
	size_t         allocated;
	size_t		   n;
	char**		   keys;
	json_value_t** values;
};

struct json_array {
	json_value_t   self;
	size_t         allocated;
	size_t		   n;
	json_value_t** elements;
};

struct json_string {
	json_value_t self;
	char*		 value; // null terminated, always
};

struct json_number {
	json_value_t self;
	double       value;
};

struct json_boolean {
	json_value_t self;
	bool		 value;
};

struct json_null {
	json_value_t self;
};

typedef struct char_node {
	struct char_node* next;
	char*  data;
} char_node_t;

struct aojls_ctx {
	_aojls_alloc_node_t* snode;
	_aojls_alloc_node_t* enode;

	json_value_t*        result;
	bool				 failed;

	char_node_t* ssnode;
	char_node_t* esnode;
};

// json value

json_type_t json_get_type(json_value_t* value) {
	if (value == NULL)
		return INVALID;
	return value->type;
}

json_object* json_as_object(json_value_t* value) {
	if (value != NULL && value->type == JS_OBJECT)
		return (json_object*)value;
	else
		return NULL;
}

json_array* json_as_array(json_value_t* value) {
	if (value != NULL && value->type == JS_ARRAY)
		return (json_array*)value;
	else
		return NULL;
}

double json_as_number(json_value_t* value, bool* correct_type) {
	if (value != NULL && value->type == JS_NUMBER) {
		if (correct_type != NULL)
			*correct_type = true;
		return ((json_number*)value)->value;
	} else
		if (correct_type != NULL)
			*correct_type = false;
	return 0;
}

char* json_as_string(json_value_t* value) {
	if (value != NULL && value->type == JS_STRING)
		return ((json_string*)value)->value;
	else
		return NULL;
}

bool json_as_bool(json_value_t* value, bool* correct_type) {
	if (value != NULL && value->type == JS_BOOL) {
		if (correct_type != NULL)
			*correct_type = true;
		return ((json_boolean*)value)->value;
	} else
		if (correct_type != NULL)
			*correct_type = false;
	return false;
}

bool json_is_null(json_value_t* value) {
	if (value != NULL && value->type == JS_NULL)
		return true;
	else
		return false;
}

// auxiliary functions

static void append_to_context(aojls_ctx_t* ctx, json_value_t* v) {
	if (v == NULL || ctx == NULL) {
		if (ctx != NULL)
			ctx->failed = true;
		return;
	}
	if (ctx->enode == NULL) {
		ctx->snode = &v->anode;
		ctx->enode = &v->anode;
	} else {
		ctx->enode->next = &v->anode;
		ctx->enode = &v->anode;
	}
	v->anode.next = NULL;
	v->ctx = ctx;
}

static char* append_string(aojls_ctx_t* ctx, char* string, size_t len) {
	if (ctx == NULL || string == NULL) {
		if (ctx != NULL)
			ctx->failed = true;
		return NULL;
	}

	char* cpy = (char*)malloc((len+1) * sizeof(char));
	if (cpy == NULL) {
		ctx->failed = true;
		return NULL;
	}
	cpy[len] = '\0';
	char_node_t* cnode = (char_node_t*)malloc(sizeof(char_node_t));
	if (cnode == NULL) {
		free(cpy);
		ctx->failed = true;
		return NULL;
	}

	memcpy(cpy, string, len);
	cnode->data = cpy;

	if (ctx->esnode == NULL) {
		ctx->esnode = cnode;
		ctx->ssnode = cnode;
	} else {
		ctx->esnode->next = cnode;
		ctx->esnode = cnode;
	}

	return cpy;
}

// json object

json_object* json_make_object(aojls_ctx_t* ctx) {
	if (ctx == NULL)
		return NULL;
	json_object* o = (json_object*)calloc(1, sizeof(json_object));
	if (o == NULL) {
		ctx->failed = true;
		return NULL;
	}

	o->self.type = JS_OBJECT;
	o->allocated = AOJLS_OBJECT_START_ALLOC_SIZE;
	o->n = 0;
	o->keys = (char**)malloc(o->allocated*sizeof(char*));
	if (o->keys == NULL) {
		free(o);
		ctx->failed = true;
		return NULL;
	}
	o->values = (json_value_t**)malloc(o->allocated*sizeof(json_value_t*));
	if (o->values == NULL) {
		free(o->keys);
		free(o);
		ctx->failed = true;
		return NULL;
	}

	append_to_context(ctx, &o->self);
	return o;
}

json_object* json_object_add(json_object* o, char* key, json_value_t* value) {
	if (key == NULL)
		return NULL;
	return json_object_nadd(o, key, strlen(key), value);
}

json_object* json_object_nadd(json_object* o, char* key, size_t len, json_value_t* value) {
	if (o == NULL || value == NULL || key == NULL) {
		if (o != NULL)
			o->self.ctx->failed = true;
		return NULL;
	}

	if (o->n == o->allocated) {
		// reallocate and increase the size
		size_t allocated = o->allocated * 2;
		char** keys = (char**)realloc(o->keys, allocated*sizeof(char*));
		if (keys == NULL) {
			o->self.ctx->failed = true;
			return NULL;
		}
		json_value_t** values = (json_value_t**)realloc(o->values, allocated*sizeof(json_value_t*));
		if (values == NULL) {
			free(keys);
			o->self.ctx->failed = true;
			return NULL;
		}
		o->allocated = allocated;
		o->keys = keys;
		o->values = values;
	}

	key = append_string(o->self.ctx, key, len);
	if (key == NULL) {
		o->self.ctx->failed = true;
		return NULL;
	}

	o->keys[o->n] = key;
	o->values[o->n] = value;
	++o->n;

	return o;
}

size_t json_object_numkeys(json_object* o) {
	if (o == NULL)
		return 0;
	return o->n;
}

char* json_object_get_key(json_object* o, size_t i) {
	if (o == NULL || i >= o->n)
		return NULL;
	return o->keys[i];
}

json_value_t* json_object_get_object_as_value(json_object* o, char* key) {
	if (o == NULL || key == NULL) {
		if (o != NULL)
			o->self.ctx->failed = true;
		return NULL;
	}
	for (size_t i=0; i<o->n; i++) {
		if (strcmp(key, o->keys[i]) == 0) {
			return o->values[i];
		}
	}
	return NULL; // not found
}

json_object* json_object_get_object(json_object* o, char* key) {
	json_value_t* value = json_object_get_object_as_value(o, key);
	if (value == NULL) {
		return NULL;
	}
	return json_as_object(value);
}

json_array* json_object_get_array(json_object* o, char* key) {
	json_value_t* value = json_object_get_object_as_value(o, key);
	if (value == NULL) {
		return NULL;
	}
	return json_as_array(value);
}

double json_object_get_double(json_object* o, char* key, bool* valid) {
	json_value_t* value = json_object_get_object_as_value(o, key);
	if (value == NULL) {
		if (valid != NULL)
			*valid = false;
		return 0;
	}
	return json_as_number(value, valid);
}

double json_object_get_double_default(json_object* o, char* key, double defval) {
	bool valid = false;
	double result = json_object_get_double(o, key, &valid);
	if (!valid)
		result = defval;
	return result;
}

char* json_object_get_string(json_object* o, char* key) {
	json_value_t* value = json_object_get_object_as_value(o, key);
	if (value == NULL) {
		return NULL;
	}
	return json_as_string(value);
}

char* json_object_get_string_default(json_object* o, char* key, char* defval) {
	char* value = json_object_get_string(o, key);
	if (value == NULL)
		value = defval;
	return value;
}

bool json_object_get_bool(json_object* o, char* key, bool* valid) {
	json_value_t* value = json_object_get_object_as_value(o, key);
	if (value == NULL) {
		if (valid != NULL)
			*valid = false;
		return 0;
	}
	return json_as_bool(value, valid);
}

bool json_object_get_bool_default(json_object* o, char* key, bool defval) {
	bool valid = false;
	bool result = json_object_get_bool(o, key, &valid);
	if (!valid)
		result = defval;
	return result;
}

bool json_object_is_null(json_object* o, char* key) {
	json_value_t* value = json_object_get_object_as_value(o, key);
	return json_is_null(value);
}

// array

json_array* json_make_array(aojls_ctx_t* ctx) {
	if (ctx == NULL)
		return NULL;
	json_array* o = (json_array*)calloc(1, sizeof(json_array));
	if (o == NULL) {
		ctx->failed = true;
		return NULL;
	}

	o->self.type = JS_ARRAY;
	o->allocated = AOJLS_ARRAY_START_ALLOC_SIZE;
	o->n = 0;
	o->elements = (json_value_t**)malloc(o->allocated*sizeof(json_value_t*));
	if (o->elements == NULL) {
		free(o);
		ctx->failed = true;
		return NULL;
	}

	append_to_context(ctx, &o->self);
	return o;
}

json_array* json_array_add(json_array* a, json_value_t* value) {
	if (a == NULL || value == NULL) {
		if (a != NULL)
			a->self.ctx->failed = true;
		return NULL;
	}

	if (a->n == a->allocated) {
		// reallocate and increase the size
		size_t allocated = a->allocated * 2;
		json_value_t** elements = (json_value_t**)realloc(a->elements, allocated*sizeof(json_value_t*));
		if (elements == NULL) {
			a->self.ctx->failed = true;
			return NULL;
		}
		a->allocated = allocated;
		a->elements = elements;
	}

	a->elements[a->n] = value;
	++a->n;

	return a;
}

size_t json_array_size(json_array* a) {
	if (a == NULL)
		return 0;
	return a->n;
}

json_value_t* json_array_get(json_array* a, size_t i) {
	if (a == NULL)
		return NULL;
	if (i >= a->n)
		return NULL;
	return a->elements[i];
}

json_object* json_array_get_object(json_array* a, size_t key) {
	json_value_t* value = json_array_get(a, key);
	if (value == NULL) {
		return NULL;
	}
	return json_as_object(value);
}

json_array* json_array_get_array(json_array* a, size_t key) {
	json_value_t* value = json_array_get(a, key);
	if (value == NULL) {
		return NULL;
	}
	return json_as_array(value);
}

double json_array_get_double(json_array* a, size_t key, bool* valid) {
	json_value_t* value = json_array_get(a, key);
	if (value == NULL) {
		if (valid != NULL)
			*valid = false;
		return 0;
	}
	return json_as_number(value, valid);
}

double json_array_get_double_default(json_array* a, size_t key, double defval) {
	bool valid = false;
	double result = json_array_get_double(a, key, &valid);
	if (!valid)
		result = defval;
	return result;
}

char* json_array_get_string(json_array* a, size_t key) {
	json_value_t* value = json_array_get(a, key);
	if (value == NULL) {
		return NULL;
	}
	return json_as_string(value);
}

char* json_array_get_string_default(json_array* a, size_t key, char* defval) {
	char* value = json_array_get_string(a, key);
	if (value == NULL)
		value = defval;
	return value;
}

bool json_array_get_bool(json_array* a, size_t key, bool* valid) {
	json_value_t* value = json_array_get(a, key);
	if (value == NULL) {
		if (valid != NULL)
			*valid = false;
		return 0;
	}
	return json_as_bool(value, valid);
}

bool json_array_get_bool_default(json_array* a, size_t key, bool defval) {
	bool valid = false;
	bool result = json_array_get_bool(a, key, &valid);
	if (!valid)
		result = defval;
	return result;
}

bool json_array_is_null(json_array* a, size_t key) {
	json_value_t* value = json_array_get(a, key);
	return json_is_null(value);
}

// primitives

json_string* json_from_string(aojls_ctx_t* ctx, char* string) {
	if (ctx == NULL || string == NULL) {
		if (ctx != NULL)
			ctx->failed = true;
		return NULL;
	}
	json_string* o = (json_string*)calloc(1, sizeof(json_string));
	if (o == NULL) {
		ctx->failed = true;
		return NULL;
	}
	string = append_string(ctx, string, strlen(string));
	if (string == NULL) {
		free(o);
		ctx->failed = true;
		return NULL;
	}

	o->self.type = JS_STRING;
	o->value = string;
	append_to_context(ctx, &o->self);
	return o;
}

json_number* json_from_number(aojls_ctx_t* ctx, double number) {
	if (ctx == NULL) {
		return NULL;
	}
	json_number* o = (json_number*)calloc(1, sizeof(json_number));
	if (o == NULL) {
		ctx->failed = true;
		return NULL;
	}
	o->self.type = JS_NUMBER;
	o->value = number;

	append_to_context(ctx, &o->self);
	return o;
}

json_boolean* json_from_boolean(aojls_ctx_t* ctx, bool b) {
	if (ctx == NULL)
		return NULL;
	json_boolean* o = (json_boolean*)calloc(1, sizeof(json_boolean));
	if (o == NULL) {
		ctx->failed = true;
		return NULL;
	}
	o->self.type = JS_BOOL;
	o->value = b;

	append_to_context(ctx, &o->self);
	return o;
}

json_null* json_make_null(aojls_ctx_t* ctx) {
	if (ctx == NULL)
		return NULL;
	json_null* o = (json_null*)calloc(1, sizeof(json_null));
	if (o == NULL) {
		ctx->failed = true;
		return NULL;
	}
	o->self.type = JS_NULL;

	append_to_context(ctx, &o->self);
	return o;
}

// context

aojls_ctx_t* json_make_context() {
	return (aojls_ctx_t*) calloc(1, sizeof(aojls_ctx_t));
}

bool json_context_error_happened(aojls_ctx_t* ctx) {
	return ctx->failed;
}

void json_free_context(aojls_ctx_t* ctx) {
	if (ctx == NULL)
		return;

	_aojls_alloc_node_t* anode = ctx->snode;
	while (anode != NULL) {
		_aojls_alloc_node_t* node = anode;
		anode = node->next;
		free(node);
	}

	char_node_t* cnode = ctx->ssnode;
	while (cnode != NULL) {
		char_node_t* node = cnode;
		cnode = node->next;
		free(node->data);
		free(node);
	}

	free(ctx);
}

// serialization

typedef struct {
	char* data;
	size_t offset;
	size_t len;
} string_writer_data_t;

static bool string_writer_function(const char* buffer, size_t len, void* writer_data) {
	string_writer_data_t* wd = (string_writer_data_t*)writer_data;

	if (wd->offset + len >= wd->len) {
		char* rb = (char*)realloc(wd->data, wd->len*2);
		if (rb == NULL) {
			return false;
		}
		wd->data = rb;
		wd->len *= 2;
		return string_writer_function(buffer, len, writer_data);
	}

	memcpy(wd->data+wd->offset, buffer, len);
	wd->offset += len;

	return true;
}

static bool do_serialize_string(char* string, aojls_serialization_prefs* prefs) {
	size_t len = strlen(string);
	if (!prefs->writer("\"", 1, prefs->writer_data))
		return false;
	for (size_t i=0; i<len; i++) {
		if (string[i] == '\n') {
			if (!prefs->writer("\\n", 2, prefs->writer_data))
				return false;
		} else if (string[i] == '\r') {
			if (!prefs->writer("\\r", 2, prefs->writer_data))
				return false;
		} else if (string[i] == '\t') {
			if (!prefs->writer("\\t", 2, prefs->writer_data))
				return false;
		} else if (string[i] == '\f') {
			if (!prefs->writer("\\f", 2, prefs->writer_data))
				return false;
		} else if (string[i] == '\b') {
			if (!prefs->writer("\\b", 2, prefs->writer_data))
				return false;
		} else if (string[i] == '/') {
			if (!prefs->writer("\\/", 2, prefs->writer_data))
				return false;
		} else if (string[i] == '\\') {
			if (!prefs->writer("\\\\", 2, prefs->writer_data))
				return false;
		} else if (string[i] == '\"') {
				if (!prefs->writer("\\\"", 2, prefs->writer_data))
					return false;
		} else {
			if (!prefs->writer(string+i, 1, prefs->writer_data))
				return false;
		}
	}
	if (!prefs->writer("\"", 1, prefs->writer_data))
		return false;
	return true;
}

static bool do_serialize(json_value_t* value, aojls_serialization_prefs* prefs,
		const char* perlinsert, const char* eol, size_t level) {
	if (value == NULL)
		return false;

	switch (json_get_type(value)) {
	case JS_OBJECT: {
		if (!prefs->writer("{", 1, prefs->writer_data))
			return false;
		if (!prefs->writer(eol, strlen(eol), prefs->writer_data))
			return false;

		size_t nl = level + 1;
		json_object* o = json_as_object(value);
		size_t keys = json_object_numkeys(o);

		for (size_t k=0; k<keys; k++) {
			for (size_t i=0; i<nl; i++) {
				if (!prefs->writer(perlinsert, strlen(perlinsert), prefs->writer_data))
					return false;
			}

			char* key = json_object_get_key(o, k);
			if (!do_serialize_string(key, prefs))
				return false;
			if (prefs->pretty) {
				if (!prefs->writer(" : ", 3, prefs->writer_data))
					return false;
			} else {
				if (!prefs->writer(":", 1, prefs->writer_data))
					return false;
			}

			json_value_t* value = json_object_get_object_as_value(o, key);
			if (!do_serialize(value, prefs, perlinsert, eol, nl))
				return false;

			if (k != keys-1) {
				if (prefs->pretty) {
					if (!prefs->writer(", ", 2, prefs->writer_data))
						return false;
				} else {
					if (!prefs->writer(",", 1, prefs->writer_data))
						return false;
				}
			}

			if (!prefs->writer(eol, strlen(eol), prefs->writer_data))
				return false;
		}

		for (size_t i=0; i<level; i++) {
			if (!prefs->writer(perlinsert, strlen(perlinsert), prefs->writer_data))
				return false;
		}
		if (!prefs->writer("}", 1, prefs->writer_data))
			return false;
		return true;
	}
	case JS_ARRAY: {
		if (!prefs->writer("[", 1, prefs->writer_data))
					return false;
		if (!prefs->writer(eol, strlen(eol), prefs->writer_data))
			return false;

		size_t nl = level + 1;
		json_array* a = json_as_array(value);
		size_t keys = json_array_size(a);

		for (size_t k=0; k<keys; k++) {
			for (size_t i=0; i<nl; i++) {
				if (!prefs->writer(perlinsert, strlen(perlinsert), prefs->writer_data))
					return false;
			}

			json_value_t* value = json_array_get(a, k);
			if (!do_serialize(value, prefs, perlinsert, eol, nl))
				return false;

			if (k != keys-1) {
				if (prefs->pretty) {
					if (!prefs->writer(", ", 2, prefs->writer_data))
						return false;
				} else {
					if (!prefs->writer(",", 1, prefs->writer_data))
						return false;
				}
			}

			if (!prefs->writer(eol, strlen(eol), prefs->writer_data))
				return false;
		}

		for (size_t i=0; i<level; i++) {
			if (!prefs->writer(perlinsert, strlen(perlinsert), prefs->writer_data))
				return false;
		}
		if (!prefs->writer("]", 1, prefs->writer_data))
			return false;
		return true;
	}
	case JS_NUMBER: {
		double num = json_as_number(value, NULL);
		char buf[2048];
		sprintf(buf, "%f", num);
		return prefs->writer(buf, strlen(buf), prefs->writer_data);
	}
	case JS_STRING: {
		char* string = json_as_string(value);
		return do_serialize_string(string, prefs);
	}
	case JS_BOOL: {
		bool v = json_as_bool(value, NULL);
		if (v)
			return prefs->writer("true", 4, prefs->writer_data);
		else
			return prefs->writer("false", 5, prefs->writer_data);
	}
	case JS_NULL: {
		return prefs->writer("null", 4, prefs->writer_data);
	}
	case INVALID:
	default:
		return false;
	}
}

static bool serialize(json_value_t* value, aojls_serialization_prefs* prefs) {
	char* eol = "";
	char* perlinsert = "";
	if (prefs->pretty) {
		if (prefs->eol == NULL)
			eol = "\n";
		else
			eol = prefs->eol;
		perlinsert = (char*)malloc(prefs->offset_per_level * sizeof(char));
		if (perlinsert == NULL) {
			return false;
		}
		for (size_t i=0; i<prefs->offset_per_level; i++)
			perlinsert[i] = ' ';
	}
	bool r = true;

	r = do_serialize(value, prefs, perlinsert, eol, 0);

	if (prefs->pretty) {
		free(perlinsert);
	}
	return r;
}

char* aojls_serialize(json_value_t* value, aojls_serialization_prefs* prefs) {
	aojls_serialization_prefs p;
	if (prefs == NULL) {
		p.pretty = false;
		p.writer_data = NULL;
		p.writer = NULL;
	} else {
		p = *prefs;
	}

	bool selfbuffer = false;
	if (p.writer == NULL) {
		selfbuffer = true;
		p.writer = string_writer_function;
		string_writer_data_t* wd = (string_writer_data_t*)malloc(sizeof(string_writer_data_t));
		if (wd == NULL) {
			p.success = false;
			return NULL;
		}

		wd->data = (char*)malloc(2048);
		if (wd->data == NULL) {
			free(wd);
			p.success = false;
			return NULL;
		}
		wd->len = 2048;
		wd->offset = 0;
		p.writer_data = wd;
	}

	bool result = serialize(value, &p);

	if ((!result && selfbuffer) || (!p.writer("\0", 1, p.writer_data) && selfbuffer)) {
		free(((string_writer_data_t*)p.writer_data)->data);
		free(p.writer_data);
		return NULL;
	}

	if (selfbuffer) {
		char* buffer = ((string_writer_data_t*)p.writer_data)->data;
		free(p.writer_data);
		return buffer;
	}

	return NULL;
}
