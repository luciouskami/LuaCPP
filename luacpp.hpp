/*
*	LuaJIT 2.0.2 C++
*	Author: Nestharus
*
*	Class wrapper for Lua C API, Debug Library, and Auxiliary Library
*
*	Includes a bytecode compiler
*
*
*	Documentation Source: http://www.lua.org/manual/5.2/manual.html
*		
*/

#pragma once

#include <fstream>

#include "lua.hpp"

class Lua {
public:
	/*
	*	typedefs
	*/

	//The type of the memory-allocation function used by Lua states. The allocator function must provide a functionality similar to realloc, but not exactly the same. Its arguments are ud, an opaque pointer passed to lua_newstate; ptr, a pointer to the block being allocated/reallocated/freed; osize, the original size of the block or some code about what is being allocated; nsize, the new size of the block.
	//
	//When ptr is not NULL, osize is the size of the block pointed by ptr, that is, the size given when it was allocated or reallocated.
	//
	//When ptr is NULL, osize encodes the kind of object that Lua is allocating. osize is any of LUA_TSTRING, LUA_TTABLE, LUA_TFUNCTION, LUA_TUSERDATA, or LUA_TTHREAD when (and only when) Lua is creating a new object of that type. When osize is some other value, Lua is allocating memory for something else.
	//
	//Lua assumes the following behavior from the allocator function:
	//
	//When nsize is zero, the allocator should behave like free and return NULL.
	//
	//When nsize is not zero, the allocator should behave like realloc. The allocator returns NULL if and only if it cannot fulfill the request. Lua assumes that the allocator never fails when osize >= nsize.
	//
	//Here is a simple implementation for the allocator function. It is used in the auxiliary library by luaL_newstate.
	//
	//	 static void *l_alloc (void *ud, void *ptr, size_t osize,
	//												size_t nsize) {
	//	   (void)ud;  (void)osize;  /* not used */
	//	   if (nsize == 0) {
	//		 free(ptr);
	//		 return NULL;
	//	   }
	//	   else
	//		 return realloc(ptr, nsize);
	//	 }
	//
	//Note that Standard C ensures that free(NULL) has no effect and that realloc(NULL, size) is equivalent to malloc(size). This code assumes that realloc does not fail when shrinking a block. (Although Standard C does not ensure this behavior, it seems to be a safe assumption.)
	typedef lua_Alloc Alloc;
	//Type for C functions.
	//
	//In order to communicate properly with Lua, a C function must use the following protocol, which defines the way parameters and results are passed: a C function receives its arguments from Lua in its stack in direct order (the first argument is pushed first). So, when the function starts, lua_gettop(L) returns the number of arguments received by the function. The first argument (if any) is at index 1 and its last argument is at index lua_gettop(L). To return values to Lua, a C function just pushes them onto the stack, in direct order (the first result is pushed first), and returns the number of results. Any other value in the stack below the results will be properly discarded by Lua. Like a Lua function, a C function called by Lua can also return many results.
	//
	//As an example, the following function receives a variable number of numerical arguments and returns their average and sum:
	//
	//	 static int foo (lua_State *L) {
	//	   int n = lua_gettop(L);    /* number of arguments */
	//	   lua_Number sum = 0;
	//	   int i;
	//	   for (i = 1; i <= n; i++) {
	//		 if (!lua_isnumber(L, i)) {
	//		   lua_pushstring(L, "incorrect argument");
	//		   lua_error(L);
	//		 }
	//		 sum += lua_tonumber(L, i);
	//	   }
	//	   lua_pushnumber(L, sum/n);		/* first result */
	//	   lua_pushnumber(L, sum);		/* second result */
	//	   return 2;				/* number of results */
	//	 }
	typedef lua_CFunction CFunction;
	//The type used by the Lua API to represent signed integral values.
	//
	//By default it is a ptrdiff_t, which is usually the largest signed integral type the machine handles "comfortably".
	typedef lua_Integer Integer;
	//The type of numbers in Lua. By default, it is double, but that can be changed in luaconf.h. Through this configuration file you can change Lua to operate with another type for numbers (e.g., float or long).
	typedef lua_Number Number;
	//The reader function used by lua_load. Every time it needs another piece of the chunk, lua_load calls the reader, passing along its data parameter. The reader must return a pointer to a block of memory with a new piece of the chunk and set size to the block size. The block must exist until the reader function is called again. To signal the end of the chunk, the reader must return NULL or set size to zero. The reader function may return pieces of any size greater than zero.
	typedef lua_Reader Reader;
	//An opaque structure that points to a thread and indirectly (through the thread) to the whole state of a Lua interpreter. The Lua library is fully reentrant: it has no global variables. All information about a state is accessible through this structure.
	//
	//A pointer to this structure must be passed as the first argument to every function in the library, except to lua_newstate, which creates a Lua state from scratch.
	typedef lua_State State;
	//The type of the writer function used by lua_dump. Every time it produces another piece of chunk, lua_dump calls the writer, passing along the buffer to be written (p), its size (sz), and the data parameter supplied to lua_dump.
	//
	//The writer returns an error code: 0 means no errors; any other value means an error and stops lua_dump from calling the writer again.
	typedef lua_Writer Writer;
	//A structure used to carry different pieces of information about a function or an activation record. lua_getstack fills only the private part of this structure, for later use. To fill the other fields of lua_Debug with useful information, call lua_getinfo.
	//
	//The fields of lua_Debug have the following meaning:
	//
	//	source: the source of the chunk that created the function. If source starts with a '@', it means that the function was defined in a file where the file name follows the '@'. If source starts with a '=', the remainder of its contents describe the source in a user-dependent manner. Otherwise, the function was defined in a string where source is that string.
	//	short_src: a "printable" version of source, to be used in error messages.
	//	linedefined: the line number where the definition of the function starts.
	//	lastlinedefined: the line number where the definition of the function ends.
	//	what: the string "Lua" if the function is a Lua function, "C" if it is a C function, "main" if it is the main part of a chunk.
	//	currentline: the current line where the given function is executing. When no line information is available, currentline is set to -1.
	//	name: a reasonable name for the given function. Because functions in Lua are first-class values, they do not have a fixed name: some functions can be the value of multiple global variables, while others can be stored only in a table field. The lua_getinfo function checks how the function was called to find a suitable name. If it cannot find a name, then name is set to NULL.
	//	namewhat: explains the name field. The value of namewhat can be "global", "local", "method", "field", "upvalue", or "" (the empty string), according to how the function was called. (Lua uses the empty string when no other option seems to apply.)
	//	istailcall: true if this function invocation was called by a tail call. In this case, the caller of this level is not in the stack.
	//	nups: the number of upvalues of the function.
	//	nparams: the number of fixed parameters of the function (always 0 for C functions).
	//	isvararg: true if the function is a vararg function (always true for C functions).
	typedef lua_Debug Debug;
	//Type for debugging hook functions.
	//
	//Whenever a hook is called, its ar argument has its field event set to the specific event that triggered the hook. Lua identifies these events with the following constants: LUA_HOOKCALL, LUA_HOOKRET, LUA_HOOKTAILCALL, LUA_HOOKLINE, and LUA_HOOKCOUNT. Moreover, for line events, the field currentline is also set. To get the value of any other field in ar, the hook must call lua_getinfo.
	//
	//For call events, event can be LUA_HOOKCALL, the normal value, or LUA_HOOKTAILCALL, for a tail call; in this case, there will be no corresponding return event.
	//
	//While Lua is running a hook, it disables other calls to hooks. Therefore, if a hook calls back Lua to execute a function or a chunk, this execution occurs without any calls to hooks.
	//
	//Hook functions cannot have continuations, that is, they cannot call lua_yieldk, lua_pcallk, or lua_callk with a non-null k.
	//
	//Hook functions can yield under the following conditions: Only count and line events can yield and they cannot yield any value; to yield a hook function must finish its execution calling lua_yield with nresults equal to zero.
	typedef lua_Hook Hook;
	//Type for a string buffer.
	//
	//A string buffer allows C code to build Lua strings piecemeal. Its pattern of use is as follows:
	//
	//First declare a variable b of type luaL_Buffer.
	//Then initialize it with a call luaL_buffinit(L, &b).
	//Then add string pieces to the buffer calling any of the luaL_add* functions.
	//Finish by calling luaL_pushresult(&b). This call leaves the final string on the top of the stack.
	//If you know beforehand the total size of the resulting string, you can use the buffer like this:
	//
	//First declare a variable b of type luaL_Buffer.
	//Then initialize it and preallocate a space of size sz with a call luaL_buffinitsize(L, &b, sz).
	//Then copy the string into that space.
	//Finish by calling luaL_pushresultsize(&b, sz), where sz is the total size of the resulting string copied into that space.
	//During its normal operation, a string buffer uses a variable number of stack slots. So, while using a buffer, you cannot assume that you know where the top of the stack is. You can use the stack between successive calls to buffer operations as long as that use is balanced; that is, when you call a buffer operation, the stack is at the same level it was immediately after the previous buffer operation. (The only exception to this rule is luaL_addvalue.) After calling luaL_pushresult the stack is back to its level when the buffer was initialized, plus the final string on its top.
	typedef luaL_Buffer l_Buffer;
	//Type for arrays of functions to be registered by luaL_setfuncs. name is the function name and func is a pointer to the function. Any array of luaL_Reg must end with an sentinel entry in which both name and func are NULL.
	typedef luaL_Reg l_Reg;

