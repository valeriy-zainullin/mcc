%{
    #include <cerrno>
    #include <climits>
    #include <cstdlib> // strtoll, strtoull
    #include <cstring> // strerror
    #include <string>
    #include <iostream>
    #include "driver.hh"
    #include "parser.hh"
%}

%option noyywrap nounput noinput batch debug

%option c++
%option yyclass="Scanner"

%{
  // Code definitions at the end of scanner.cpp

  // A number symbol corresponding to the value in S.
  yy::parser::symbol_type make_number(
    const std::string &s,
    const yy::parser::location_type& loc
  );

  // A std::string corresponding to string constant.
  //   For now, just deletes double quotes.
  yy::parser::symbol_type make_string(
    const std::string &s,
    const yy::parser::location_type& loc
  );

  void Scanner::UpdateLocation() {
    if (driver.location_debug) {
        std::cerr << "Action called " << driver.location << std::endl;
    }
    driver.location.columns(yyleng);
  }
%}


blank  [ \t\r]

/*
identifier:
	identifier-nondigit
	identifier identifier-nondigit
	identifier digit
identifier-nondigit:
	nondigit
	universal-character-name
	other implementation-defined characters (no in our case)
nondigit: one of
	_ a b c d e f g h i j k l m
	  n o p q r s t u v w x y z
	  A B C D E F G H I J K L M
	  N O P Q R S T U V W X Y Z
digit: one of
	0 1 2 3 4 5 6 7 8 9
*/
nondigit   [_a-zA-Z]
digit      [0-9]
identifier {nondigit}({nondigit}|{digit})*

/*
unsigned-suffix: one of
	u U
long-suffix: one of
	l L
long-long-suffix: one of
	ll LL
integer-suffix:
	unsigned-suffix long-suffix_opt       -> uint (u), ulong (ul)
	unsigned-suffix long-long-suffix      -> ulonglong (ull)
	long-suffix unsigned-suffix_opt       -> long (l), ulong (lu)
	long-long-suffix unsigned-suffix_opt  -> longlong (ll), ulonglong (llu)
*/
uint_suffix      u|U
long_suffix      l|L
longlong_suffix  ll|LL
ulong_suffix     {unsigned_suffix}{long_suffix}|{long_suffix}{unsigned_suffix}
ulonglong_suffix {unsigned_suffix}{long_suffix}{2}
integer_suffix   {uint_suffix}|{long_suffix}|{longlong_suffix}|{ulong_suffix}|{ulonglong_suffix}

/*
nonzero-digit: one of
	1 2 3 4 5 6 7 8 9
decimal-constant:
	nonzero-digit
	decimal-constant digit
*/
nonzero_digit [1-9]
decimal_constant {nonzero_digit}{digit}*

/*
octal-digit: one of
	0 1 2 3 4 5 6 7
octal-constant:
	0
	octal-constant octal-digit
*/
octal_digit [0-7]
octal_constant 0{octal_digit}*

/*
hexadecimal-digit: one of
	0 1 2 3 4 5 6 7 8 9
	a b c d e f
	A B C D E F
hexadecimal-prefix: one of
	0x 0X
hexadecimal-constant:
	hexadecimal-prefix hexadecimal-digit
	hexadecimal-constant hexadecimal-digit
*/
hexadecimal_digit [0-9a-fA-F]
hexadecimal_prefix 0[xX]
hexadecimal_constant {hexadecimal_prefix}{hexadecimal_digit}{hexadecimal_digit}*

/*
integer-constant:
	decimal-constant integer-suffix_opt
	octal-constant integer-suffix_opt
	hexadecimal-constant integer-suffix_opt
*/
/* Три разных правила, в зависимости от основания.
   Чтобы не if-ать его. Суффикс выделим просто:
   все символы с конца. Благо мы проверили, что
   он уже удовлетворяет шаблону.
*/
decimal_int_const     {decimal_constant}{integer_suffix}?
octal_int_const       {octal_constant}{integer_suffix}?
hexadecimal_int_const {hexadecimal_constant}{integer_suffix}?

/*
floating-suffix: one of
	f l F L
*/
float_suffix      f|F
longdouble_suffix l|L

/* TODO: implement comments. Just match them, log and throw away, accounting newlines inside of them. */

%{
  // Code run each time a pattern is matched.
  #define YY_USER_ACTION UpdateLocation();
%}

/*
punctuator: one of
	[ ] ( ) { } . ->
	++ -- & * + - ~ !
	/ % << >> < > <= >= == != ^ | && ||
	? : ; ...
	= *= /= %= += -= <<= >>= &= ^= |=
	, # ##
	<: :> <% %> %: %:%:
*/
// Punctuators after # seem to not be used. So we don't support them yet.
//   They won't be parsed anyway, if there are no rules in grammar that
//   would contain them.

