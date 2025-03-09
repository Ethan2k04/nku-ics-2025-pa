#include "nemu.h"

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <sys/types.h>
#include <regex.h>

enum {
  TK_NOTYPE = 256, TK_EQ,

  /* TODO: Add more token types */
  TK_NEQ, TK_NUM, TK_HEX, TK_REG, TK_SYM, TK_NG, TK_NL, TK_AND, TK_OR, TK_DEREF, TK_NEG
};

static struct rule {
  char *regex;
  int token_type;
} rules[] = {

  /* TODO: Add more rules.
   * Pay attention to the precedence level of different rules.
   */

  {" +", TK_NOTYPE},                  // spaces
  {"\\+", '+'},                       // plus
  {"\\-", '-'},                       // minus
  {"\\*", '*'},                       // multiply
  {"/", '/'},                         // divide
  {"%", '%'},                         // mod
  {"==", TK_EQ},                      // equal
  {"!=", TK_NEQ},                     // not equal
  {"0[x,X][0-9a-fA-F]+", TK_HEX},     // hex
  {"[0-9]+", TK_NUM},                 // number
  {"\\$e[a,b,c,d]x", TK_REG},         // register (eax, ebx, ecx, edx)
  {"\\$e[s,b]p", TK_REG},             // register (esp, ebp)
  {"\\$e[d,s]i", TK_REG},             // register (edi, esi)
  {"[a-zA-Z_][a-zA-Z0-9_]*", TK_SYM}, // symbol
  {"<=", TK_NG},                      // not greater than
  {">=", TK_NL},                      // not less than
  {"<", '<'},                         // less than
  {">", '>'},                         // greater than
  {"&&", TK_AND},                     // and
  {"\\|\\|", TK_OR},                  // or
  {"!", '!'},                         // not
  {"\\^", '^'},                       // xor
  {"~", '~'},                         // neg
  {"\\(", '('},                       // left bracket
  {"\\)", ')'},                       // right bracket
  {"&&", TK_AND},                     // and
  {"\\|\\|", TK_OR},                  // or
  {"&", '&'},                         // get addr
};

#define NR_REGEX (sizeof(rules) / sizeof(rules[0]) )

static regex_t re[NR_REGEX];

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex() {
  int i;
  char error_msg[128];
  int ret;

  for (i = 0; i < NR_REGEX; i ++) {
    ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
    if (ret != 0) {
      regerror(ret, &re[i], error_msg, 128);
      panic("regex compilation failed: %s\n%s", error_msg, rules[i].regex);
    }
  }
}

typedef struct token {
  int type;
  char str[32];
} Token;

Token tokens[32];
int nr_token;

static bool make_token(char *e) {
  int position = 0;
  int i;
  regmatch_t pmatch;

  nr_token = 0;

  while (e[position] != '\0') {
    /* Try all rules one by one. */
    for (i = 0; i < NR_REGEX; i ++) {
      if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
        char *substr_start = e + position;
        int substr_len = pmatch.rm_eo;

        Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
            i, rules[i].regex, position, substr_len, substr_len, substr_start);
        position += substr_len;

        /* TODO: Now a new token is recognized with rules[i]. Add codes
         * to record the token in the array `tokens'. For certain types
         * of tokens, some extra actions should be performed.
         */

	int j;
        switch (rules[i].token_type) {
          case TK_NOTYPE:
            break;
          case TK_REG:
          case TK_HEX:
          case TK_NUM:
          case TK_SYM:
            for(j=0;j<substr_len;j++) {
	    	tokens[nr_token].str[j] = *(substr_start+j);
	    }
	    tokens[nr_token].str[j]='\0';
            Log("Token %d: Copied substring \"%s\" to tokens[%d].str (type: %d)",
                nr_token, tokens[nr_token].str, nr_token, rules[i].token_type);
          default:
            tokens[nr_token].type = rules[i].token_type;
            Log("Token %d: Set type to %d (no string copied)", nr_token, rules[i].token_type);
            nr_token++;
        }

        break;
      }
    }

    if (i == NR_REGEX) {
      printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
      return false;
    }
  }

  return true;
}

uint32_t expr(char *e, bool *success) {
  if (!make_token(e)) {
    *success = false;
    return 0;
  }
  
  /* TODO: Insert codes to evaluate the expression. */
  printf("expression parsing not implemented!");
  TODO();

  return 0;
}