	private:
		State* L;
		bool dependent;

		static class Compiler {
			private:
				static std::fstream file;

				//writer for compiler
				static int writer(State* L, const void* p, size_t size, void* u) {
					file.write((const char*)p, size);
					return 0;
				} //writer

			public:
				//execute compiler
				static bool execute(State* L, char* input, const char* output) {
					file.open(input, std::ios::in | std::ios::binary);
					if (!file.is_open()) {
						return false;
					}
					luaL_loadfile(L, input);
					file.close();

					file.open(output, std::ios::out | std::ios::binary);
					if (!file.is_open()) {
						return false;
					}
					lua_dump(L, writer, NULL);
					file.close();

					return true;
				} //compile
		}; //Compiler

		//hide = operator
		Lua* operator=(Lua& rhs) { return nullptr; }

		Lua(State* L) { this->L = L; dependent = true; }
	public:

		/*
		*	Creators/Destructors
		*/

		//Creates a new Lua state. It calls lua_newstate with an allocator based on the standard C realloc function and then sets a panic function that prints an error message to the standard error output in case of fatal errors.
		//
		//Returns the new state, or NULL if there is a memory allocation error.
		Lua() { L = luaL_newstate(); dependent = false;}
		//Creates a new thread running in a new, independent state. Returns NULL if cannot create the thread or the state (due to lack of memory). The argument f is the allocator function; Lua does all memory allocation for this state through this function. The second argument, ud, is an opaque pointer that Lua passes to the allocator in every call.
		Lua(Alloc f, void* ud) { L = lua_newstate(f, ud); dependent = false;}
		//Creates a new thread, pushes it on the stack, and returns a pointer to a lua_State that represents this new thread. The new thread returned by this function shares with the original thread its global environment, but has an independent execution stack.
		//
		//There is no explicit function to close or to destroy a thread. Threads are subject to garbage collection, like any Lua object.
		Lua(Lua& lua) { L = lua_newthread(lua.L); dependent = true;}
		
		//Destroys all objects in the given Lua state (calling the corresponding garbage-collection metamethods, if any) and frees all dynamic memory used by this state. On several platforms, you may not need to call this function, because all resources are naturally released when the host program ends. On the other hand, long-running programs that create multiple states, such as daemons or web servers, might need to close states as soon as they are not needed.
		~Lua() { if (!dependent) lua_close(L); }

		/*
		*	Methods
		*/

		//compiles an input file to bytecode, outputs to output file
		inline bool compile(char* input, const char* output) { return Compiler::execute(L, input, output); }
		//returns true if Lua object is a child thread of another Lua object
		inline bool isdependent() { return dependent; }

		/*
		*	C API
		*/

