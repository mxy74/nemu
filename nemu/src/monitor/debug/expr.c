#include "nemu.h"

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <sys/types.h>
#include <regex.h>

enum {
	NOTYPE = 256, EQ = 257, LBRA = 258,RBRA = 259,MUL = 260,DIV = 261,PLUS = 262, SUB = 263,NEQ = 264, AND = 265, OR = 266, NOT = 267, NUM_16 = 268, NUM_10 = 269,REG = 270,MARK = 271,NEG,DEREF 

	/* TODO: Add more token types */

};

static struct rule {
	char *regex;
	int token_type;
} rules[] = {

	/* TODO: Add more rules.
	 * Pay attention to the precedence level of different rules.
	 */

	{" +",	NOTYPE},				// spaces
	{"\\+", PLUS},					// plus
	{"==", EQ},						// equal
        {"\\(" , LBRA},
        {"\\)", RBRA},
        {"\\*",MUL},
        {"/", DIV},
        {"-", SUB},
        {"!=", NEQ},
        {"&&",AND},
        {"\\|\\|", OR},
        {"!", NOT},
        {"\\b0[xX][0-9a-fA-F]+\\b",NUM_16},
        {"\\b[0-9]+\\b", NUM_10},
        {"\\$[a-zA-Z]+",REG},
        {"\\b[a-zA-Z_0-9]+", MARK},
        
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

	for(i = 0; i < NR_REGEX; i ++) {
		ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
		if(ret != 0) {
			regerror(ret, &re[i], error_msg, 128);
			Assert(ret == 0, "regex compilation failed: %s\n%s", error_msg, rules[i].regex);
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

	while(e[position] != '\0') {
		/* Try all rules one by one. */
		for(i = 0; i < NR_REGEX; i ++) {
			if(regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
				char *substr_start = e + position;
				int substr_len = pmatch.rm_eo;

				Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s", i, rules[i].regex, position, substr_len, substr_len, substr_start);
				position += substr_len;

				/* TODO: Now a new token is recognized with rules[i]. Add codes
				 * to record the token in the array `tokens'. For certain types
				 * of tokens, some extra actions should be performed.
				 */

				switch(rules[i].token_type) {
                                        case NUM_10 : tokens[nr_token].type=rules[i].token_type;
                                        if(substr_len>=32) printf("NUM_10 too large");
                                        else{
                                             int j;
                                             for (j = 0;j < 32;j ++)
                                             tokens[nr_token].str[j] = '\0';
                                             for (j =0;j <substr_len;j ++){
                                                 tokens[nr_token].str[j] = substr_start[j];
                                              }
                                             }
                                        break;
                                        case NUM_16: tokens[nr_token].type = rules[i].token_type;
                                        if (substr_len >=34) printf("NUM_16 too largee");
                                        else{
                                             int j;
                                             for( j = 0;j < 32;j ++)
                                                tokens[nr_token].str[j] ='\0';
                                             for( j = 0; j < substr_len -2;j ++){
                                                tokens[nr_token].str[j] = substr_start[j + 2];
                                               }
                                            }
                                        break;
                                        case REG:tokens[nr_token].type = rules[i].token_type;
                                            int j;
                                            for( j = 0;j < 32;j ++)
                                            tokens[nr_token].str[j] = '\0';
                                            for( j = 0; j < substr_len -1;j++)
                                            tokens[nr_token].str[j] = substr_start[j + 1];
                                            break;
                                        case NOTYPE: nr_token--;break;

                                             
					default:tokens[nr_token].type = rules[i].token_type;
                                     // panic("please implement me");
				}
                                nr_token++;

				break;
			}
		}

		if(i == NR_REGEX) {
			printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
			return false;
		}
	}

	return true; 
}

bool check_parentheses(int p, int q, bool *success){
     bool result = false;
     int judge[40]= {0, };
     int i;
     int n=0;
     for(i = p;i <= q; i ++){
          if(tokens[i].type == LBRA || tokens[i].type == RBRA){
             judge[n]= tokens[i].type;
             if(n > 0 &&(judge[n] == RBRA &&judge [n-1]==LBRA)){
                 judge[n - 1] =0;judge [n] =0;
                 n =n-2;
             }
             n++;
           }
      }
      if(judge[0] ==0){
           *success = true;
     }
     else *success = false;
     if(tokens[p].type == LBRA && tokens[q].type == RBRA && judge[0]==0)
     result = true;
     return result;
}
  

int dominant_operator(int p,int q){
   int op =-1;
   int i;
   int nr_b=0;
   int min_rank =4;
   for(i = q; i >= p; i --){
      if(tokens[i].type == RBRA) nr_b++;
      if(tokens[i].type == LBRA) nr_b--;
      if(nr_b == 0&& (tokens[i].type == MUL || tokens[i].type ==DIV) &&min_rank >3){
         op=i;
         min_rank=3;
      }
      if(nr_b == 0&& (tokens[i].type == PLUS || tokens[i].type == SUB) &&min_rank >2){
         op =i;
         min_rank = 2;
      }
      if(nr_b==0&&(tokens[i].type == NEQ||tokens[i].type == EQ || tokens[i].type == AND || tokens[i].type == OR)&&min_rank>1){
       op =i;
       min_rank =1;
      }
  }
  return op;
}

uint32_t eval(int p,int q,bool *success){
         if(*success == 0) return 0;
         if(p>q){
            *success =false;
            return 0;
         }
         if(p == q){
           int m;
           if(tokens[p].type == NUM_10){
               sscanf(tokens[p].str,"%d",&m);
               *success =true;
               return m;
           }
           if(tokens[p].type == NUM_16){
               sscanf(tokens[p].str,"%x",&m);
               *success =true;
               return m;
           }
           if(tokens[p].type == REG){
           int i;
           *success =true;
           const char* reg_32[8]={"eax","ecx","edx","ebx","esp","ebp","esi","edi"};
           const char* reg_16[8]={"ax","cx","dx","bx","sp","bp","si","di"};
           const char* reg_8[8]={"al","ah","cl","ch","dl","dh","bl","bh"};
                                                                                                                                     for(i=0;i<8;i++){
               if(strcmp(tokens[p].str,reg_32[i])==0){
                  m=cpu.gpr[i]._32;
                  break;
               }
               if(strcmp(tokens[p].str,reg_16[i])==0){
                  m=cpu.gpr[i]._16;
                  break;
               }
               if(strcmp(tokens[p].str,reg_8[i])==0){
                 m = cpu .gpr[i/2]._8[i%2];
                 break;
               }
            }
            if(strcmp(tokens[p].str,"eip")==0)
               m=cpu.eip;
               return m;
         }
         else{
              *success = false;
              return 0;
             }
      }
      else if(check_parentheses(p,q,success)==true){
               return eval(p+1,q-1,success);
       }
            else{
                 if((q-p)==1){
                    if(tokens[p].type == NEG) return 0-eval(p+1,q,success);
                    if(tokens[p].type == NOT) return !eval(p+1,q,success);
                    if(tokens[p].type == DEREF) return swaddr_read(eval(p+1,q,success),4);
                    else{
                        *success = false;
                        return 0;
                     }
                 }
              int op=dominant_operator(p,q);
              int val1 = eval(p,op-1,success);
              int val2 = eval(op-1,q,success);
              int op_type =tokens[op].type;
              switch(op_type){
                          case PLUS : return val1+val2;break;
                          case SUB: return val1-val2;break;
                          case MUL : return val1*val2;break;
                          case DIV : return val1/val2;break;
                          case AND : return val1&&val2;break;
                          case OR: return val1||val2;break;
                          case NEQ: return val1!=val2;break;
                          case EQ: return val1== val2;break;
                          default :assert(0);return 0;
              }
           }
} 

uint32_t expr(char *e, bool *success) {
	if(!make_token(e)) {
		*success = false;
		return 0;
	}

	/* TODO: Insert codes to evaluate the expression. */
	int i;
        for(i = 0;i < nr_token; i++){
            if((tokens[i].type == MUL|| tokens[i].type ==SUB)&&(i==0||tokens[i-1].type ==PLUS||tokens[i-1].type==SUB||tokens[i-1].type==MUL||tokens[i-1].type==DIV||tokens[i-1].type==LBRA)){
           if(tokens[i].type == MUL) tokens[i].type = DEREF;
           if(tokens[i].type ==SUB) tokens[i].type=NEG;
           }
        }
        uint32_t result =0;
        result = eval (0,nr_token-1,success);
 
    //       panic("please implement me");
	return result;
}

