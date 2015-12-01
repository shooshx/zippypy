// Copyright 2015 by Intigua, Inc.
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
#define name_op def_op
#define jrel_op def_op
#define jabs_op def_op
#define _def_const def_op

#define IMPL 1


def_op(STOP_CODE, 0, 0)
def_op(POP_TOP,   1, IMPL)
def_op(ROT_TWO,   2, IMPL)
def_op(ROT_THREE, 3, IMPL)
def_op(DUP_TOP,   4, IMPL)
def_op(ROT_FOUR,  5, IMPL)

def_op(NOP,            9,  0)
def_op(UNARY_POSITIVE, 10, IMPL)
def_op(UNARY_NEGATIVE, 11, IMPL)
def_op(UNARY_NOT,      12, IMPL)
def_op(UNARY_CONVERT,  13, 0)
def_op(UNARY_INVERT,   15, IMPL)

def_op(BINARY_POWER,         19, 0)
def_op(BINARY_MULTIPLY,      20, IMPL)
def_op(BINARY_DIVIDE,        21, IMPL)
def_op(BINARY_MODULO,        22, 0)
def_op(BINARY_ADD,           23, IMPL)
def_op(BINARY_SUBTRACT,      24, IMPL)
def_op(BINARY_SUBSCR,        25, IMPL)
def_op(BINARY_FLOOR_DIVIDE,  26, 0)
def_op(BINARY_TRUE_DIVIDE,   27, 0)
def_op(INPLACE_FLOOR_DIVIDE, 28, 0)
def_op(INPLACE_TRUE_DIVIDE,  29, 0)
def_op(SLICE_0, 30, IMPL)
def_op(SLICE_1, 31, IMPL)
def_op(SLICE_2, 32, IMPL)
def_op(SLICE_3, 33, IMPL)

def_op(STORE_SLICE_0, 40, 0)
def_op(STORE_SLICE_1, 41, 0)
def_op(STORE_SLICE_2, 42, 0)
def_op(STORE_SLICE_3, 43, 0)

def_op(DELETE_SLICE_0, 50, 0)
def_op(DELETE_SLICE_1, 51, 0)
def_op(DELETE_SLICE_2, 52, 0)
def_op(DELETE_SLICE_3, 53, 0)

def_op(STORE_MAP,        54, IMPL)
def_op(INPLACE_ADD,      55, IMPL)
def_op(INPLACE_SUBTRACT, 56, IMPL)
def_op(INPLACE_MULTIPLY, 57, IMPL)
def_op(INPLACE_DIVIDE,   58, IMPL)
def_op(INPLACE_MODULO,   59, 0)
def_op(STORE_SUBSCR,     60, IMPL)
def_op(DELETE_SUBSCR,    61, 0)
def_op(BINARY_LSHIFT,    62, IMPL)
def_op(BINARY_RSHIFT,    63, IMPL)
def_op(BINARY_AND,       64, IMPL)
def_op(BINARY_XOR,       65, IMPL)
def_op(BINARY_OR,        66, IMPL)
def_op(INPLACE_POWER,    67, 0)
def_op(GET_ITER,         68, IMPL)

def_op(PRINT_EXPR,       70, IMPL)
def_op(PRINT_ITEM,       71, IMPL)
def_op(PRINT_NEWLINE,    72, IMPL)
def_op(PRINT_ITEM_TO,    73, 0)
def_op(PRINT_NEWLINE_TO, 74, 0)

def_op(INPLACE_LSHIFT, 75, IMPL)
def_op(INPLACE_RSHIFT, 76, IMPL)
def_op(INPLACE_AND,    77, IMPL)
def_op(INPLACE_XOR,    78, IMPL)
def_op(INPLACE_OR,     79, IMPL)

def_op(BREAK_LOOP,   80, 0)
def_op(WITH_CLEANUP, 81, 0)
def_op(LOAD_LOCALS,  82, IMPL)
def_op(RETURN_VALUE, 83, IMPL)
def_op(IMPORT_STAR,  84, IMPL)
def_op(EXEC_STMT,    85, 0)
def_op(YIELD_VALUE,  86, IMPL)
def_op(POP_BLOCK,    87, IMPL)
def_op(END_FINALLY,  88, 0)
def_op(BUILD_CLASS,  89, IMPL)

