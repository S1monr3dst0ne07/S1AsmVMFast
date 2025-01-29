#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <ctype.h>
#include <string.h>




typedef uint16_t vint_t;


#define INT_LIMIT    (1 << (sizeof(vint_t)*8 - 1))
#define SOURCE_SIZE  (INT_LIMIT * 30)
#define MEM_SIZE     INT_LIMIT
#define STACK_SIZE   INT_LIMIT
#define PROG_SIZE    INT_LIMIT

#define LABEL_MAPPER_SIZE 2048


#define render_opcode_list(f) \
    f(set) \
    f(add) \
    f(sub) \
    f(shg) \
    f(shs) \
    f(lor) \
    f(and) \
    f(xor) \
    f(not) \
    f(lDA) \
    f(lDR) \
    f(sAD) \
    f(sRD) \
    f(lPA) \
    f(lPR) \
    f(sAP) \
    f(sRP) \
    f(out) \
    f(inp) \
    f(got) \
    f(jm0) \
    f(jmA) \
    f(jmG) \
    f(jmL) \
    f(jmS) \
    f(ret) \
    f(pha) \
    f(pla) \
    f(brk) \
    f(clr) \
    f(putstr) \
    f(ahm) \
    f(fhm) \

#define render_enum(x) x,
#define render_mapper(x) { x, #x }, 


enum opcode_t { render_opcode_list(render_enum) };
struct _opcode_mapper_entry {
    enum opcode_t operation;
    char* string;
} opcodeMapper[] = { render_opcode_list(render_mapper) };
const vint_t opcodeMapperSize = sizeof(opcodeMapper) / sizeof(struct _opcode_mapper_entry);
    


typedef struct 
{
    //parsed command
    char* operSource;
    char* attrSource;

    //resolved command
    enum opcode_t operation;
    vint_t        attribute;

    //error feedback
    vint_t sourceOriginLine;
} inst_t;

inst_t prog[PROG_SIZE] = { 0 };
vint_t progSize = 0;




typedef struct _chunk_node
{
    unsigned int ptr;
    unsigned int size;
    struct _chunk_node* next;
    struct _chunk_node* prev;
} chunk_node_t;


struct _label_map
{
    char* label;
    vint_t index;
} labelMapper[LABEL_MAPPER_SIZE] = { 0 };
vint_t labelMapperSize = 0;




#define panic0(message     ) { printf("Error: %s\n"  , message, arg); exit(1); }
#define panic1(message, arg) { printf("Error: %s%s\n", message, arg); exit(1); }
#define line_panic0(line, message     ) { printf("Error on line %d: %s\n"  , line, message, arg); exit(1); }
#define line_panic1(line, message, arg) { printf("Error on line %d: %s%s\n", line, message, arg); exit(1); }



char sourceBuffer[SOURCE_SIZE] = { 0 };
char* sourcePtr = sourceBuffer;

char  peekSource()  { return *sourcePtr;   }
void  nextSource()  {         sourcePtr++; }
char  popSource()   { return *sourcePtr++; }
char* indexSource() { return  sourcePtr;   }


bool isNumber(char*s )
{
    while (*s)
        if (!isdigit(*s++))
            return false;

    return true;
}




void loadSource(char* path)
{
    FILE* fd = fopen(path, "r");
    if (!fd)
        panic1("No such file: ", path);

    char  c;
    char* p = sourceBuffer;

    while ((c = fgetc(fd)) != EOF)
        *p++ = c;

    *p = '\0';
    fclose(fd);
}




void parse()
{
    enum parse_state_t
    {
        PARSE_INIT,
        PARSE_PRE,
        PARSE_INIT_OP,
        PARSE_OP,
        PARSE_FINAL_OP,
        PARSE_INIT_AT,
        PARSE_AT,
        PARSE_FINAL_AT,
        PARSE_FINAL,

        PARSE_COMMENT,
    } state = PARSE_PRE;

    char c;

    char* operation = NULL;
    char* attribute = NULL;

    vint_t line      = 0;

    while (c = peekSource())
    switch (state)
    {
        case PARSE_INIT:
            line++;
            state = PARSE_PRE;
            break;

        case PARSE_PRE:
            /**/ if (c == '"') state = PARSE_COMMENT;
            else if (c != ' ') state = PARSE_INIT_OP;
            else nextSource();
            break;

        case PARSE_INIT_OP:
            operation = indexSource();
            state = PARSE_OP;
            break;

        case PARSE_OP:
            if (c != ' ') nextSource();
            else state = PARSE_FINAL_OP;
            break;

        case PARSE_FINAL_OP:
            *indexSource() = '\0';
            nextSource();
            state = PARSE_INIT_AT;
            break;

        case PARSE_INIT_AT:
            attribute = indexSource();
            state = PARSE_AT;
            break;

        case PARSE_AT:
            if (c != '\n') nextSource();
            else state = PARSE_FINAL_AT;
            break;

        case PARSE_FINAL_AT:
            *indexSource() = '\0';
            nextSource();
            state = PARSE_FINAL;
            break;

        case PARSE_FINAL:
            if (!strcmp(operation, "lab"))
            {
                labelMapper[labelMapperSize].label = attribute;
                labelMapper[labelMapperSize].index = progSize;

                labelMapperSize++;
            }
            else
            {  
                prog[progSize].operSource = operation;
                prog[progSize].attrSource = attribute;
                prog[progSize].sourceOriginLine = line;

                progSize++;
            }

            state = PARSE_INIT;
            break;


        case PARSE_COMMENT:
            if (c == '\n') state = PARSE_INIT;
            nextSource();
            break;
    }
}


enum opcode_t operationLoopup(inst_t inst)
{
    for (int i = 0; i < opcodeMapperSize; i++)
        if (!strcmp(opcodeMapper[i].string, inst.operSource))
            return opcodeMapper[i].operation;

    line_panic1(inst.sourceOriginLine, "No such operation: ", inst.operSource); 
}

vint_t labelLookup(inst_t inst)
{
    for (int i = 0; i < labelMapperSize; i++)
        if (!strcmp(labelMapper[i].label, inst.attrSource))
            return labelMapper[i].index;
          
    line_panic1(inst.sourceOriginLine, "No such label: ", inst.attrSource); 
}




void resolve()
{
    for (int i = 0; i < progSize; i++)
    {
        inst_t* inst = &prog[i];

        inst->operation = operationLoopup(*inst);
        inst->attribute = isNumber(inst->attrSource)
            ? atoi(inst->attrSource)
            : labelLookup(*inst);
    }
}



int main(int argc, char** argv)
{
    char* path = argc > 1 ? argv[1] : "build.s1";
    loadSource(path);

    parse();
    resolve();

    
}

