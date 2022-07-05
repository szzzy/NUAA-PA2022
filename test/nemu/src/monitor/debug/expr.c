#include "nemu.h"

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <sys/types.h>
#include <regex.h>

enum {
  TK_NOTYPE = 256,
  TK_EQ = 0,

  /* TODO: Add more token types */
  TK_NUM = 1,
  TK_HEX = 2,
  TK_REG = 3,
  TK_NOTEQ = 4,
  TK_OR = 5,
  TK_AND = 6,
  DEREF,
  NEG
};

static struct rule {
  char *regex;
  int token_type;
} rules[] = {

  /* TODO: Add more rules.
   * Pay attention to the precedence level of different rules.
   */

  {" +", TK_NOTYPE},    // spaces
  {"\\+", '+'},         // plus
  {"==", TK_EQ},         // equal
  {"\\-", '-'},			//-
  {"\\*", '*'},			//*
  {"\\/", '/'},			// /
  
  {"0x[0-9,a-f]+", TK_HEX}, 	//hex
  {"[0-9]+", TK_NUM},			//num
  {"\\$[a-z]{2,3}", TK_REG}, 	//register name
  
  {"\\(", '('},			// (
  {"\\)", ')'},			// )
  
  {"!=", TK_NOTEQ},
  {"\\|\\|", TK_OR},
  {"&&", TK_AND},
  {"!", '!'}
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
		 
		for(int j=0;j<32;j++){
			tokens[nr_token].str[j] = '\0';
		}

        switch (rules[i].token_type) {
		  case '+':
			tokens[nr_token].type = '+';
			nr_token++;
			break;
		  case '-':
			tokens[nr_token].type = '-';
			nr_token++;
			break;
		  case '*':
			tokens[nr_token].type = '*';
			nr_token++;
			break;
		  case '/':
			tokens[nr_token].type = '/';
			nr_token++;
			break;
		  case '(':
		    tokens[nr_token].type = '(';
			nr_token++;
			break;
		  case ')':
			tokens[nr_token].type = ')';
			nr_token++;
			break;
			
		  case 256:
			break;
		  case 0:
			tokens[nr_token].type = 0;
			strcpy(tokens[nr_token].str, "==");
			nr_token++;
			break;
		  case 1:
			tokens[nr_token].type = 1;
			strncpy(tokens[nr_token].str, substr_start, substr_len);
			nr_token++;
			break;
		  case 2:
			tokens[nr_token].type = 2;
			strncpy(tokens[nr_token].str, substr_start, substr_len);
			nr_token++;
			break;
		  case 3:
			tokens[nr_token].type = 3;
			strncpy(tokens[nr_token].str, substr_start, substr_len);
			nr_token++;
			break;
			
		  case 4:
			tokens[nr_token].type = 4;
			strcpy(tokens[nr_token].str, "!=");
			nr_token++;
			break;
		  case 5:
			tokens[nr_token].type = 5;
			strcpy(tokens[nr_token].str, "||");
			nr_token++;
			break;
		  case 6:
			tokens[nr_token].type = 6;
			strcpy(tokens[nr_token].str, "&&");
			nr_token++;
			break;
			
		  case '!':
			tokens[nr_token].type = '!';
			nr_token++;
			break;
		  default:
			assert(0);
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

bool check_parentheses(int p, int q){
	int count=0;
    if(tokens[p].type == '('){
		for(int i=p; i<=q; i++){
			if(tokens[i].type == '('){
				count++;
			}
			if(tokens[i].type == ')'){
				count--;
			}
			if(count == 0 && i!=q){
				return false;
			}
		}
		if(count == 0)
			return true;
		else
			return false;
	}
    return false;
}

int find_dominated_op(int p, int q){
	int op = -1;
	int count = 0;
	int priority = 999;
	for(int i=p; i<=q; i++){
		if(tokens[i].type == '('){
			count++;
		}
		if(tokens[i].type == ')'){
			count--;
		}
		
		if(count == 0){
			if(tokens[i].type == 5){	// ||
				if(priority >= 0){
					priority = 0;
					op = i;
				}
			}
			else if(tokens[i].type == 6){	// &&
				if(priority >= 1){
					priority = 1;
					op = i;
				}
			}
			else if(tokens[i].type == 0 || tokens[i].type == 4){	// == !=
				if(priority >= 2){
					priority = 2;
					op = i;
				}
			}
			else if(tokens[i].type == '+' || tokens[i].type == '-'){	//+ -
				if(priority >= 3){
					priority = 3;
					op = i;
				}
			}
			else if(tokens[i].type == '*' || tokens[i].type == '/'){	//* /
				if(priority >= 4){
					priority = 4;
					op = i;
				}
			}
			
		}
	}
	return op;
}

uint32_t eval(int p, int q) {
	int num;
	int op = -1;
	int val1,val2;
	vaddr_t address;
    if (p > q) {
        /* Bad expression */
		assert(0);
    }
    else if (p == q) {
        /* Single token.
        * For now this token should be a number.
        * Return the value of the number.
        */
		if(tokens[p].type == 1){
			sscanf(tokens[p].str, "%d", &num);
			return num;
		}
		else if(tokens[p].type == 2){
			sscanf(tokens[p].str, "%x", &num);
			return num;
		}
		else if(tokens[p].type == 3){
			if(strcmp(tokens[p].str, "$eax") == 0){
				return cpu.eax;
			}
			else if(strcmp(tokens[p].str, "$ecx") == 0){
				return cpu.ecx;
			}
			else if(strcmp(tokens[p].str, "$edx") == 0){
				return cpu.edx;
			}
			else if(strcmp(tokens[p].str, "$ebx") == 0){
				return cpu.ebx;
			}
			else if(strcmp(tokens[p].str, "$esp") == 0){
				return cpu.esp;
			}
			else if(strcmp(tokens[p].str, "$ebp") == 0){
				return cpu.ebp;
			}
			else if(strcmp(tokens[p].str, "$esi") == 0){
				return cpu.esi;
			}
			else if(strcmp(tokens[p].str, "$edi") == 0){
				return cpu.edi;
			}
			else if(strcmp(tokens[p].str, "$eip") == 0){
				return cpu.eip;
			}
			else{
				assert(0);
			}
		}
    }
    else if (check_parentheses(p, q) == true) {
        /* The expression is surrounded by a matched pair of parentheses.
        * If that is the case, just throw away the parentheses.
        */
        return eval(p + 1, q - 1);
    }
    else {
        op = find_dominated_op(p, q);
		
		if(op == -1){	//一个操作符
			if(tokens[p].type == '!'){
				sscanf(tokens[q].str, "%d", &num);
				return !num;
			}
			else if(tokens[p].type == NEG){
				sscanf(tokens[q].str, "%d", &num);
				return -num;
			}
			else if(tokens[p].type == DEREF){
				if(tokens[q].type == TK_REG){
					if(strcmp(tokens[q].str, "$eax") == 0){
						return vaddr_read(cpu.eax, 4);
					}
					else if(strcmp(tokens[q].str, "$ecx") == 0){
						return vaddr_read(cpu.ecx, 4);
					}
					else if(strcmp(tokens[q].str, "$edx") == 0){
						return vaddr_read(cpu.edx, 4);
					}
					else if(strcmp(tokens[q].str, "$ebx") == 0){
						return vaddr_read(cpu.ebx, 4);
					}
					else if(strcmp(tokens[q].str, "$esp") == 0){
						return vaddr_read(cpu.esp, 4);
					}
					else if(strcmp(tokens[q].str, "$ebp") == 0){
						return vaddr_read(cpu.ebp, 4);
					}
					else if(strcmp(tokens[q].str, "$esi") == 0){
						return vaddr_read(cpu.esi, 4);
					}
					else if(strcmp(tokens[q].str, "$edi") == 0){
						return vaddr_read(cpu.edi, 4);
					}
					else if(strcmp(tokens[q].str, "$eip") == 0){
						return vaddr_read(cpu.eip, 4);
					}
					else{
						//printf("%s\n", tokens[q].str);
						//printf("here\n");
						assert(0);
					}
				}
				else if(tokens[q].type == 1){
					sscanf(tokens[q].str, "%d", &address);
					return vaddr_read(address, 4);
				}
				else if(tokens[q].type == 2){
					sscanf(tokens[q].str, "%x", &address);
					return vaddr_read(address, 4);
				}
			}
		}
		
        val1 = eval(p, op - 1);
        val2 = eval(op + 1, q);
        switch (tokens[op].type) {
            case '+': return val1 + val2;
            case '-': return val1 - val2;/* ... */
            case '*': return val1 * val2;/* ... */
            case '/': return val1 / val2;/* ... */
			case 0:
				if(val1 == val2) return 1;
				else return 0;
			case 4:
				if(val1 != val2) return 1;
				else return 0;
			case 5:
				return val1 || val2;
			case 6:
				return val1 && val2;
            default: assert(0);
        }
    }
	return 0;
}


uint32_t expr(char *e, bool *success) {
  if (!make_token(e)) {
    *success = false;
    return 0;
  }
  //printf("%d\n", nr_token);
  /* TODO: Insert codes to evaluate the expression. */
/*  
  bool flag = check_parentheses(0, nr_token-1);
  if(flag == false) printf("false\n");
  else printf("true\n");
  
  int op = find_dominated_op(0, nr_token-1);
  printf("%d\n", op);
  if(op != -1)  printf("%c\n", tokens[op].type);
*/
  
  for (int i = 0; i < nr_token; i ++) {
    if (tokens[i].type == '*' && (i == 0 || tokens[i - 1].type == '(') ) {
        tokens[i].type = DEREF;
    }
	if (tokens[i].type == '-' && (i == 0 || tokens[i - 1].type == '(') ) {
        tokens[i].type = NEG;
    }
}
  
  uint32_t result;
  result = eval(0, nr_token-1);
  //printf("result is %d\n", result);
  return result;
}
