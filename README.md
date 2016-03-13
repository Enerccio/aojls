# AOJLS

AOJLS - "All other JSON libraries suck" is JSON parsing/generating library that is not aiming for speed or efficiency, but instead aims for programmers comfort. With this in mind, AOJLS is built on these principles:

1. Creating simple and complex JSON values should be easy and intuitive
2. Programmer should not be bothered to track liveness of all JSON values manually
3. If there is any error when working with JSON values, it should be possible to defer error checking until next logical step

To fulfill these principles, AOJLS employs these techniques:

* easy to use api for value creation and usage
* all values require context and their liveness is bound by it
* in case of failure, context is marked as such and all operations on the JSON data continue to work in a fashion that they will not cause undefined behavior (however returned values might be sentinel values)

## License

AOJLS is released under BSD 3.0 license. You can freerly use or modify this code, as long as you provide attribution. Author is, however, not liable for any damages for using this code.

## Example usage

### Value creation

For all operations except deserialization (and even there, if we want), you need first to create instance of `aojls_ctx_t`:

```c
	aojls_ctx_t* ctx = json_make_context(); // creates new context, or returns NULL on failure
```

After creating context, we can create other JSON values:

```c
	json_object* root = json_make_object(ctx);             // creates empty JSON object {}
	json_array* array = json_make_array(ctx);              // creates empty JSON array []
	json_string* string = json_from_string(ctx, "foo");    // creates JSON string "foo"
	json_number* number = json_from_number(ctx, 20);       // creates JSON number 20
	json_boolean* boolean = json_from_boolean(ctx, false); // creates JSON false value 
```

### Object/Array filling

Both objects and arrays are one-way mutable. You can push new values into them, but you can't delete values already in them. For objects, you can push in multiple equal keys, and it will produce JSON that can be deserialized by AOJLS, however, might be invalid for other deserializers.

To push new key-value pair into object, use:

```c
	json_object_add(object, "foo", (json_value_t*)json_value);
```

To push new value into array, use:

```c
	json_array_add(array, (json_value_t*)json_value);
```

**Warning**: You may create nested object, however any attempt to serialize those will end up with stack overflow!

For more operations on objects/arrays see API.

### Serialization

To serialize some JSON value, simply use `aojls_serialize`. This function takes `json_value_t` reference and reference to `aojls_serialization_prefs` which may be `NULL`, in which case default preferences will be used. `aojls_serialize` returns `char*` with result, **if** you use default preferences or you do not specify custom writer. Otherwise, or in case of failure, it returns `NULL`. If it does return non`NULL` value, you must deallocate it via `free` if you are done using it. In case of failure, if you provided preferences, it will also put `false` into `aojls_serialization_prefs.success`.

Example serialization:

```c
    aojls_serialization_prefs p;
	p.eol = NULL;
	p.pretty = false;
	p.offset_per_level = 4;
	p.writer = NULL;
	p.number_formatter = NULL;

	char* result = aojls_serialize((json_value_t*)root, &p);
```

For more options about serialization, see API.

### Deserialization

If you need to deserialize, you either can provide a context in which new deserialized objects will residue, or deserialization will make new context for you. Use function `aojls_deserialize`, which takes three parameters, a `char*` string (does not need to be `\0` terminated), `size_t` size of previous string and `aojls_deserialization_prefs`. `aojls_deserialization_prefs` may be `NULL`, in which case default preferences are used. If you do not specify custom reader, provided string must be valid and length must be correct, otherwise string may be `NULL`. `aojls_deserialize` returns reference to `aojls_ctx_t` (either new, or provided). You can use `json_context_get_result` to get the resulting deserialized object from this context. In case of failure, `json_context_get_result` will return NULL and provided preferences, if any, will contain error description. 

Example deserialization:

```c
	aojls_deserialization_prefs dp;
	memset(&dp, 0, sizeof(aojls_deserialization_prefs));

	aojls_ctx_t* context = aojls_deserialize(source, strlen(source), &dp);
	json_value_t* result = json_context_get_result(context);
```

For more options about deserialization (including providing context yourself), see API. 

### Value liveness & memory leak prevention

All JSON values's memory is tracked by the context they residue in. If you want to free all the memory, simply use `json_free_context` as in:

```c
	json_free_context(context); // after this point, all references to JSON values held in this context
								// become invalid and dangling!
```

When providing string keys for objects, they are copied upon call and copies are tracked by context, so you can do whatever you want with strings after the call, ie:

```c
	char* key = (char*)malloc(sizeof(char)*4); // allocate 4 bytes
	memcpy(key, "foo", 4);                     // copy foo and \0 into key
	json_object_add(object, key, json_value);  // at this point, key is copied 
											   // and stored in the context
	free(key);                                 // valid operation
```

String passed into deserialization is not modified and can be freed/modified after deserialization is finished. String returned from serialization needs to be freed by programmer. 

### Error checking

All functions in AOJLS give some way of error checking, either by sentinel value or via function call/validity pass-value. However, you only need to do it when absolutely necessary. All operations must succeed in some way and cannot cause any failure in AOJLS. Therefore, for instance, if you are building some deeply nested JSON value, you can first build that structure completely, and at the end, check whether there were any issues. You can do that via `json_context_error_happened` at any time. Deserialization will also add error string with explanation in `aojls_deserialization_prefs.error`. Serialization will instead announce success via setting `aojls_serialization_prefs.success` to `true`.

## API
Autogenerated documentation is available here: 

## Errors & Problems

If you find any problems or errors, feel free to submit new issue, if there is no such issue already submitted.
