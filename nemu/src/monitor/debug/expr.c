#include "nemu.h"

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <sys/types.h>
#include <regex.h>

enum {
  TK_NOTYPE = 256, TK_EQ,

  /* TODO✔️: Add more token types */
  TK_NEQ, TK_NUM, TK_HEX, TK_REG, TK_NG, TK_NL, TK_AND, TK_OR, TK_DEREF, TK_NEG
};

static struct rule {
  char *regex;
  int token_type;
} rules[] = {

  /* TODO✔️: Add more rules.
   * Pay attention to the precedence level of different rules.
   */
  {" +", TK_NOTYPE},                  // spaces
  {"\\+", '+'},                       // plus
  {"\\-", '-'},                       // minus
  {"\\*", '*'},                       // multiply
  {"/", '/'},                         // divide
  {"==", TK_EQ},                      // equal
  {"!=", TK_NEQ},                     // not equal (matched first to avoid collisions with not)
  {"0[x,X][0-9a-fA-F]+", TK_HEX},     // hex (matched first to avoid collisions with number)
  {"[0-9]+", TK_NUM},                 // number
  {"\\$e[a,b,c,d]x", TK_REG},         // register (eax, ebx, ecx, edx)
  {"\\$e[s,b,i]p", TK_REG},           // register (esp, ebp, eip)
  {"\\$e[d,s]i", TK_REG},             // register (edi, esi)
  {"<=", TK_NG},                      // not greater than
  {">=", TK_NL},                      // not less than
  {"<", '<'},                         // less than
  {">", '>'},                         // greater than
  {"\\(", '('},                       // left bracket
  {"\\)", ')'},                       // right bracket
  {"\\^", '^'},                       // xor
  {"!", '!'},                         // not
  {"&&", TK_AND},                     // and
  {"\\|\\|", TK_OR},                  // or
};

#define NR_REGEX (sizeof(rules) / sizeof(rules[0]))

