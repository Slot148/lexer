#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <ctype.h>
#include <pcre2posix.h>

#define CATCH_STATUS(status_code, message) catch_status(status_code, message, __FILE__, __LINE__, __func__)

typedef enum TokenType{
    TOK_WHITESPACE,         //MANTER SEMPRE PRIMEIRO(só para ser mais rápido, vai ser o primeiro verificado)
    TOK_KEYWORD,
    TOK_NUMBER, 
    TOK_ASSIGN, 
    TOK_IDENTIFIER,         //ADICIONAR OS DEMAIS ENTRE
    TOK_EOF                 //MANTER SEMPRE POR ULTIMO
}TokenType;
typedef struct Token* Token;
struct Token{
    TokenType type;
    char *lexeme;
    int line;
    int column;
};

typedef struct TokenPattern{
    TokenType type;
    const char *pattern;
    int should_keep;
    regex_t regex;
}TokenPattern;

static TokenPattern patterns[TOK_EOF] = {
    {TOK_WHITESPACE, "\\s+", 0},
    {TOK_KEYWORD,    "\\b(int)\\b", 1},
    {TOK_NUMBER,     "\\d+(\\.\\d+)?([eE][+-]?\\d+)?", 1},
    {TOK_ASSIGN,     "=", 1},
    {TOK_IDENTIFIER, "[a-zA-Z_][a-zA-Z0-9_]*", 1},
};

static const char* token_type_names[] = {
    [TOK_WHITESPACE] = "WHITESPACE",
    [TOK_KEYWORD] = "KEYWORD",
    [TOK_NUMBER] = "NUMBER",
    [TOK_ASSIGN] = "ASSIGN",
    [TOK_IDENTIFIER] = "IDENTIFIER",
    [TOK_EOF] = "EOF"
};

typedef struct Vector* Vector;
struct Vector{
    Token* tokens;
    size_t size, capacity;
};

typedef enum Status{OK, ERROR, WARNING}Status;

void catch_status(Status status_code, const char* message, const char* file, int line, const char* function){
    static const char* status[] = {
        [OK] = "OK", 
        [ERROR] = "ERROR",
        [WARNING] = "WARNING"
    };
    FILE* stream = (status_code == ERROR || status_code == WARNING) ? stderr : stdout;
    fprintf(stream, "(%s) %s [%s:%d in %s]\n", status[status_code], message, file, line, function);
    if(status_code == ERROR) exit(EXIT_FAILURE);
}

Vector new_vector(size_t capacity){
    const char* code = "int x = 10;";
    Vector this = malloc(sizeof(struct Vector));
    if(this == NULL) CATCH_STATUS(ERROR, "Erro ao alocar memória para o vetor");
    this->size = 0;
    this->capacity = capacity;
    this->tokens = malloc(sizeof(Token) * capacity);
    if(this->tokens == NULL)
    free(this), CATCH_STATUS(ERROR, "Erro ao alocar memória para os tokens");
    return this;
}

Status push_back(Vector this, Token token){
    if(this->size == this->capacity){
        this->capacity *= 2;
        this->tokens = realloc(this->tokens, this->capacity * sizeof(struct Token));
        if(this->tokens == NULL) CATCH_STATUS(ERROR, "Erro ao realocar memória para os tokens");
    }
    this->tokens[this->size++] = token; 
    return OK;
}

Token new_token(TokenType type, const char* lexeme, int line, int column){
    size_t len_lexeme = strlen(lexeme) + 1;
    Token this = malloc(sizeof(struct Token));
    if(this == NULL) CATCH_STATUS(ERROR, "Erro ao alocar memória para o token");
    this->line = line;
    this->column = column;
    this->lexeme = strdup(lexeme);
    this->type = type;
    return this;
}

Status destroy_token(Token token){
    free(token->lexeme);
    free(token);
    return OK;
}

Status destroy_vector(Vector this){
    for(size_t i = 0; i < this->size; i++){
        destroy_token(this->tokens[i]);
    }
    free(this->tokens);
    free(this);
    return OK;
}

Status compile_patterns(TokenPattern patterns[]){
    for(size_t i = 0; i < TOK_EOF; i++){
        int result = regcomp(&patterns[i].regex, patterns[i].pattern, REG_EXTENDED);
        if(result != 0){
            char error_buffer[100];
            regerror(result, &patterns[i].regex, error_buffer, sizeof(error_buffer));
            CATCH_STATUS(result, error_buffer);
            return ERROR;
        }
    }
    return OK;
}

Status free_patterns(TokenPattern patterns[]){
    for(size_t i = 0; i < TOK_EOF; i++){
        regfree(&patterns[i].regex);
    }
    return OK;
}

Vector tokenize(const char *code){
    Vector vector = new_vector(10);
    int line = 1, column = 1;
    const char *ptr = code;

    while(*ptr != '\0'){
        regmatch_t matches[1];
        bool match = false;
        for(int i = 0; i < TOK_EOF; i++){
            bool result = regexec(&patterns[i].regex, ptr, 1, matches, 0);
            if(result == 0 && matches[0].rm_so == 0){
                match = true;
                int start = matches[0].rm_so;
                int end = matches[0].rm_eo;
                int len = end - start;
                
                char *lexeme = malloc(len + 1);
                if(lexeme != NULL){
                    strncpy(lexeme, ptr + start, len);
                    lexeme[len] = '\0';
                }

                if(patterns[i].should_keep){
                    push_back(vector, new_token(patterns[i].type, lexeme, line, column));
                }else{
                    free(lexeme);
                }

                for(int j = 0; j < len; j++){
                    if(ptr[j] == '\n'){
                        line++;
                        column = 1;
                    }else{
                        column++;
                    }
                }

                ptr += end;
                break;
            }
        }

        if(!match){
            char error_msg[100];
            snprintf(error_msg, sizeof(error_msg), "Caractere não reconhecido: '%c' (linha %d, coluna %d)", *ptr, line, column);
            CATCH_STATUS(ERROR, error_msg);
            break;
        }

    }

    push_back(vector, new_token(TOK_EOF, "", line, column));
    return vector;
}

void print_tokens(Vector vector){
    printf("\n=== TOKENS ENCONTRADOS ===\n");
    for(size_t i = 0; i < vector->size; i++){
        Token token = vector->tokens[i];
        printf("Token[%zu]: %-12s '%-10s' (linha %d, coluna %d)\n", i, token_type_names[token->type], token->lexeme, token->line, token->column);
    }
}

int main(){
    const char *code = "int x = 10";
    
    printf("Compilando padrões regex...\n");
    
    if(compile_patterns(patterns) != OK){
        return EXIT_FAILURE;
    }
    
    printf("Tokenizando: '%s'\n", code);
    Vector vector = tokenize(code);
    
    print_tokens(vector);
    
    destroy_vector(vector);
    free_patterns(patterns);
    
    return 0;
}