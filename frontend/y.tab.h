/* A Bison parser, made by GNU Bison 3.8.2.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2021 Free Software Foundation,
   Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

#ifndef YY_YY_Y_TAB_H_INCLUDED
# define YY_YY_Y_TAB_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
#endif

/* Token kinds.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    YYEMPTY = -2,
    YYEOF = 0,                     /* "end of file"  */
    YYerror = 256,                 /* error  */
    YYUNDEF = 257,                 /* "invalid token"  */
    T_RIGHT_BRACE = 258,           /* T_RIGHT_BRACE  */
    T_LEFT_BRACE = 259,            /* T_LEFT_BRACE  */
    T_RIGHT_PAREN = 260,           /* T_RIGHT_PAREN  */
    T_LEFT_PAREN = 261,            /* T_LEFT_PAREN  */
    T_COMMA = 262,                 /* T_COMMA  */
    T_ASSIGN = 263,                /* T_ASSIGN  */
    T_ARROW = 264,                 /* T_ARROW  */
    T_OR = 265,                    /* T_OR  */
    T_AND = 266,                   /* T_AND  */
    T_LESS_EQ = 267,               /* T_LESS_EQ  */
    T_GREATER_EQ = 268,            /* T_GREATER_EQ  */
    T_EQUALS = 269,                /* T_EQUALS  */
    T_NEQUALS = 270,               /* T_NEQUALS  */
    T_IF = 271,                    /* T_IF  */
    T_ELSE = 272,                  /* T_ELSE  */
    T_RETURN = 273,                /* T_RETURN  */
    T_FUNC = 274,                  /* T_FUNC  */
    T_INT = 275,                   /* T_INT  */
    T_CHAR = 276,                  /* T_CHAR  */
    T_BOOL = 277,                  /* T_BOOL  */
    T_INT_TYPE = 278,              /* T_INT_TYPE  */
    T_BOOL_TYPE = 279,             /* T_BOOL_TYPE  */
    T_CHAR_TYPE = 280,             /* T_CHAR_TYPE  */
    T_VOID_TYPE = 281,             /* T_VOID_TYPE  */
    T_IDENTIFIER = 282             /* T_IDENTIFIER  */
  };
  typedef enum yytokentype yytoken_kind_t;
#endif
/* Token kinds.  */
#define YYEMPTY -2
#define YYEOF 0
#define YYerror 256
#define YYUNDEF 257
#define T_RIGHT_BRACE 258
#define T_LEFT_BRACE 259
#define T_RIGHT_PAREN 260
#define T_LEFT_PAREN 261
#define T_COMMA 262
#define T_ASSIGN 263
#define T_ARROW 264
#define T_OR 265
#define T_AND 266
#define T_LESS_EQ 267
#define T_GREATER_EQ 268
#define T_EQUALS 269
#define T_NEQUALS 270
#define T_IF 271
#define T_ELSE 272
#define T_RETURN 273
#define T_FUNC 274
#define T_INT 275
#define T_CHAR 276
#define T_BOOL 277
#define T_INT_TYPE 278
#define T_BOOL_TYPE 279
#define T_CHAR_TYPE 280
#define T_VOID_TYPE 281
#define T_IDENTIFIER 282

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
union YYSTYPE
{
#line 17 "grammar.y"

    double fval;
    int ival;
    char cval;
    char* sval;

#line 128 "y.tab.h"

};
typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;


int yyparse (void);


#endif /* !YY_YY_Y_TAB_H_INCLUDED  */