_def_const(HAVE_ARGUMENT, 90, 0)              // Opcodes from here have an argument:

name_op(STORE_NAME,     90, IMPL)       // Index in name list
name_op(DELETE_NAME,    91, 0)      // ""
def_op(UNPACK_SEQUENCE, 92, IMPL)   // Number of tuple items
jrel_op(FOR_ITER,       93, IMPL)
def_op(LIST_APPEND,     94, IMPL)
name_op(STORE_ATTR,     95, IMPL)       // Index in name list
name_op(DELETE_ATTR,    96, 0)      // ""
name_op(STORE_GLOBAL,   97, IMPL)     // ""
name_op(DELETE_GLOBAL,  98, 0)    // ""
def_op(DUP_TOPX,        99, 0)          // number of items to duplicate
def_op(LOAD_CONST,     100, IMPL)       // Index in const list
// hasconst.append(100)
name_op(LOAD_NAME,     101, IMPL)       // Index in name list
def_op(BUILD_TUPLE,    102, IMPL)      // Number of tuple items
def_op(BUILD_LIST,     103, IMPL)       // Number of list items
def_op(BUILD_SET,      104, 0)        // Number of set items
def_op(BUILD_MAP,      105, IMPL)        // Number of dict entries (upto 255)
name_op(LOAD_ATTR,     106, IMPL)       // Index in name list
def_op(COMPARE_OP,     107, IMPL)       // Comparison operator
//  hascompare.append(107)

name_op(IMPORT_NAME,   108, IMPL)     // Index in name list
name_op(IMPORT_FROM,   109, 0)     // Index in name list

jrel_op(JUMP_FORWARD,         110, IMPL)    // Number of bytes to skip
jabs_op(JUMP_IF_FALSE_OR_POP, 111, IMPL) // Target byte offset from beginning of code
jabs_op(JUMP_IF_TRUE_OR_POP,  112, IMPL)  // ""
jabs_op(JUMP_ABSOLUTE,        113, IMPL)        // ""
jabs_op(POP_JUMP_IF_FALSE,    114, IMPL)    // ""
jabs_op(POP_JUMP_IF_TRUE,     115, IMPL)     // ""

name_op(LOAD_GLOBAL,          116, IMPL)     // Index in name list

jabs_op(CONTINUE_LOOP, 119, 0)   // Target address
jrel_op(SETUP_LOOP,    120, IMPL)      // Distance to target address
jrel_op(SETUP_EXCEPT,  121, 0)    // ""
jrel_op(SETUP_FINALLY, 122, 0)   // ""

def_op(LOAD_FAST,      124, IMPL)        // Local variable number
//    haslocal.append(124)
def_op(STORE_FAST,     125, IMPL)       // Local variable number
//    haslocal.append(125)
def_op(DELETE_FAST,    126, 0)      // Local variable number
//    haslocal.append(126)

def_op(RAISE_VARARGS, 130, IMPL)    // Number of raise arguments (1, 2, or 3)
def_op(CALL_FUNCTION, 131, IMPL)    // //args + (//kwargs << 8)
def_op(MAKE_FUNCTION, 132, IMPL)    // Number of args with default values
def_op(BUILD_SLICE,   133, IMPL)      // Number of items
def_op(MAKE_CLOSURE,  134, 0)
def_op(LOAD_CLOSURE,  135, 0)
//    hasfree.append(135)
def_op(LOAD_DEREF,    136, 0)
//    hasfree.append(136)
def_op(STORE_DEREF,   137, 0)
//    hasfree.append(137)

def_op(CALL_FUNCTION_VAR,    140, 0)     // //args + (//kwargs << 8)
def_op(CALL_FUNCTION_KW,     141, 0)      // //args + (//kwargs << 8)
def_op(CALL_FUNCTION_VAR_KW, 142, 0)  // //args + (//kwargs << 8)

jrel_op(SETUP_WITH,  143, 0)

def_op(EXTENDED_ARG, 145, 0)
//   EXTENDED_ARG = 145
def_op(SET_ADD, 146, 0)
def_op(MAP_ADD, 147, 0)



#undef name_op
#undef jrel_op
#undef jabs_op
#undef _def_const