		//Sets a new panic function and returns the old one.
		inline CFunction atpanic(CFunction panicf) { return lua_atpanic(L, panicf); }
		//Calls a function.
		//
		//To call a function you must use the following protocol: first, the function to be called is pushed onto the stack; then, the arguments to the function are pushed in direct order; that is, the first argument is pushed first. Finally you call lua_call; nargs is the number of arguments that you pushed onto the stack. All arguments and the function value are popped from the stack when the function is called. The function results are pushed onto the stack when the function returns. The number of results is adjusted to nresults, unless nresults is LUA_MULTRET. In this case, all results from the function are pushed. Lua takes care that the returned values fit into the stack space. The function results are pushed onto the stack in direct order (the first result is pushed first), so that after the call the last result is on the top of the stack.
		//
		//Any error inside the called function is propagated upwards (with a longjmp).
		//
		//The following example shows how the host program can do the equivalent to this Lua code:
		//
		//	 a = f("how", t.x, 14)
		//
		//Here it is in C:
		//
		//	 lua_getglobal(L, "f");								/* function to be called */
		//	 lua_pushstring(L, "how");							/* 1st argument */
		//	 lua_getglobal(L, "t");								/* table to be indexed */
		//	 lua_getfield(L, -1, "x");								/* push result of t.x (2nd arg) */
		//	 lua_remove(L, -2);								/* remove 't' from the stack */
		//	 lua_pushinteger(L, 14);								/* 3rd argument */
		//	 lua_call(L, 3, 1);									/* call 'f' with 3 arguments and 1 result */
		//	 lua_setglobal(L, "a");								/* set global 'a' */
		//
		//Note that the code above is "balanced": at its end, the stack is back to its original configuration. This is considered good programming practice.
		inline void call(int nargs, int nresults) { lua_call(L, nargs, nresults); }
		//Ensures that there are at least extra free stack slots in the stack. It returns false if it cannot fulfill the request, because it would cause the stack to be larger than a fixed maximum size (typically at least a few thousand elements) or because it cannot allocate memory for the new stack size. This function never shrinks the stack; if the stack is already larger than the new size, it is left unchanged.
		inline int checkstack(int extra) { return lua_checkstack(L, extra); }
		//Concatenates the n values at the top of the stack, pops them, and leaves the result at the top. If n is 1, the result is the single value on the stack (that is, the function does nothing); if n is 0, the result is the empty string. Concatenation is performed following the usual semantics of Lua.
		inline void concat(int n) { lua_concat(L, n); }
		//Calls the C function func in protected mode. func starts with only one element in its stack, a light userdata containing ud. In case of errors, lua_cpcall returns the same error codes as lua_pcall, plus the error object on the top of the stack; otherwise, it returns zero, and does not change the stack. All values returned by func are discarded.
		inline int cpcall(CFunction func, void* ud) { return lua_cpcall(L, func, ud); }
		//Creates a new empty table and pushes it onto the stack. Parameter narr is a hint for how many elements the table will have as a sequence; parameter nrec is a hint for how many other elements the table will have. Lua may use these hints to preallocate memory for the new table. This pre-allocation is useful for performance when you know in advance how many elements the table will have. Otherwise you can use the function lua_newtable.
		inline void createtable(int narr, int nrec) { lua_createtable(L, narr, nrec); }
		//Dumps a function as a binary chunk. Receives a Lua function on the top of the stack and produces a binary chunk that, if loaded again, results in a function equivalent to the one dumped. As it produces parts of the chunk, lua_dump calls function writer (see lua_Writer) with the given data to write them.
		//
		//The value returned is the error code returned by the last call to the writer; 0 means no errors.
		//
		//This function does not pop the Lua function from the stack.
		inline int dump(Writer writer, void *data) { return lua_dump(L, writer, data); }
		//Returns 1 if the two values in acceptable indices index1 and index2 are equal, following the semantics of the Lua == operator (that is, may call metamethods). Otherwise returns 0. Also returns 0 if any of the indices is non valid.
		inline int equal(int index1, int index2) { return lua_equal(L, index1, index2); }
		//Generates a Lua error. The error message (which can actually be a Lua value of any type) must be on the stack top. This function does a long jump, and therefore never returns (see luaL_error).
		inline int error() { return lua_error(L); }
		//Controls the garbage collector.
		//
		//This function performs several tasks, according to the value of the parameter what:
		//
		//	LUA_GCSTOP: stops the garbage collector.
		//	LUA_GCRESTART: restarts the garbage collector.
		//	LUA_GCCOLLECT: performs a full garbage-collection cycle.
		//	LUA_GCCOUNT: returns the current amount of memory (in Kbytes) in use by Lua.
		//	LUA_GCCOUNTB: returns the remainder of dividing the current amount of bytes of memory in use by Lua by 1024.
		//	LUA_GCSTEP: performs an incremental step of garbage collection. The step "size" is controlled by data (larger values mean more steps) in a non-specified way. If you want to control the step size you must experimentally tune the value of data. The function returns 1 if the step finished a garbage-collection cycle.
		//	LUA_GCSETPAUSE: sets data as the new value for the pause of the collector (see §2.5). The function returns the previous value of the pause.
		//	LUA_GCSETSTEPMUL: sets data as the new value for the step multiplier of the collector (see §2.5). The function returns the previous value of the step multiplier.
		//	LUA_GCISRUNNING: returns a boolean that tells whether the collector is running (i.e., not stopped).
		//	LUA_GCGEN: changes the collector to generational mode (see §2.5).
		//	LUA_GCINC: changes the collector to incremental mode. This is the default mode.
		//
		//For more details about these options, see collectgarbage.
		inline int gc(int what, int data) { return lua_gc(L, what, data); }
		//Returns the memory-allocation function of a given state. If ud is not NULL, Lua stores in *ud the opaque pointer passed to lua_newstate.
		inline Alloc getallocf(void** ud) { return lua_getallocf(L, ud); }
		//Pushes onto the stack the environment table of the value at the given index.
		inline void getfenv(int index) { lua_getfenv(L, index); }
		//Pushes onto the stack the value t[k], where t is the value at the given index. As in Lua, this function may trigger a metamethod for the "index" event.
		inline void getfield(int index, const char *k) { lua_getfield(L, index, k); }
		//Pushes onto the stack the value of the global name.
		inline void getglobal(const char *name) { lua_getglobal(L, name); }
		//Pushes onto the stack the metatable of the value at the given index. If the value does not have a metatable, the function returns 0 and pushes nothing on the stack.
		inline int getmetatable(int index) { return lua_getmetatable(L, index); }
		//Pushes onto the stack the value t[k], where t is the value at the given index and k is the value at the top of the stack.
		//
		//This function pops the key from the stack (putting the resulting value in its place). As in Lua, this function may trigger a metamethod for the "index" event.
		inline void gettable(int index) { lua_gettable(L, index); }
		//Returns the index of the top element in the stack. Because indices start at 1, this result is equal to the number of elements in the stack (and so 0 means an empty stack).
		inline int gettop() { return lua_gettop(L); }
		//Moves the top element into the given valid index, shifting up the elements above this index to open space. This function cannot be called with a pseudo-index, because a pseudo-index is not an actual stack position.
		inline void insert(int index) { lua_insert(L, index); }
		//Returns 1 if the value at the given index is a boolean, and 0 otherwise.
		inline int isboolean(int index) { return lua_isboolean(L, index); }
		//Returns 1 if the value at the given index is a C function, and 0 otherwise.
		inline int iscfunction(int index) { return lua_iscfunction(L, index); }
		//Returns 1 if the value at the given index is a function (either C or Lua), and 0 otherwise.
		inline int isfunction(int index) { return lua_isfunction(L, index); }
		//Returns 1 if the value at the given index is a light userdata, and 0 otherwise.
		inline int islightuserdata(int index) { return lua_islightuserdata(L, index); }
		//Returns 1 if the value at the given index is nil, and 0 otherwise.
		inline int isnil(int index) { return lua_isnil(L, index); }
		//Returns 1 if the given index is not valid, and 0 otherwise.
		inline int isnone(int index) { return lua_isnone(L, index); }
		//Returns 1 if the given index is not valid or if the value at this index is nil, and 0 otherwise.
		inline int isnoneornil(int index) { return lua_isnoneornil(L, index); }
		//Returns 1 if the value at the given index is a number or a string convertible to a number, and 0 otherwise.
		inline int isnumber(int index) { return lua_isnumber(L, index); }
		//Returns 1 if the value at the given index is a string or a number (which is always convertible to a string), and 0 otherwise.
		inline int isstring(int index) { return lua_isstring(L, index); }
		//Returns 1 if the value at the given index is a table, and 0 otherwise.
		inline int istable(int index) { return lua_istable(L, index); }
		//Returns 1 if the value at the given index is a thread, and 0 otherwise.
		inline int isthread(int index) { return lua_isthread(L, index); }
		//Returns 1 if the value at the given index is a userdata (either full or light), and 0 otherwise.
		inline int isuserdata(int index) { return lua_isuserdata(L, index); }
		//Returns 1 if the value at acceptable index index1 is smaller than the value at acceptable index index2, following the semantics of the Lua < operator (that is, may call metamethods). Otherwise returns 0. Also returns 0 if any of the indices is non valid.
		inline int lessthan(int index1, int index2) { return lua_lessthan(L, index1, index2); }
		//Loads a Lua chunk (without running it). If there are no errors, lua_load pushes the compiled chunk as a Lua function on top of the stack. Otherwise, it pushes an error message.
		//
		//The return values of lua_load are:
		//
		//	LUA_OK: no errors;
		//	LUA_ERRSYNTAX: syntax error during precompilation;
		//	LUA_ERRMEM: memory allocation error;
		//	LUA_ERRGCMM: error while running a __gc metamethod. (This error has no relation with the chunk being loaded. It is generated by the garbage collector.)
		//
		//The lua_load function uses a user-supplied reader function to read the chunk (see lua_Reader). The data argument is an opaque value passed to the reader function.
		//
		//The source argument gives a name to the chunk, which is used for error messages and in debug information (see §4.9).
		//
		//lua_load automatically detects whether the chunk is text or binary and loads it accordingly (see program luac). The string mode works as in function load, with the addition that a NULL value is equivalent to the string "bt".
		//
		//lua_load uses the stack internally, so the reader function should always leave the stack unmodified when returning.
		//
		//If the resulting function has one upvalue, this upvalue is set to the value of the global environment stored at index LUA_RIDX_GLOBALS in the registry (see §4.5). When loading main chunks, this upvalue will be the _ENV variable.
		inline int load(Reader reader, void* data, const char* source) { return lua_load(L, reader, data, source); }
		//Creates a new empty table and pushes it onto the stack. It is equivalent to lua_createtable(L, 0, 0).
		inline void newtable() { lua_newtable(L); }
		//This function allocates a new block of memory with the given size, pushes onto the stack a new full userdata with the block address, and returns this address. The host program can freely use this memory.
		inline void* newuserdata(size_t size) { return lua_newuserdata(L, size); }
		//Pops a key from the stack, and pushes a key–value pair from the table at the given index (the "next" pair after the given key). If there are no more elements in the table, then lua_next returns 0 (and pushes nothing).
		//
		//A typical traversal looks like this:
		//
		//	 /* table is in the stack at index 't' */
		//	 lua_pushnil(L);  /* first key */
		//	 while (lua_next(L, t) != 0) {
		//	   /* uses 'key' (at index -2) and 'value' (at index -1) */
		//	   printf("%s - %s\n",
		//			  lua_typename(L, lua_type(L, -2)),
		//			  lua_typename(L, lua_type(L, -1)));
		//	   /* removes 'value'; keeps 'key' for next iteration */
		//	   lua_pop(L, 1);
		//	 }
		//
		//While traversing a table, do not call lua_tolstring directly on a key, unless you know that the key is actually a string. Recall that lua_tolstring may change the value at the given index; this confuses the next call to lua_next.
		//
		//See function next for the caveats of modifying the table during its traversal.
		inline int next(int index) { return lua_next(L, index); }
		//Returns the "length" of the value at the given acceptable index: for strings, this is the string length; for tables, this is the result of the length operator ('#'); for userdata, this is the size of the block of memory allocated for the userdata; for other values, it is 0.
		inline size_t objlen(int index) { return lua_objlen(L, index); }
		//Calls a function in protected mode.
		//
		//Both nargs and nresults have the same meaning as in lua_call. If there are no errors during the call, lua_pcall behaves exactly like lua_call. However, if there is any error, lua_pcall catches it, pushes a single value on the stack (the error message), and returns an error code. Like lua_call, lua_pcall always removes the function and its arguments from the stack.
		//
		//If msgh is 0, then the error message returned on the stack is exactly the original error message. Otherwise, msgh is the stack index of a message handler. (In the current implementation, this index cannot be a pseudo-index.) In case of runtime errors, this function will be called with the error message and its return value will be the message returned on the stack by lua_pcall.
		//
		//Typically, the message handler is used to add more debug information to the error message, such as a stack traceback. Such information cannot be gathered after the return of lua_pcall, since by then the stack has unwound.
		//
		//The lua_pcall function returns one of the following codes (defined in lua.h):
		//
		//	LUA_OK (0): success.
		//	LUA_ERRRUN: a runtime error.
		//	LUA_ERRMEM: memory allocation error. For such errors, Lua does not call the message handler.
		//	LUA_ERRERR: error while running the message handler.
		//	LUA_ERRGCMM: error while running a __gc metamethod. (This error typically has no relation with the function being called. It is generated by the garbage collector.)
		inline int pcall(int nargs, int nresults, int msgh) { return lua_pcall(L, nargs, nresults, msgh); }
		//Pops n elements from the stack.
		inline void pop(int n) { lua_pop(L, n); }
		//Pushes a boolean value with value b onto the stack.
		inline void	pushboolean(bool b) { lua_pushboolean(L, b); }
		//Pushes a new C closure onto the stack.
		//
		//When a C function is created, it is possible to associate some values with it, thus creating a C closure (see §4.4); these values are then accessible to the function whenever it is called. To associate values with a C function, first these values should be pushed onto the stack (when there are multiple values, the first value is pushed first). Then lua_pushcclosure is called to create and push the C function onto the stack, with the argument n telling how many values should be associated with the function. lua_pushcclosure also pops these values from the stack.
		//
		//The maximum value for n is 255.
		//
		//When n is zero, this function creates a light C function, which is just a pointer to the C function. In that case, it never throws a memory error.
		inline void	pushccloser(CFunction fn, int n) { lua_pushcclosure(L, fn, n); }
		//Pushes a C function onto the stack. This function receives a pointer to a C function and pushes onto the stack a Lua value of type function that, when called, invokes the corresponding C function.
		//
		//Any function to be registered in Lua must follow the correct protocol to receive its parameters and return its results (see lua_CFunction).
		//
		//lua_pushcfunction is defined as a macro:
		//
		//		#define lua_pushcfunction(L,f)  lua_pushcclosure(L,f,0)
		//
		//Note that f is used twice.
		inline void	pushcfunction(CFunction f) { lua_pushcfunction(L, f); }
		//Pushes onto the stack a formatted string and returns a pointer to this string. It is similar to the ANSI C function sprintf, but has some important differences:
		//
		//	You do not have to allocate space for the result: the result is a Lua string and Lua takes care of memory allocation (and deallocation, through garbage collection).
		//	The conversion specifiers are quite restricted. There are no flags, widths, or precisions. The conversion specifiers can only be '%%' (inserts a '%' in the string), '%s' (inserts a zero-terminated string, with no size restrictions), '%f' (inserts a lua_Number), '%p' (inserts a pointer as a hexadecimal numeral), '%d' (inserts an int), and '%c' (inserts an int as a byte).
		inline const char* pushfstring(const char* fmt, ...) { va_list argptr; va_start(argptr,fmt); return lua_pushfstring(L, fmt, argptr); }
		//Pushes a number with value n onto the stack.
		inline void pushinteger(Integer n) { lua_pushinteger(L, n); }
		//Pushes a light userdata onto the stack.
		//
		//Userdata represent C values in Lua. A light userdata represents a pointer, a void*. It is a value (like a number): you do not create it, it has no individual metatable, and it is not collected (as it was never created). A light userdata is equal to "any" light userdata with the same C address.
		inline void pushlightuserdata(void* p) { lua_pushlightuserdata(L, p); }
		//This macro is equivalent to lua_pushlstring, but can be used only when s is a literal string. It automatically provides the string length.
		inline const char* pushliteral(const char* s) { lua_pushlstring(L, s, sizeof(s)/sizeof(char) - 1); }
		//Pushes the string pointed to by s with size len onto the stack. Lua makes (or reuses) an internal copy of the given string, so the memory at s can be freed or reused immediately after the function returns. The string can contain any binary data, including embedded zeros.
		//
		//Returns a pointer to the internal copy of the string.
		inline void pushlstring(const char* s, size_t len) { lua_pushlstring(L, s, len); }
		//Pushes a nil value onto the stack.
		inline void pushnil() { lua_pushnil(L); }
		//Pushes a number with value n onto the stack.
		inline void pushnumber(Number n) { lua_pushnumber(L, n); }
		//Pushes the zero-terminated string pointed to by s onto the stack. Lua makes (or reuses) an internal copy of the given string, so the memory at s can be freed or reused immediately after the function returns.
		//
		//Returns a pointer to the internal copy of the string.
		//
		//If s is NULL, pushes nil and returns NULL.
		inline void pushstring(const char* s) { lua_pushstring(L, s); }
		//Pushes the thread represented by L onto the stack. Returns 1 if this thread is the main thread of its state.
		inline int pushthread() { return lua_pushthread(L); }
		//Pushes a copy of the element at the given index onto the stack.
		inline void pushvalue(int index) { lua_pushvalue(L, index); }
		//Equivalent to lua_pushfstring, except that it receives a va_list instead of a variable number of arguments.
		inline const char* pushvfstring(const char* fmt, va_list argp) { return lua_pushvfstring(L, fmt, argp); }
		//Returns 1 if the two values in indices index1 and index2 are primitively equal (that is, without calling metamethods). Otherwise returns 0. Also returns 0 if any of the indices are non valid.
		inline int rawequal(int index1, int index2) { return lua_rawequal(L, index1, index2); }
		//Similar to lua_gettable, but does a raw access (i.e., without metamethods).
		inline void rawget(int index) { lua_rawget(L, index); }
		//Pushes onto the stack the value t[n], where t is the table at the given index. The access is raw; that is, it does not invoke metamethods.
		inline void rawgeti(int index, int n) { lua_rawgeti(L, index, n); }
		//Similar to lua_settable, but does a raw assignment (i.e., without metamethods).
		inline void rawset(int index) { lua_rawset(L, index); }
		//Does the equivalent of t[n] = v, where t is the table at the given index and v is the value at the top of the stack.
		//
		//This function pops the value from the stack. The assignment is raw; that is, it does not invoke metamethods.
		inline void rawseti(int index, int n) { lua_rawseti(L, index, n); }
		//Sets the C function f as the new value of global name. It is defined as a macro:
		inline void registerfunc(const char* name, CFunction f) { lua_register(L, name, f); }
		//Removes the element at the given valid index, shifting down the elements above this index to fill the gap. This function cannot be called with a pseudo-index, because a pseudo-index is not an actual stack position.
		inline void remove(int index) { lua_remove(L, index); }
		//Moves the top element into the given valid index without shifting any element (therefore replacing the value at the given index), and then pops the top element.
		inline void replace(int index) { lua_replace(L, index); }
		//Starts and resumes a coroutine in a given thread.
		//
		//To start a coroutine, you push onto the thread stack the main function plus any arguments; then you call lua_resume, with nargs being the number of arguments. This call returns when the coroutine suspends or finishes its execution. When it returns, the stack contains all values passed to lua_yield, or all values returned by the body function. lua_resume returns LUA_YIELD if the coroutine yields, LUA_OK if the coroutine finishes its execution without errors, or an error code in case of errors (see lua_pcall).
		//
		//In case of errors, the stack is not unwound, so you can use the debug API over it. The error message is on the top of the stack.
		//
		//To resume a coroutine, you remove any results from the last lua_yield, put on its stack only the values to be passed as results from yield, and then call lua_resume.
		//
		//The parameter from represents the coroutine that is resuming L. If there is no such coroutine, this parameter can be NULL.
		inline int resume(int nargs) { return lua_resume(L, nargs); }
		//Changes the allocator function of a given state to f with user data ud.
		inline void setallocf(Alloc f, void* ud) { lua_setallocf(L, f, ud); }
		//Pops a table from the stack and sets it as the new environment for the value at the given index. If the value at the given index is neither a function nor a thread nor a userdata, lua_setfenv returns 0. Otherwise it returns 1.
		inline int setfenv(int index) { return lua_setfenv(L, index); }
		//Does the equivalent to t[k] = v, where t is the value at the given index and v is the value at the top of the stack.
		//
		//This function pops the value from the stack. As in Lua, this function may trigger a metamethod for the "newindex" event.
		inline void setfield(int index, const char* k) { lua_setfield(L, index, k); }
		//Pops a value from the stack and sets it as the new value of global name.
		inline void setglobal(const char* name) { lua_setglobal(L, name); }
		//Pops a table from the stack and sets it as the new metatable for the value at the given index.
		inline void setmetatable(int index) { lua_setmetatable(L, index); }
		//Does the equivalent to t[k] = v, where t is the value at the given index, v is the value at the top of the stack, and k is the value just below the top.
		//
		//This function pops both the key and the value from the stack. As in Lua, this function may trigger a metamethod for the "newindex" event.
		inline void settable(int index) { lua_settable(L, index); }
		//Accepts any index, or 0, and sets the stack top to this index. If the new top is larger than the old one, then the new elements are filled with nil. If index is 0, then all stack elements are removed.
		inline void settop(int index) { lua_settop(L, index); }
		//Returns the status of the thread L.
		//
		//The status can be 0 (LUA_OK) for a normal thread, an error code if the thread finished the execution of a lua_resume with an error, or LUA_YIELD if the thread is suspended.
		//
		//You can only call functions in threads with status LUA_OK. You can resume threads with status LUA_OK (to start a new coroutine) or LUA_YIELD (to resume a coroutine).
		inline int status() { return lua_status(L); }
		//Converts the Lua value at the given index to a C boolean value (0 or 1). Like all tests in Lua, lua_toboolean returns true for any Lua value different from false and nil; otherwise it returns false. (If you want to accept only actual boolean values, use lua_isboolean to test the value's type.)
		inline int toboolean(int index) { return lua_toboolean(L, index); }
		//Converts a value at the given index to a C function. That value must be a C function; otherwise, returns NULL.
		inline CFunction tocfunction(int index) { return lua_tocfunction(L, index); }
		//Equivalent to lua_tointegerx with isnum equal to NULL.
		inline Integer tointeger(int index) { return lua_tointeger(L, index); }
		//Converts the Lua value at the given index to a C string. If len is not NULL, it also sets *len with the string length. The Lua value must be a string or a number; otherwise, the function returns NULL. If the value is a number, then lua_tolstring also changes the actual value in the stack to a string. (This change confuses lua_next when lua_tolstring is applied to keys during a table traversal.)
		//
		//lua_tolstring returns a fully aligned pointer to a string inside the Lua state. This string always has a zero ('\0') after its last character (as in C), but can contain other zeros in its body. Because Lua has garbage collection, there is no guarantee that the pointer returned by lua_tolstring will be valid after the corresponding value is removed from the stack.
		inline const char* tolstring(int index, size_t* len) { return lua_tolstring(L, index, len); }
		//Equivalent to lua_tonumberx with isnum equal to NULL.
		inline Number tonumber(int index) { return lua_tonumber(L, index); }
		//Converts the value at the given index to a generic C pointer (void*). The value can be a userdata, a table, a thread, or a function; otherwise, lua_topointer returns NULL. Different objects will give different pointers. There is no way to convert the pointer back to its original value.
		//
		//Typically this function is used only for debug information.
		inline const void* topointer(int index) { return lua_topointer(L, index); }
		//Equivalent to lua_tolstring with len equal to NULL.
		inline const char* tostring(int index) { return lua_tostring(L, index); }
		//Converts the value at the given index to a Lua thread (represented as lua_State*). This value must be a thread; otherwise, the function returns NULL.
		//
		//Instantiates a new class, be sure to destroy
		inline Lua* tothread(int index) { State* l = lua_tothread(L, index); if (l == 0) return 0; return new Lua(l); }
		//If the value at the given index is a full userdata, returns its block address. If the value is a light userdata, returns its pointer. Otherwise, returns NULL.
		inline void* touserdata(int index) { return lua_touserdata(L, index); }
		//Returns the type of the value in the given valid index, or LUA_TNONE for a non-valid (but acceptable) index. The types returned by lua_type are coded by the following constants defined in lua.h: LUA_TNIL, LUA_TNUMBER, LUA_TBOOLEAN, LUA_TSTRING, LUA_TTABLE, LUA_TFUNCTION, LUA_TUSERDATA, LUA_TTHREAD, and LUA_TLIGHTUSERDATA.
		inline int type(int index) { return lua_type(L, index); }
		//Returns the name of the type encoded by the value tp, which must be one the values returned by lua_type.
		inline const char* vtypename(int tp) { return lua_typename(L, tp); }
		//Returns the pseudo-index that represents the i-th upvalue of the running function.
		static inline int upvalueindex(int i) { return lua_upvalueindex(i); }
		//Exchange values between different threads of the same state.
		//
		//This function pops n values from the stack from, and pushes them onto the stack to.
		inline void xmove(Lua& to, int n) { lua_xmove(L, to.L, n); }
		//This function is equivalent to lua_yieldk, but it has no continuation (see §4.7). Therefore, when the thread resumes, it returns to the function that called the function calling lua_yield.
		inline int yield(int nresults) { return lua_yield(L, nresults); }
		/*
		*	Debug Interface
		*/
		//Returns the current hook function.
		inline Hook gethook() { return lua_gethook(L); }
		//Returns the current hook count.
		inline int gethookcount() { return lua_gethookcount(L); }
		//Returns the current hook mask.
		inline int gethookmask() { return lua_gethookmask(L); }
		//Gets information about a specific function or function invocation.
		//
		//To get information about a function invocation, the parameter ar must be a valid activation record that was filled by a previous call to lua_getstack or given as argument to a hook (see lua_Hook).
		//
		//To get information about a function you push it onto the stack and start the what string with the character '>'. (In that case, lua_getinfo pops the function from the top of the stack.) For instance, to know in which line a function f was defined, you can write the following code:
		//
		//	 lua_Debug ar;
		//	 lua_getglobal(L, "f");  /* get global 'f' */
		//	 lua_getinfo(L, ">S", &ar);
		//	 printf("%d\n", ar.linedefined);
		//
		//Each character in the string what selects some fields of the structure ar to be filled or a value to be pushed on the stack:
		//
		//	'n': fills in the field name and namewhat;
		//	'S': fills in the fields source, short_src, linedefined, lastlinedefined, and what;
		//	'l': fills in the field currentline;
		//	't': fills in the field istailcall;
		//	'u': fills in the fields nups, nparams, and isvararg;
		//	'f': pushes onto the stack the function that is running at the given level;
		//	'L': pushes onto the stack a table whose indices are the numbers of the lines that are valid on the function. (A valid line is a line with some associated code, that is, a line where you can put a break point. Non-valid lines include empty lines and comments.)
		//
		//This function returns 0 on error (for instance, an invalid option in what).
		inline int getinfo(const char* what, Debug* ar) { return lua_getinfo(L, what, ar); }
		//Gets information about a local variable of a given activation record or a given function.
		//
		//In the first case, the parameter ar must be a valid activation record that was filled by a previous call to lua_getstack or given as argument to a hook (see lua_Hook). The index n selects which local variable to inspect; see debug.getlocal for details about variable indices and names.
		//
		//lua_getlocal pushes the variable's value onto the stack and returns its name.
		//
		//In the second case, ar should be NULL and the function to be inspected must be at the top of the stack. In this case, only parameters of Lua functions are visible (as there is no information about what variables are active) and no values are pushed onto the stack.
		//
		//Returns NULL (and pushes nothing) when the index is greater than the number of active local variables.
		inline const char* getlocal(Debug* ar, int n) { return lua_getlocal(L, ar, n); }
		//Gets information about the interpreter runtime stack.
		//
		//This function fills parts of a lua_Debug structure with an identification of the activation record of the function executing at a given level. Level 0 is the current running function, whereas level n+1 is the function that has called level n (except for tail calls, which do not count on the stack). When there are no errors, lua_getstack returns 1; when called with a level greater than the stack depth, it returns 0.
		inline int getstack(int level, Debug* ar) { return lua_getstack(L, level, ar); }
		//Gets information about a closure's upvalue. (For Lua functions, upvalues are the external local variables that the function uses, and that are consequently included in its closure.) lua_getupvalue gets the index n of an upvalue, pushes the upvalue's value onto the stack, and returns its name. funcindex points to the closure in the stack. (Upvalues have no particular order, as they are active through the whole function. So, they are numbered in an arbitrary order.)
		//
		//Returns NULL (and pushes nothing) when the index is greater than the number of upvalues. For C functions, this function uses the empty string "" as a name for all upvalues.
		inline const char* getupvalue(int funcindex, int n) { return lua_getupvalue(L, funcindex, n); }
		//Sets the debugging hook function.
		//
		//Argument f is the hook function. mask specifies on which events the hook will be called: it is formed by a bitwise or of the constants LUA_MASKCALL, LUA_MASKRET, LUA_MASKLINE, and LUA_MASKCOUNT. The count argument is only meaningful when the mask includes LUA_MASKCOUNT. For each event, the hook is called as explained below:
		//
		//	The call hook: is called when the interpreter calls a function. The hook is called just after Lua enters the new function, before the function gets its arguments.
		//	The return hook: is called when the interpreter returns from a function. The hook is called just before Lua leaves the function. There is no standard way to access the values to be returned by the function.
		//	The line hook: is called when the interpreter is about to start the execution of a new line of code, or when it jumps back in the code (even to the same line). (This event only happens while Lua is executing a Lua function.)
		//	The count hook: is called after the interpreter executes every count instructions. (This event only happens while Lua is executing a Lua function.)
		
