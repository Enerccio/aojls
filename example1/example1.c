/**
 * Copyright (c) 2016, Peter Vanusanik
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * * Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 *
 * * Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 *
 * * Neither the name of AOJLS nor the names of its
 *   contributors may be used to endorse or promote products derived from
 *   this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <aojls.h>

#include <stdio.h>
#include <stdbool.h>

int main() {
	aojls_ctx_t* ctx = json_make_context();

	json_object* root = json_make_object(ctx);
	json_object_add(root, "key1", (json_value_t*)json_from_string(ctx, "value"));
	json_object_add(root, "key2", (json_value_t*)json_from_number(ctx, 20));
	json_object_add(root, "key3", (json_value_t*)json_from_boolean(ctx, false));
	json_object_add(root, "key4", (json_value_t*)json_make_null(ctx));

	json_array* arr = json_make_array(ctx);
	json_array_add(arr, (json_value_t*)json_from_string(ctx, "foo"));
	json_array_add(arr, (json_value_t*)json_from_string(ctx, "bar"));
	json_array_add(arr, (json_value_t*)json_from_string(ctx, "baz"));
	json_array_add(arr, (json_value_t*)json_from_number(ctx, 123.123));
	json_object_add(root, "key \\\" 5", (json_value_t*)arr);

	json_object* o = json_make_object(ctx);
	json_object_add(o, "1key1", (json_value_t*)json_from_string(ctx, "\t\n\r\\"));
	json_object_add(o, "1key2", (json_value_t*)json_from_number(ctx, 0.1+0.2));
	json_object_add(o, "1key3", (json_value_t*)json_from_boolean(ctx, false));
	json_object_add(o, "1key4", (json_value_t*)json_make_null(ctx));
	json_object_add(root, "key6", (json_value_t*)o);

	aojls_serialization_prefs p;
	p.eol = NULL;
	p.pretty = false;
	p.offset_per_level = 4;
	p.writer = NULL;
	p.number_formatter = NULL;
	p.number_formatter = "%.17g";

	char* result = aojls_serialize((json_value_t*)root, &p);
	printf(result);
	free(result);

	printf("\n\n");

	p.writer = NULL;
	p.pretty = true;
	result = aojls_serialize((json_value_t*)root, &p);
	printf(result);

	aojls_deserialization_prefs dp;
	memset(&dp, 0, sizeof(aojls_deserialization_prefs));

	aojls_ctx_t* ctx2 = aojls_deserialize(result, strlen(result), &dp);
	json_value_t* rr = json_context_get_result(ctx2);

	p.writer = NULL;
	char* result2 = aojls_serialize((json_value_t*)rr, &p);
	if (strcmp(result, result2) != 0) {
		printf(result2);
		exit(-1);
	}

	json_free_context(ctx);
	json_free_context(ctx2);
	free(result);
	free(result2);

	printf("\nCorrect! \n");
}