/*
simple-escape-sequence: one of
	\' \" \? \\ \a \b \f \n \r \t \v
*/
simple_escape_sequence \\['"?\\abfnrtv]

/*
octal-escape-sequence:
	\ octal-digit
	\ octal-digit octal-digit
	\ octal-digit octal-digit octal-digit
*/
octal_escape_sequence \\{octal_digit}{1,3}

/*
hexadecimal-escape-sequence:
	\x hexadecimal-digit
	hexadecimal-escape-sequence hexadecimal-digit
*/
hexadecimal_escape_sequence \\x{hexadecimal_digit}+

/*
escape-sequence:
	simple-escape-sequence
	octal-escape-sequence
	hexadecimal-escape-sequence
	universal-character-name
Paragraph 6.4.2.1, semantics:
3. Each universal character name in an identifier shall designate a character whose encoding
     in ISO/IEC 10646 falls into one of the ranges specified in annex D.60) The initial
     character shall not be a universal character name designating a digit. An implementation
     may allow multibyte characters that are not part of the basic source character set to
     appear in identifiers; which characters and their correspondence to universal character
     names is implementation-defined.
*/
/* We don't support universal character names for now. */
escape_sequence {simple_escape_sequence}|{octal_escape_sequence}|{hexadecimal_escape_sequence}

/*
c-char:
	any member of the source character set except the single-quote ', backslash \, or new-line character
	escape-sequence
c-char-sequence:
	c-char
	c-char-sequence c-char
character-constant:
	' c-char-sequence '
	L' c-char-sequence '
*/
c_char [^'\\\n]|{escape_sequence}
char_const       \'{c_char}+\'
long_char_const L\'{c_char}+\'

/*
s-char:
	any member of the source character set except the double-quote ", backslash \, or new-line character
	escape-sequence
s-char-sequence:
	s-char
	s-char-sequence s-char
string-literal:
	" s-char-sequence_opt "
	L" s-char-sequence_opt "
*/
s_char [^"\\\n]|{escape_sequence}
s_char_sequence {s_char}+
str_literal       \"{s_char}+?\"
long_str_literal L\"{s_char}+?\"


%%


%{
  // A handy shortcut to the location held by the driver.
  yy::location& loc = driver.location;
  if (driver.location_debug) {
  // Code run each time yylex is called.
    std::cerr << "BEFORE " << loc << std::endl;
  }
  // loc.step();
  if (driver.location_debug) {
    std::cerr << "AFTER " <<  loc << std::endl;
  }
%}

{blank}+   {
    if (driver.location_debug) {
        std::cerr << "Blank matched" << std::endl;
    }
    // loc.step();
}

\n+ {
    if (driver.location_debug) {
        std::cerr << "EOL called" << std::endl;
    }
    loc.lines(yyleng);
    loc.step();
}

"["                    return yy::parser::make_LBRACKET    (loc);
"]"                    return yy::parser::make_RBRACKET    (loc);
"("                    return yy::parser::make_LPAREN      (loc);
")"                    return yy::parser::make_RPAREN      (loc);
"{"                    return yy::parser::make_LBRACE      (loc);
"}"                    return yy::parser::make_RBRACE      (loc);
"."                    return yy::parser::make_DOT         (loc);
"->"                   return yy::parser::make_ARROW       (loc);
"&"                    return yy::parser::make_BIN_AND     (loc);
"*"                    return yy::parser::make_STAR        (loc);
"+"                    return yy::parser::make_PLUS        (loc);
"-"                    return yy::parser::make_MINUS       (loc);
"~"                    return yy::parser::make_TILDE       (loc);
"!"                    return yy::parser::make_EXCLMARK    (loc);
"/"                    return yy::parser::make_SLASH       (loc);
"%"                    return yy::parser::make_PERCENT     (loc);
"<<"                   return yy::parser::make_SHL         (loc);
">>"                   return yy::parser::make_SHR         (loc);
"<"                    return yy::parser::make_LT          (loc);
">"                    return yy::parser::make_GT          (loc);
"<="                   return yy::parser::make_LEQ         (loc);
">="                   return yy::parser::make_GEQ         (loc);
"=="                   return yy::parser::make_EQ          (loc);
"!="                   return yy::parser::make_NEQ         (loc);
"^"                    return yy::parser::make_CARET       (loc);
"|"                    return yy::parser::make_BIN_OR      (loc);
"&&"                   return yy::parser::make_AND         (loc);
"||"                   return yy::parser::make_OR          (loc);
"?"                    return yy::parser::make_QSTN_MARK   (loc);
":"                    return yy::parser::make_COLON       (loc);
";"                    return yy::parser::make_SEMICOLON   (loc);
"..."                  return yy::parser::make_ELLIPSIS    (loc);
"="                    return yy::parser::make_EQ_SIGN     (loc);
"*="                   return yy::parser::make_STAR_EQ     (loc);
"/="                   return yy::parser::make_SLASH_EQ    (loc);
"%="                   return yy::parser::make_PERC_EQ     (loc);
"+="                   return yy::parser::make_PLUS_EQ     (loc);
"-="                   return yy::parser::make_MINUS_EQ    (loc);
"<<="                  return yy::parser::make_SHL_EQ      (loc);
">>="                  return yy::parser::make_SHR_EQ      (loc);
"&="                   return yy::parser::make_AND_EQ      (loc);
"^="                   return yy::parser::make_CARET_EQ    (loc);
"|="                   return yy::parser::make_OR_EQ       (loc);
","                    return yy::parser::make_COMMA       (loc);
"#"                    return yy::parser::make_NUMSIGN     (loc);

"auto"                  return yy::parser::make_AUTO       (loc);
"break"                 return yy::parser::make_BREAK      (loc);
"case"                  return yy::parser::make_CASE       (loc);
"char"                  return yy::parser::make_CHAR       (loc);
"const"                 return yy::parser::make_CONST      (loc);
"continue"              return yy::parser::make_CONTINUE   (loc);
"default"               return yy::parser::make_DEFAULT    (loc);
"do"                    return yy::parser::make_DO         (loc);
"double"                return yy::parser::make_DOUBLE     (loc);
"else"                  return yy::parser::make_ELSE       (loc);
"enum"                  return yy::parser::make_ENUM       (loc);
"extern"                return yy::parser::make_EXTERN     (loc);
"float"                 return yy::parser::make_FLOAT      (loc);
"for"                   return yy::parser::make_FOR        (loc);
"goto"                  return yy::parser::make_GOTO       (loc);
"if"                    return yy::parser::make_IF         (loc);
"inline"                return yy::parser::make_INLINE     (loc);
"int"                   return yy::parser::make_INT        (loc);
"long"                  return yy::parser::make_LONG       (loc);
"register"              return yy::parser::make_REGISTER   (loc);
"restrict"              return yy::parser::make_RESTRICT   (loc);
"return"                return yy::parser::make_RETURN     (loc);
"short"                 return yy::parser::make_SHORT      (loc);
"signed"                return yy::parser::make_SIGNED     (loc);
"sizeof"                return yy::parser::make_SIZEOF     (loc);
"static"                return yy::parser::make_STATIC     (loc);
"struct"                return yy::parser::make_STRUCT     (loc);
"switch"                return yy::parser::make_SWITCH     (loc);
"typedef"               return yy::parser::make_TYPEDEF    (loc);
"union"                 return yy::parser::make_UNION      (loc);
"unsigned"              return yy::parser::make_UNSIGNED   (loc);
"void"                  return yy::parser::make_VOID       (loc);
"volatile"              return yy::parser::make_VOLATILE   (loc);
"while"                 return yy::parser::make_WHILE      (loc);
"_Bool"                 return yy::parser::make_BOOL       (loc);
"_Complex"              return yy::parser::make_COMPLEX    (loc);
"_Imaginary"            return yy::parser::make_IMAGINARY  (loc);


{decimal_int_const}     return make_number(yytext, 10, loc);
{octal_int_const}       return make_number(yytext,  8, loc);
{hexadecimal_int_const} return make_number(yytext, 16, loc);

{char_const}            return make_char_const(yytext, loc);
{long_char_const}       return make_long_char_const(yytext, loc);

{str_literal}           return make_str_literal(yytext, loc);
{long_str_literal}      return make_long_str_literal(yytext, loc);

{identifier}            {
                            if (driver.location_debug) {
                                std::cerr << "ID found " << yytext << std::endl;
                            }
                            return make_identifier_or_name(yytext, loc);
                        }

.                       {
                            throw yy::parser::syntax_error(loc, "invalid character: " + std::string(yytext));
                        }

<<EOF>>                 return yy::parser::make_EOF(loc);
%%

yy::parser::symbol_type make_number(
  const std::string &str,
  int base,
  const yy::parser::location_type& loc
) {
  // Число не может быть пустым, правила чисел отвергают такое.
  assert(!str.empty());

  unsigned int long_level = 0;
  bool is_signed = true;

  for (size_t pos = str.size(); pos > 0; pos -= 1) {
    if (str[pos - 1] == 'l' || str[pos - 1] == 'L') {
      long_level += 1;

      // long или long long, уж не более; правила должны не пропускать long long long.
      assert(long_level <= 2);
    } elif (str[pos - 1] == 'u' || str[pos - 1] == 'U') {
      assert(is_signed);
      is_signed = false;
    } else (std::isdigit(str[pos - 1])) {
      // Цифра? Ок.
      continue;
    } else {
      // Такого быть не должно.
      assert(false); __builtin_unreachable();
    }
  }

  // Пока типы 1-к-1 соответствуют host типам.
  // Host -- компьютер, на котором работает компилятор.
  // Target -- целевая архитектура (компьютер мб, тоже), для чего собирается.
  // Пока Host == Target. Кросс-компиляции нет.
  // Если типы не соответствуют, то можно сделать длинную арифметику на будущее,
  //   чтобы в целом было проще архитектуры добавлять.
  // Просто значения чисел могут быть нужны при оптимизациях.. Можно, например,
  //   сразу посчитать значение суммы, чтобы быстрее работать.
  // Хотя можно и поддержать все пары архитектур, для которых размеры типа у
  //   хоста не менее размеров типа у цели. Но это какие-то сравнения размеров
  //   типов будут.. Лучше длинку, наверно.

  if (is_signed) {
    errno = 0;
    long long num = std::strtoll(s.c_str(), NULL, base);
    if (
      ! (LLONG_MIN <= n && n <= LLONG_MAX && errno != ERANGE) ||
      (long_level == 0 && ! (  INT_MIN <= n && n <=  INT_MAX )) || 
      (long_level == 1 && ! ( LONG_MIN <= n && n <= LONG_MAX ))
    ) {
      throw yy::parser::syntax_error(loc, "signed integer is out of range: " + str);
    }

    return yy::parser::make_signed_number(num, long_level, loc);
  } else {
    errno = 0;
    unsigned long long num = std::strtoull(s.c_str(), NULL, base);
    if (
      ! (ULLONG_MIN <= n && n <= ULLONG_MAX && errno != ERANGE) ||
      (long_level == 0 && ! (  UINT_MIN <= n && n <=  UINT_MAX )) || 
      (long_level == 1 && ! ( ULONG_MIN <= n && n <= ULONG_MAX ))
    ) {
      throw yy::parser::syntax_error(loc, "unsigned integer is out of range: " + str);
    }

    return yy::parser::make_unsigned_number(num, long_level, loc);
  }
}

yy::parser::symbol_type make_char_const(
  const std::string &str,
  const yy::parser::location_type& loc
) {
  assert(str.size() >= 2); // Правила не пропустят строку без ковычек.

  std::string escaped_text(str.begin() + 1, str.end() - 1);
  return yy::parser::make_char_const(escaped_text, loc);
}

yy::parser::symbol_type make_long_char_const(
  const std::string &str,
  const yy::parser::location_type& loc
) {
  assert(str.front() == 'L');
  assert(str.size() >= 3); // Правила не пропустят строку без ковычек и L в начале.

  std::string escaped_text(str.begin() + 2, str.end() - 1);
  return yy::parser::make_long_char_const(escaped_text, loc);
}

yy::parser::symbol_type make_str_literal(
  const std::string &str,
  const yy::parser::location_type& loc
) {
  assert(str.size() >= 2); // Правила не пропустят строку без ковычек.

  std::string escaped_text(str.begin() + 1, str.end() - 1);
  return yy::parser::make_string(escaped_text, loc);
}

yy::parser::symbol_type make_long_str_literal(
  const std::string &str,
  const yy::parser::location_type& loc
) {
  assert(str.front() == 'L');
  assert(str.size() >= 3); // Правила не пропустят строку без ковычек и L в начале.

  std::string escaped_text(str.begin() + 2, str.end() - 1);
  return yy::parser::make_long_string(escaped_text, loc);
}

yy::parser::symbol_type make_identifier_or_name(
  const std::string &str,
  const yy::parser::location_type& loc
) {
  driver::DeclaredIdentType* identifier_type = driver.scope_tracker.find_item(str);

  if (identifier_type == nullptr) {
    return yy::parser::make_identifier(str, loc);
  }

  switch (*identifier_type) {
    case driver::DeclaredIdentType::VarName:    return yy::parser::make_var_name  (str, loc);
    case driver::DeclaredIdentType::TypeName:   return yy::parser::make_type_name  (str, loc);
    case driver::DeclaredIdentType::FuncName:   return yy::parser::make_func_name  (str, loc);
    case driver::DeclaredIdentType::StructName: return yy::parser::make_struct_name(str, loc);
    default: assert(false); __builtin_unreachable();
  }
}
