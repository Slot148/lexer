#include "main.h"

void catch_status(Status status_code, const char* message, const char* file, int line, const char* function){
    static const char* status[] = {
        [OK] = "OK", 
        [ERROR] = "ERROR",
        [WARNING] = "WARNING"
    };
    FILE* stream = (status_code == ERROR || status_code == WARNING) ? stderr : stdout;
    fprintf(stream, "(%s)-%s [%s:%d]\n", status[status_code], message, file, line);
    if (status_code == ERROR || status_code == WARNING) fprintf(stderr, "   Location: %s : %d in %s\n", file, line, function);
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
    this->lexeme = (char*)malloc(sizeof(char) * len_lexeme); 
    if(this->lexeme == NULL)
    free(this), CATCH_STATUS(ERROR, "Erro ao alocar memória para o lexema do token");
    strcpy(this->lexeme, lexeme);
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

Status compile_patterns(TokenPattern patterns[], int count){
    regex_t regex;
    for(size_t i = 0; i < count; i++){
        Status result = regcomp(&regex, patterns[i].pattern, REG_EXTENDED);
        if(result != OK){
            char error_buffer[100];
            regerror(result, &patterns[i].regex, error_buffer, sizeof(error_buffer));
            CATCH_STATUS(result, error_buffer);
        }
    }
}

Status free_patterns(TokenPattern patterns[], int count){
    for(size_t i = 0; i < count; i++){
        regfree(&patterns[i].regex);
    }
    return OK;
}

Vector tokenizer(const char* code){
    Vector vector = new_vector(100);
    size_t len_code = strlen(code);
    int column = 1, row = 1;
    for(size_t i = 0; i < len_code; i++){

    };
    return OK;
}

int main(){
    const char* code = "int x = 10;";
    Vector vector = tokenizer(code);
    regex_t regex;
    printf("%d\n", vector->size);
    push_back(vector, new_token(TOK_EOF, "EOF", 0, 0));
    printf("%d\n", vector->size);
    printf("%s\n", vector->tokens[0]->lexeme);
    return 0;
}