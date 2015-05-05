#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#define MAXNUMTOKENS 1000
#define MAXTOKENSIZE 10
#define MAXVARNUM 150
#define STACKSIZE 200

typedef struct prog{
    char words[MAXNUMTOKENS][MAXTOKENSIZE];
    int current_word;
} Program;

typedef struct element{
    float value;
} Element;

typedef struct stack{
    Element stack[STACKSIZE];
    int top;
} Stack;

typedef struct var_array{
    int value;
    char letter;
} var_array;


int string_same(char *str1, char *str2);
void error (char *error_message);

void Main(Program *p);
void Instruction_list(Program *p);
void Instruction(Program *p);

void Varnum(Program *p);
void Number(Program *p);
void Var(Program *p);
void assign_var(var_array *v, char a, int n);

void Do(Program *p);
void Set(Program *p);

void Polish(Program *p, Stack *s);
void Op(Program *p);

int evaluate_expression(int a, int b, char *operand);
void Initialise_Stack(Stack *s);
void Push(Stack *s, int n);
int Pop(Stack *s);

int main(int argc, char *argv[])
{
    int i;
    char *prog_name = argv[1];
    FILE *fp;
    Program prog;
    prog.current_word = 0;

    for(int i = 0; i < MAXNUMTOKENS; ++i){
        prog.words[i][0] = '\0';
    }

    if(!(fp = fopen (prog_name, "r"))){
        printf("couldn't open file, %s\n", prog_name);
        exit (1);
    }

    i = 0;
    while(fscanf (fp, "%s", prog.words[i]) == 1 && i < MAXNUMTOKENS){
        i++;
    }

    Main(&prog);

    printf ("Parsed OK\n");
    return 0;
}

//returns a 1 if two strings entered are the same, 0 otherwise
int string_same(char *str1, char *str2){
    if(strcmp (str1, str2) == 0){
        return 1;
    }
    else{
        return 0;
    }
}

//prints a specific error message, shows line/file of error and exits program
void error(char *error_message){
    fprintf(stderr, "\n%s It happened on line %d in %s\n", error_message, __LINE__, __FILE__);
    exit(2);
}

void Main(Program *p){
    if(!string_same (p->words[p->current_word], "{")){
    error("Missing start brace?");
    }
    p->current_word++;
    Instruction_list(p);
}

void Instruction_list(Program *p){
    if (string_same(p->words[p->current_word], "}")){
        return;
    }
    Instruction(p);
    p->current_word++;
    Instruction_list(p);
}

void Instruction(Program *p){
    if(string_same(p->words[p->current_word], "FD")){
        p->current_word++;
        Varnum(p);
        return;
    }
    if(string_same(p->words[p->current_word], "LT")){
        p->current_word++;
        Varnum(p);
        return;
    }
    if(string_same(p->words[p->current_word], "RT")){
        p->current_word++;
        Varnum(p);
        return;
    }
    if(string_same(p->words[p->current_word], "DO")){
        p->current_word++;
        Do(p);
        return;
    }
    if(string_same(p->words[p->current_word], "SET")){
        p->current_word++;
        Set(p);
        return;
    }
    error("Invalid INSTRUCTION?");
}

void Varnum(Program *p){
    float varnum = 0;
    //if Varnum is an integer
    if(sscanf(p->words[p->current_word], "%f", &varnum) == 1){
        Number(p);
        return;
    }
    else{
        Var(p);
        return;
    }
}

void Number(Program *p){
    float varnum = 0;
    if(sscanf(p->words[p->current_word], "%f", &varnum) == 1 && varnum > 0 && varnum < MAXVARNUM){
        return;
    }
    Var(p);
    error("Invalid VARNUM?");
}

void Var(Program *p){
    char var = 0;
    //checks if Var is a letter from A to Z and if it's 1 character long
    if(sscanf(p->words[p->current_word], "%[A-Z]c", &var) == 1 && strlen(p->words[p->current_word]) < 2){
        return;
    }
    printf("current_word = %s\n", p->words[p->current_word]);
    error("Invalid VAR?");
}

void assign_var(var_array *v, char a, int n){
    
}


void Do(Program *p){
    Var(p);
    p->current_word++;
    if(string_same(p->words[p->current_word], "FROM")){
        p->current_word++;
        Varnum(p);
        p->current_word++;
        if(string_same(p->words[p->current_word], "TO")){
            p->current_word++;
            Varnum(p);
            p->current_word++;
            if(string_same(p->words[p->current_word], "{")){
                p->current_word++;
                Instruction_list(p);
                return;
            }
        }
    }
    error("Incorrect format for DO command?");
}

void Set(Program *p){
    Var(p);
    Stack s;
    Initialise_Stack(&s);
    p->current_word++;
    if(string_same(p->words[p->current_word], ":=")){
        p->current_word++;
        Polish(p, &s);
        return;
    }
    error("Invalid SET syntax?");
}

void Polish(Program *p, Stack *s){
    char instruction = 0;
    float a, b, result, variable_value;
    printf("s->top = %d\n", s->top);
    if(string_same(p->words[p->current_word], ";")){
        return;
    }
    //checks if next instruction is an operand
    printf("current_word = %s\n", p->words[p->current_word]);
    if(sscanf(p->words[p->current_word], "%[+,-,*,/]s", &instruction) == 1){
        Op(p);
        a = Pop(s);
        b = Pop(s);
        result = evaluate_expression(a, b, &instruction);
        printf("result = %f\n", result);
        Push(s, result);
    }
    else{
        Varnum(p);
        printf("current_word = %s\n", p->words[p->current_word]);
        //if Varnum is not a Variable (a letter)
        if(sscanf(p->words[p->current_word], "%[A-Z]c", &instruction) != 1){
            if(sscanf(p->words[p->current_word], "%f", &variable_value) != 1){
                printf("variable_value = %f\n", variable_value);
                error("fatal error. sscanf in Polish function.");
            }
            printf("variable_value %.2f pushed to stack\n", variable_value);
            Push(s, variable_value);
        }
    }
    p->current_word++;
    Polish(p, s);
}

void Op(Program *p){
    if(string_same(p->words[p->current_word], "+")){
        return;
    }
    if(string_same(p->words[p->current_word], "-")){
        return;
    }
    if(string_same(p->words[p->current_word], "*")){
        return;
    }
    if(string_same(p->words[p->current_word], "/")){
        return;
    }
    error("Invalid OPERATOR?");
}

int evaluate_expression(int a, int b, char *operand){
    int result = 0;
    if(string_same(operand, "+")){
        result = a + b;
    }
    if(string_same(operand, "-")){
        result = a - b;
    }
    if(string_same(operand, "*")){
        result = a * b;
    }
    if(string_same(operand, "/")){
        result = a / b;
    }
    return result;
}

void Initialise_Stack(Stack *s){
    s->top = 0;
}

void Push(Stack *s, int n){
    assert(s->top < STACKSIZE);
    s->stack[s->top].value = n;
    s->top = s->top + 1;
}

int Pop(Stack *s){
    assert(s->top > 0);
    s->top = s->top - 1;
    return s->stack[s->top].value;
}