		//A hook is disabled by setting mask to zero.
		inline int sethook(Hook f, int mask, int count) { return lua_sethook(L, f, mask, count); }
		//Sets the value of a local variable of a given activation record. Parameters ar and n are as in lua_getlocal (see lua_getlocal). lua_setlocal assigns the value at the top of the stack to the variable and returns its name. It also pops the value from the stack.
		//
		//Returns NULL (and pops nothing) when the index is greater than the number of active local variables.
		inline const char* setlocal(Debug* ar, int n) { return lua_setlocal(L, ar, n); }
		//Sets the value of a closure's upvalue. It assigns the value at the top of the stack to the upvalue and returns its name. It also pops the value from the stack. Parameters funcindex and n are as in the lua_getupvalue (see lua_getupvalue).
		//
		//Returns NULL (and pops nothing) when the index is greater than the number of upvalues.
		inline const char* setupvalue(int funcindex, int n) { return lua_setupvalue(L, funcindex, n); }
		//Returns an unique identifier for the upvalue numbered n from the closure at index funcindex. Parameters funcindex and n are as in the lua_getupvalue (see lua_getupvalue) (but n cannot be greater than the number of upvalues).
		//
		//These unique identifiers allow a program to check whether different closures share upvalues. Lua closures that share an upvalue (that is, that access a same external local variable) will return identical ids for those upvalue indices.
		inline void* upvalueid(int funcindex, int n) { return lua_upvalueid(L, funcindex, n); }
		//Make the n1-th upvalue of the Lua closure at index funcindex1 refer to the n2-th upvalue of the Lua closure at index funcindex2.
		inline void upvaluejoin(int funcindex1, int n1, int funcindex2, int n2) { lua_upvaluejoin(L, funcindex1, n1, funcindex2, n2); }
		
