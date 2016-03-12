# aojls

AOJLS - "All other JSON libraries suck" is JSON parsing/generating library that is not aiming for speed or efficiency, but instead aims for programmer's comfort. The main two principles in AOJLS is that it should be easy to create JSON objects, it should not fail to create JSON object, and if so, programmer should not bother checking that every single time, and lastly, it should be easy to deallocate bunch of JSON objects that are related without doing that manually. For these purposes, aojls employs these techniques:

* easy to use api for object creation and usage
* all objects require context and their liveness is bound by it
* in case of failure, context is marked and all operations on the JSON data continue to work in a fashion that they will not abort/cause sigsegv. All you need to do is check context, whenever you want, instead of after each action (you can still do that though).
