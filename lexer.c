#include <ctype.h>
#include <stdlib.h>

#include "mycc.h"
#include "util.h"

static char buf[BUF_SIZE];

static enum KeywordType parse_keyword(char *word) {
  if (!strcmp(word, "return")) return KEYWORD_return;
  if (!strcmp(word, "int")) return KEYWORD_int;
  if (!strcmp(word, "if")) return KEYWORD_if;
  if (!strcmp(word, "else")) return KEYWORD_else;
  return KEYWORD_unknown;
};

void lex(TokenVector *tokens, char code[]) {
  /* TODO: code + code_counter to a vector */
  int code_counter = 0, word_counter = 0;
  while (code[code_counter]) {
    switch (code[code_counter]) {
      case '{':
      case '}':
      case '(':
      case ')':
      case ';':
      case '~':
      case ':':
      case '?':
        push_back_token(tokens, init_punctuation(code[code_counter]));
        code_counter++;
        break;
      case '+':
      case '-':
      case '*':
      case '/':
      case '%':
      case '^':
        if (code[code_counter + 1] == '=') {
          if (code[code_counter] == '+')
            push_back_token(tokens, init_punctuation(PUNCTUATION_add_equal));
          else if (code[code_counter] == '-')
            push_back_token(tokens, init_punctuation(PUNCTUATION_sub_equal));
          else if (code[code_counter] == '*')
            push_back_token(tokens, init_punctuation(PUNCTUATION_mul_equal));
          else if (code[code_counter] == '/')
            push_back_token(tokens, init_punctuation(PUNCTUATION_div_equal));
          else if (code[code_counter] == '%')
            push_back_token(tokens, init_punctuation(PUNCTUATION_mod_equal));
          else if (code[code_counter] == '^')
            push_back_token(tokens,
                            init_punctuation(PUNCTUATION_bitwise_xor_equal));
          code_counter += 2;
        } else {
          push_back_token(tokens, init_punctuation(code[code_counter]));
          code_counter++;
        }
        break;
      case '!':
        if (code[code_counter + 1] == '=') {
          push_back_token(tokens, init_punctuation(PUNCTUATION_not_equal));
          code_counter++;
        } else {
          push_back_token(tokens, init_punctuation(code[code_counter]));
        }
        code_counter++;
        break;
      case '>':
      case '<':
        if (code[code_counter] == code[code_counter + 1] &&
            code[code_counter + 2] == '=') {
          /* >>= <<= */
          if (code[code_counter] == '>') {
            push_back_token(tokens,
                            init_punctuation(PUNCTUATION_shift_right_equal));
          } else if (code[code_counter] == '<') {
            push_back_token(tokens,
                            init_punctuation(PUNCTUATION_shift_left_equal));
          }
          code_counter += 3;
        } else if (code[code_counter] == code[code_counter + 1]) {
          /* >> << */
          if (code[code_counter] == '>') {
            push_back_token(tokens,
                            init_punctuation(PUNCTUATION_bitwise_shift_right));
          } else if (code[code_counter] == '<') {
            push_back_token(tokens,
                            init_punctuation(PUNCTUATION_bitwise_shift_left));
          }
          code_counter += 2;
        } else if (code[code_counter + 1] == '=') {
          /* >= <= */
          if (code[code_counter] == '>') {
            push_back_token(tokens,
                            init_punctuation(PUNCTUATION_greater_equal));
          } else if (code[code_counter] == '<') {
            push_back_token(tokens, init_punctuation(PUNCTUATION_less_equal));
          }
          code_counter += 2;
        } else {
          /* > < */
          push_back_token(tokens, init_punctuation(code[code_counter]));
          code_counter++;
        }
        break;
      case '=':
      case '&':
      case '|':
        if (code[code_counter + 1] == code[code_counter]) {
          if (code[code_counter] == '&')
            push_back_token(tokens, init_punctuation(PUNCTUATION_logical_and));
          else if (code[code_counter] == '|')
            push_back_token(tokens, init_punctuation(PUNCTUATION_logical_or));
          else if (code[code_counter] == '=')
            push_back_token(tokens, init_punctuation(PUNCTUATION_equal));
          code_counter += 2;
        } else if (code[code_counter + 1] == '=') {
          /* &= |= */
          if (code[code_counter] == '&') {
            push_back_token(tokens,
                            init_punctuation(PUNCTUATION_bitwise_and_equal));
          } else if (code[code_counter] == '|') {
            push_back_token(tokens,
                            init_punctuation(PUNCTUATION_bitwise_or_equal));
          }
          code_counter += 2;
        } else {
          push_back_token(tokens, init_punctuation(code[code_counter]));
          code_counter++;
        }
        break;
      case '\n':
      case ' ':
      case '\t':
      case '\r':
        code_counter++;
        break;
      default:
        word_counter = 0;
        if (isalpha(code[code_counter])) {
          do {
            buf[word_counter++] = code[code_counter++];
          } while (isalnum(code[code_counter]));
          buf[word_counter] = '\0';
          char *name = (char *)malloc((word_counter + 1) * sizeof(char));
          strcpy(name, buf);
          int keyword_type = parse_keyword(buf);
          if (keyword_type != KEYWORD_unknown) {
            push_back_token(tokens, init_keyword(keyword_type));
          } else {
            push_back_token(tokens, init_identifier(name));
          }
        } else if (isdigit(code[code_counter])) {
          do {
            buf[word_counter++] = code[code_counter++];
          } while (isdigit(code[code_counter]));
          buf[word_counter] = '\0';
          char *name = (char *)malloc((word_counter + 1) * sizeof(char));
          strcpy(name, buf);
          push_back_token(tokens, init_token(LITERAL, (uintptr_t)name));
        }
    }
  }
}