		/*
		*	Auxillary
		*/
		
		//Adds the byte c to the buffer B (see luaL_Buffer).
		static inline void l_addchar(l_Buffer* B, char c) { luaL_addchar(B, c); }
		//Adds the string pointed to by s with length l to the buffer B (see luaL_Buffer). The string can contain embedded zeros.
		static inline void l_addlstring(l_Buffer* B, const char* s, size_t l) { luaL_addlstring(B, s, l); }
		//Adds to the buffer B (see luaL_Buffer) a string of length n previously copied to the buffer area (see luaL_prepbuffer).
		static inline void l_addsize(l_Buffer* B, size_t n) { luaL_addsize(B, n); }
		//Adds the zero-terminated string pointed to by s to the buffer B (see luaL_Buffer). The string cannot contain embedded zeros.
		static inline void l_addstring(l_Buffer* B, const char* s) { luaL_addstring(B, s); }
		//Adds the value at the top of the stack to the buffer B (see luaL_Buffer). Pops the value.
		//
		//This is the only function on string buffers that can (and must) be called with an extra element on the stack, which is the value to be added to the buffer.
		static inline void l_addvalue(l_Buffer* B) { luaL_addvalue(B); }
		//Checks whether cond is true. If not, raises an error with a standard message.
		inline void l_argcheck(int cond, int arg, const char* extramsg) { luaL_argcheck(L, cond, arg, extramsg); }
		//Raises an error with a standard message that includes extramsg as a comment.
		//
		//This function never returns, but it is an idiom to use it in C functions as return luaL_argerror(args).
		inline int l_argerror(int arg, const char* extramsg) { return luaL_argerror(L, arg, extramsg); }
		//Initializes a buffer B. This function does not allocate any space; the buffer must be declared as a variable (see luaL_Buffer).
		inline void l_buffinit(l_Buffer* B) { luaL_buffinit(L, B); }
		//Calls a metamethod.
		//
		//If the object at index obj has a metatable and this metatable has a field e, this function calls this field passing the object as its only argument. In this case this function returns true and pushes onto the stack the value returned by the call. If there is no metatable or no metamethod, this function returns false (without pushing any value on the stack).
		inline int l_callmeta(int obj, const char* e) { return luaL_callmeta(L, obj, e); }
		//Checks whether the function has an argument of any type (including nil) at position arg.
		inline void l_checkany(int arg) { luaL_checkany(L, arg); }
		//Checks whether the function argument arg is a number and returns this number cast to an int.
		inline int l_checkint(int arg) { return luaL_checkint(L, arg); }
		//Checks whether the function argument arg is a number and returns this number cast to a lua_Integer.
		inline Integer l_checkinteger(int arg) { return luaL_checkinteger(L, arg); }
		//Checks whether the function argument arg is a number and returns this number cast to a long.
		inline long l_checklong(int arg) { return luaL_checklong(L, arg); }
		//Checks whether the function argument arg is a string and returns this string; if l is not NULL fills *l with the string's length.
		//
		//This function uses lua_tolstring to get its result, so all conversions and caveats of that function apply here.
		inline const char* l_checklstring(int arg, size_t* l) { return luaL_checklstring(L, arg, l); }
		//Checks whether the function argument arg is a number and returns this number.
		inline Number l_checknumber(int arg) { return luaL_checknumber(L, arg); }
		//Checks whether the function argument arg is a string and searches for this string in the array lst (which must be NULL-terminated). Returns the index in the array where the string was found. Raises an error if the argument is not a string or if the string cannot be found.
		//
		//If def is not NULL, the function uses def as a default value when there is no argument arg or when this argument is nil.
		//
		//This is a useful function for mapping strings to C enums. (The usual convention in Lua libraries is to use strings instead of numbers to select options.)
		inline int l_checkoption(int arg, const char* def, const char* const lst[]) { return luaL_checkoption(L, arg, def, lst); }
		//Grows the stack size to top + sz elements, raising an error if the stack cannot grow to that size. msg is an additional text to go into the error message (or NULL for no additional text).
		inline void l_checkstack(int sz, const char* msg) { luaL_checkstack(L, sz, msg); }
		//Checks whether the function argument arg is a string and returns this string.
		//
		//This function uses lua_tolstring to get its result, so all conversions and caveats of that function apply here.
		inline const char* l_checkstring(int arg) { return luaL_checkstring(L, arg); }
		//Checks whether the function argument arg has type t. See lua_type for the encoding of types for t.
		inline void l_checktype(int arg, int t) { luaL_checktype(L, arg, t); }
		//Checks whether the function argument arg is a userdata of the type tname (see luaL_newmetatable) and returns the userdata address (see lua_touserdata).
		inline void* l_checkudata(int arg, const char* tname) { return luaL_checkudata(L, arg, tname); }
		//Loads and runs the given file. It is defined as the following macro:
		//
		//	 (luaL_loadfile(L, filename) || lua_pcall(L, 0, LUA_MULTRET, 0))
		//
		//It returns false if there are no errors or true in case of errors.
		inline int l_dofile(const char* filename) { return luaL_dofile(L, filename); }
		//Loads and runs the given string. It is defined as the following macro:
		//
		//	 (luaL_loadstring(L, str) || lua_pcall(L, 0, LUA_MULTRET, 0))
		//
		//It returns false if there are no errors or true in case of errors.
		inline int l_dostring(const char* str) { return luaL_dostring(L, str); }
		//Raises an error. The error message format is given by fmt plus any extra arguments, following the same rules of lua_pushfstring. It also adds at the beginning of the message the file name and the line number where the error occurred, if this information is available.
		//
		//This function never returns, but it is an idiom to use it in C functions as return luaL_error(args).
		inline int l_error(const char* fmt, ...) { va_list argptr; va_start(argptr,fmt); return luaL_error(L, fmt, argptr); }
		//This function produces the return values for process-related functions in the standard library (os.execute and io.close).
		inline int l_execresult(int stat) { return luaL_execresult(L, stat); }
		//This function produces the return values for file-related functions in the standard library (io.open, os.rename, file:seek, etc.).
		inline int l_fileresult(int stat, const char* fname) { return luaL_fileresult(L, stat, fname); }
		//Pushes onto the stack the field e from the metatable of the object at index obj. If the object does not have a metatable, or if the metatable does not have this field, returns false and pushes nothing.
		inline int l_getmetafield(int obj, const char* e) { return luaL_getmetafield(L, obj, e); }
		//Pushes onto the stack the metatable associated with name tname in the registry (see luaL_newmetatable).
		inline void l_getmetatable(const char* tname) { luaL_getmetatable(L, tname); }
		//Creates a copy of string s by replacing any occurrence of the string p with the string r. Pushes the resulting string on the stack and returns it.
		inline const char* l_gsub(const char* s, const char* p, const char* r) { return luaL_gsub(L, s, p, r); }
		//Equivalent to luaL_loadbufferx with mode equal to NULL.
		inline int l_loadbuffer(const char* buff, size_t sz, const char* name) { return luaL_loadbuffer(L, buff, sz, name); }
		//Loads a buffer as a Lua chunk. This function uses lua_load to load the chunk in the buffer pointed to by buff with size sz.
		//
		//This function returns the same results as lua_load. name is the chunk name, used for debug information and error messages. The string mode works as in function lua_load.
		inline int l_loadbufferx(const char* buff, size_t sz, const char* name, const char* mode) { return luaL_loadbufferx(L, buff, sz, name, mode); }
		//Equivalent to luaL_loadfilex with mode equal to NULL.
		inline int l_loadfile(const char* filename) { return luaL_loadfile(L, filename); }
		//Loads a file as a Lua chunk. This function uses lua_load to load the chunk in the file named filename. If filename is NULL, then it loads from the standard input. The first line in the file is ignored if it starts with a #.
		//
		//The string mode works as in function lua_load.
		//
		//This function returns the same results as lua_load, but it has an extra error code LUA_ERRFILE if it cannot open/read the file or the file has a wrong mode.
		//
		//As lua_load, this function only loads the chunk; it does not run it.
		inline int l_loadfilex(const char* filename, const char* mode) { return luaL_loadfilex(L, filename, mode); }
		//Loads a string as a Lua chunk. This function uses lua_load to load the chunk in the zero-terminated string s.
		//
		//This function returns the same results as lua_load.
		//
		//Also as lua_load, this function only loads the chunk; it does not run it.
		inline int l_loadstring(const char* s) { return luaL_loadstring(L, s); }
		//If the registry already has the key tname, returns 0. Otherwise, creates a new table to be used as a metatable for userdata, adds it to the registry with key tname, and returns 1.
		//
		//In both cases pushes onto the stack the final value associated with tname in the registry.
		inline int l_newmetatable(const char* tname) { return luaL_newmetatable(L, tname); }
		//Opens all standard Lua libraries into the given state.
		inline void l_openlibs() { luaL_openlibs(L); }
		//If the function argument arg is a number, returns this number cast to an int. If this argument is absent or is nil, returns d. Otherwise, raises an error.
		inline int l_optint(int arg, int d) { return luaL_optint(L, arg, d); }
		//If the function argument arg is a number, returns this number cast to a lua_Integer. If this argument is absent or is nil, returns d. Otherwise, raises an error.
		inline Integer l_optinteger(int arg, Integer d) { return luaL_optinteger(L, arg, d); }
		//If the function argument arg is a number, returns this number cast to a long. If this argument is absent or is nil, returns d. Otherwise, raises an error.
		inline long l_optlong(int arg, long d) { return luaL_optlong(L, arg, d); }
		//If the function argument arg is a string, returns this string. If this argument is absent or is nil, returns d. Otherwise, raises an error.
		//
		//If l is not NULL, fills the position *l with the result's length.
		inline const char* l_optlstring(int arg, const char* d, size_t* l) { return luaL_optlstring(L, arg, d, l); }
		//If the function argument arg is a number, returns this number. If this argument is absent or is nil, returns d. Otherwise, raises an error.
		inline Number l_optnumber(int arg, Number d) { return luaL_optnumber(L, arg, d); }
		//If the function argument arg is a string, returns this string. If this argument is absent or is nil, returns d. Otherwise, raises an error.
		inline const char* l_optstring(int arg, const char* d) { return luaL_optstring(L, arg, d); }
		//Equivalent to luaL_prepbuffsize with the predefined size LUAL_BUFFERSIZE.
		static inline char* l_prepbuffer(l_Buffer* B) { return luaL_prepbuffer(B); }
		//Finishes the use of buffer B leaving the final string on the top of the stack.
		static inline void l_pushresult(l_Buffer* B) { luaL_pushresult(B); }
		//Creates and returns a reference, in the table at index t, for the object at the top of the stack (and pops the object).
		//
		//A reference is a unique integer key. As long as you do not manually add integer keys into table t, luaL_ref ensures the uniqueness of the key it returns. You can retrieve an object referred by reference r by calling lua_rawgeti(L, t, r). Function luaL_unref frees a reference and its associated object.
		//
		//If the object at the top of the stack is nil, luaL_ref returns the constant LUA_REFNIL. The constant LUA_NOREF is guaranteed to be different from any reference returned by luaL_ref.
		inline int l_ref(int t) { return luaL_ref(L, t); }
		//Opens a library.
		//
		//When called with libname equal to NULL, it simply registers all functions in the list l(see luaL_Reg) into the table on the top of the stack.
		//
		//When called with a non - null libname, luaL_register creates a new table t, sets it as the value of the global variable libname, sets it as the value of package.loaded[libname], and registers on it all functions in the list l.If there is a table in package.loaded[libname] or in variable libname, reuses this table instead of creating a new one.
		//
		//In any case the function leaves the table on the top of the stack.
		inline void l_register(const char* libname, const l_Reg* l) { luaL_register(L, libname, l); }
		//Creates and pushes a traceback of the stack L1. If msg is not NULL it is appended at the beginning of the traceback. The level parameter tells at which level to start the traceback.
		inline void l_traceback (Lua& l2, const char *msg, int level) { luaL_traceback(L, l2.L, msg, level); }
		//Returns the name of the type of the value at the given index.
		inline const char* l_typename (int index) { return luaL_typename(L, index); }
		//Generates an error with a message like the following :
		//
		//	location : bad argument narg to 'func' (tname expected, got rt)
		//
		//where location is produced by luaL_where, func is the name of the current function, and rt is the type name of the actual argument.
		inline int l_typerror(int narg, const char* tname) { return luaL_typerror(L, narg, tname); }
		//Releases reference ref from the table at index t (see luaL_ref). The entry is removed from the table, so that the referred object can be collected. The reference ref is also freed to be used again.
		inline void	l_unref(int t, int ref) { luaL_unref(L, t, ref); }
		//Pushes onto the stack a string identifying the current position of the control at level lvl in the call stack. Typically this string has the following format:
		//
		//	chunkname:currentline:
		//
		//Level 0 is the running function, level 1 is the function that called the running function, etc.
		//
		//This function is used to build a prefix for error messages.
		inline void	l_where(int lvl) { luaL_where(L, lvl); }

		/*
		*	JIT
		*/

		//Modes
		//
		//	LUAJIT_MODE_ENGINE
		//	LUAJIT_MODE_FUNC
		//	LUAJIT_MODE_ALLFUNC
		//	LUAJIT_MODE_ALLSUBFUNC
		//
		//Flags
		//
		//	LUAJIT_MODE_OFF
		//	LUAJIT_MODE_ON
		//	LUAJIT_MODE_FLUSH
		int setmode(int index, int mode) { return luaJIT_setmode(L, index, mode); }
};
