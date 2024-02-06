// Copyright (c) 2024 Epic Games Tools
// Licensed under the MIT license (https://opensource.org/license/mit/)

//- GENERATED CODE

#ifndef EVAL_META_H
#define EVAL_META_H

typedef U32 EVAL_ExprKind;
enum
{
EVAL_ExprKind_ArrayIndex,
EVAL_ExprKind_MemberAccess,
EVAL_ExprKind_Deref,
EVAL_ExprKind_Address,
EVAL_ExprKind_Cast,
EVAL_ExprKind_Sizeof,
EVAL_ExprKind_Neg,
EVAL_ExprKind_LogNot,
EVAL_ExprKind_BitNot,
EVAL_ExprKind_Mul,
EVAL_ExprKind_Div,
EVAL_ExprKind_Mod,
EVAL_ExprKind_Add,
EVAL_ExprKind_Sub,
EVAL_ExprKind_LShift,
EVAL_ExprKind_RShift,
EVAL_ExprKind_Less,
EVAL_ExprKind_LsEq,
EVAL_ExprKind_Grtr,
EVAL_ExprKind_GrEq,
EVAL_ExprKind_EqEq,
EVAL_ExprKind_NtEq,
EVAL_ExprKind_BitAnd,
EVAL_ExprKind_BitXor,
EVAL_ExprKind_BitOr,
EVAL_ExprKind_LogAnd,
EVAL_ExprKind_LogOr,
EVAL_ExprKind_Ternary,
EVAL_ExprKind_LeafBytecode,
EVAL_ExprKind_LeafMember,
EVAL_ExprKind_LeafU64,
EVAL_ExprKind_LeafF64,
EVAL_ExprKind_LeafF32,
EVAL_ExprKind_TypeIdent,
EVAL_ExprKind_Ptr,
EVAL_ExprKind_Array,
EVAL_ExprKind_Func,
EVAL_ExprKind_Define,
EVAL_ExprKind_LeafIdent,
EVAL_ExprKind_COUNT
};

U8 eval_expr_kind_child_counts[] =
{
2,
2,
1,
1,
2,
1,
1,
1,
1,
2,
2,
2,
2,
2,
2,
2,
2,
2,
2,
2,
2,
2,
2,
2,
2,
2,
2,
3,
0,
0,
0,
0,
0,
0,
1,
2,
1,
2,
0,
};

String8 eval_expr_kind_strings[] =
{
str8_lit_comp("ArrayIndex"),
str8_lit_comp("MemberAccess"),
str8_lit_comp("Deref"),
str8_lit_comp("Address"),
str8_lit_comp("Cast"),
str8_lit_comp("Sizeof"),
str8_lit_comp("Neg"),
str8_lit_comp("LogNot"),
str8_lit_comp("BitNot"),
str8_lit_comp("Mul"),
str8_lit_comp("Div"),
str8_lit_comp("Mod"),
str8_lit_comp("Add"),
str8_lit_comp("Sub"),
str8_lit_comp("LShift"),
str8_lit_comp("RShift"),
str8_lit_comp("Less"),
str8_lit_comp("LsEq"),
str8_lit_comp("Grtr"),
str8_lit_comp("GrEq"),
str8_lit_comp("EqEq"),
str8_lit_comp("NtEq"),
str8_lit_comp("BitAnd"),
str8_lit_comp("BitXor"),
str8_lit_comp("BitOr"),
str8_lit_comp("LogAnd"),
str8_lit_comp("LogOr"),
str8_lit_comp("Ternary"),
str8_lit_comp("LeafBytecode"),
str8_lit_comp("LeafMember"),
str8_lit_comp("LeafU64"),
str8_lit_comp("LeafF64"),
str8_lit_comp("LeafF32"),
str8_lit_comp("TypeIdent"),
str8_lit_comp("Ptr"),
str8_lit_comp("Array"),
str8_lit_comp("Func"),
str8_lit_comp("Define"),
str8_lit_comp("LeafIdent"),
};

String8 eval_expr_op_strings[] =
{
str8_lit_comp("[]"),
str8_lit_comp("."),
str8_lit_comp("*"),
str8_lit_comp("&"),
str8_lit_comp("cast"),
str8_lit_comp("sizeof"),
str8_lit_comp("-"),
str8_lit_comp("!"),
str8_lit_comp("~"),
str8_lit_comp("*"),
str8_lit_comp("/"),
str8_lit_comp("%"),
str8_lit_comp("+"),
str8_lit_comp("-"),
str8_lit_comp("<<"),
str8_lit_comp(">>"),
str8_lit_comp("<"),
str8_lit_comp("<="),
str8_lit_comp(">"),
str8_lit_comp(">="),
str8_lit_comp("=="),
str8_lit_comp("!="),
str8_lit_comp("&"),
str8_lit_comp("^"),
str8_lit_comp("|"),
str8_lit_comp("&&"),
str8_lit_comp("||"),
str8_lit_comp("? "),
str8_lit_comp("bytecode"),
str8_lit_comp("member"),
str8_lit_comp("U64"),
str8_lit_comp("F64"),
str8_lit_comp("F32"),
str8_lit_comp("type_ident"),
str8_lit_comp("ptr"),
str8_lit_comp("array"),
str8_lit_comp("function"),
str8_lit_comp("="),
str8_lit_comp("leaf_ident"),
};


#endif // EVAL_META_H