static regex_t re[NR_REGEX];

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex() {
  int i;
  char error_msg[128];
  int ret;
  for (i = 0; i < NR_REGEX; i++) {
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
    for (i = 0; i < NR_REGEX; i++) {
      if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
        char *substr_start = e + position;
        int substr_len = pmatch.rm_eo;

        // Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
        //     i, rules[i].regex, position, substr_len, substr_len, substr_start);
        position += substr_len;

        /* TODO✔️: Now a new token is recognized with rules[i]. Add codes
         * to record the token in the array `tokens'. For certain types
         * of tokens, some extra actions should be performed.
         */
	      int j;
        switch (rules[i].token_type) {
          case TK_NOTYPE: { break; }
          case TK_REG:
          case TK_HEX:
          case TK_NUM: {
            for(j = 0; j < substr_len; j++) {
              tokens[nr_token].str[j] = *(substr_start + j);
            }
	          tokens[nr_token].str[j]='\0';
          }
          default: {
            tokens[nr_token].type = rules[i].token_type;
            nr_token++;
          }
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

static struct Pair {
  int operand;
  int precedence;
} table[] = {
  {TK_OR, 1},
  {TK_AND, 2},
  {'^', 3},
  {'<', 4},
  {'>', 4},
  {TK_NG, 4},
  {TK_NL, 4},
  {TK_EQ, 5},
  {TK_NEQ, 5},
  {'+', 6},
  {'-', 6},
  {'*', 7},
  {'/', 7},
  {'!', 8},
  {TK_NEG, 8},
  {TK_DEREF, 8},
};

#define NR_TABLE (sizeof(table) / sizeof(table[0]))

bool check_parentheses(int p, int q, bool *valid) {
  bool flag = true;
  if (tokens[p].type != '(' || tokens[q].type != ')') {
      flag = false;
  }
  int left = 0;
  int right = 0;
  for (int i = p; i < q; i++){
      if (tokens[i].type == '(') { left++; }
      else if (tokens[i].type == ')') {
          if (left > 0) {
              left--;
              if (left == 0) { flag = false; }
          }
          else { right++; }
      }
  }
  if (tokens[q].type == ')') {
      if (left > 0) { left--; }
      else { right++; }
  }
  if (left + right == 0) { *valid = true; return flag; }
  else { *valid = false; return false; }
}

bool is_operand(int op_type) {
  return op_type != TK_NOTYPE && op_type != TK_NUM && op_type != TK_HEX
  && op_type != TK_REG && op_type != '(' && op_type != ')';
}

int find_dominant_op(int p, int q) {
  int i, op_index= -1, op_precedence = 0, level = -1;
  for (i = p; i <= q; i++) {
    if (tokens[i].type == '(') { level++; }
    else if (tokens[i].type == ')') { level--; }
    else if (level == -1 && is_operand(tokens[i].type) == true) {
      int j;
      for (j = 0; j < NR_TABLE; j++) {
        if (table[j].operand == tokens[i].type) {
          if (op_index == -1 || table[j].precedence >= op_precedence) {
            op_index = i;
            op_precedence = table[j].precedence;
          }
        }
      }
    }
  }
  return op_index;
}

uint32_t eval(int p, int q, bool *valid) {
  if (*valid == false) return 0;
  if (p > q) {
    /* Bad expression */
    *valid = false;
    return 0;
  }
  else if (p == q) {
    /* Single token. 
     * For now this token should be a number.
     * Return the value of the number.
     */
    if (tokens[p].type == TK_NUM) {
      uint32_t parsed_value = 0;
      sscanf(tokens[p].str, "%d", &parsed_value);
      return parsed_value;
    }
    else if (tokens[p].type == TK_HEX) {
      uint32_t parsed_value = 0;
      sscanf(tokens[p].str, "%x", &parsed_value);
      return parsed_value;
    }
    else if (tokens[p].type == TK_REG) {
      if (strcmp(tokens[p].str, "$eax") == 0) { return cpu.eax; }
      else if (strcmp(tokens[p].str, "$ebx") == 0) { return cpu.ebx; }
      else if (strcmp(tokens[p].str, "$ecx") == 0) { return cpu.ecx; }
      else if (strcmp(tokens[p].str, "$edx") == 0) { return cpu.edx; }
      else if (strcmp(tokens[p].str, "$esp") == 0) { return cpu.esp; }
      else if (strcmp(tokens[p].str, "$ebp") == 0) { return cpu.ebp; }
      else if (strcmp(tokens[p].str, "$esi") == 0) { return cpu.esi; }
      else if (strcmp(tokens[p].str, "$edi") == 0) { return cpu.edi; }
      else if (strcmp(tokens[p].str, "$eip") == 0) { return cpu.eip; }
      else { printf("No matching register name."); return 0; }
    }
    else { printf("No matching register.\n"); assert(0); }
    return 0;
  }
  else if (check_parentheses(p, q, valid) == true) {
    /* The expression is surrounded by a matched pair of parentheses.
     * If that is the case, just throw away the parentheses.
     */
    return eval(p + 1, q - 1, valid);
  }
  else if (*valid == true) {
    /* We should do more things here. */
    int op_index = find_dominant_op(p, q);
    if (op_index < 0) { 
      printf("No dominant operator found.\n");
      *valid = false;
      return 0;
    }
    int op_type = tokens[op_index].type;
    uint32_t val1, val2;
    if (op_type != TK_DEREF && op_type != TK_NEG && op_type != '!') {
      val1 = eval(p, op_index - 1, valid);
    }
    else { val1 = 0; }
    val2 = eval(op_index + 1, q, valid);
    switch (op_type) {
      case '+': { return val1 + val2; break; }
      case '-': { return val1 - val2; break; }
      case '*': { return val1 * val2; break; }
      case '/': { return val1 / val2; break; }
      case '!': { return !val2; break; }
      case '^': { return val1 ^ val2; break; }
      case '<': { return val1 < val2; break; }
      case '>': { return val1 > val2; break; }
      case TK_NG: { return val1 <= val2; break; }
      case TK_NL: { return val1 >= val2; break; }
      case TK_EQ: { return val1 == val2; break; }
      case TK_NEQ: { return val1 != val2; break; }
      case TK_NEG: { return -val2; break; }
      case TK_DEREF: { return (uint32_t) vaddr_read(val2, 4); break; }
      case TK_OR: { return val1 || val2; break; }
      case TK_AND: { return val1 && val2; break; }
      default: { printf("No matching dominant op_type.\n"); assert(0); }
    }
  }
  return 0;
}

uint32_t expr(char *e, bool *success) {
  if (!make_token(e)) {
    *success = false;
    return 0;
  }
  
  /* TODO✔️: Insert codes to evaluate the expression. */
  int i;
  for (i = 0; i < nr_token; i++) {
    if (tokens[i].type == '-' && (i == 0 || is_operand(tokens[i - 1].type))) {
      // printf("Negation token detected at index %d\n", i);
      tokens[i].type = TK_NEG;
    }
    if (tokens[i].type == '*' && (i == 0 || is_operand(tokens[i - 1].type))) {
      // printf("Dereference token detected at index %d\n", i);
      tokens[i].type = TK_DEREF;
    }
  }
  bool valid = true;
  uint32_t res = eval(0, nr_token - 1, &valid);
  if (valid == true) { *success = true; return res; } 
  else { *success = false; return 0; }
}
