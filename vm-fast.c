




typedef uint16_t vint_t;


#define INT_LIMIT  (1 << (sizeof(vint_t)*8 - 1))
#define PROG_SIZE  INT_LIMIT
#define MEM_SIZE   INT_LIMIT
#define STACK_SIZE INT_LIMIT

#define GEN_BUFFER_SIZE 2048



enum opcode_t
{
    invalid = 0,
    set,
    add,
    sub,
    shg,
    shs,
    lor,
    and,
    xor,
    not,
    lDA,
    lDR,
    sAD,
    sRD,
    lPA,
    lPR,
    sAP,
    sRP,
    out,
    inp,
    lab,
    got,
    jm0,
    jmA,
    jmG,
    jmL,
    jmS,
    ret,
    pha,
    pla,
    brk,
    clr,
    putstr,
    ahm,
    fhm,
};


typedef struct 
{
    //original command
    char instOriginal[GEN_BUFFER_SIZE];
    char attrOriginal[GEN_BUFFER_SIZE];

    //parsed command
    enum opcode_t operation;
    vint          attribute;

    //error feedback
    int sourceOriginLine;
} inst_t;




typedef struct _chunk_node
{
    unsigned int ptr;
    unsigned int size;
    struct _chunk_node* next;
    struct _chunk_node* prev;
} chunk_node_t;



#define panic0(message     ) { printf("Error: %s\n"  , message, arg); exit(1); }
#define panic1(message, arg) { printf("Error: %s%s\n", message, arg); exit(1); }



char* sourceProgBuffer[PROG_SIZE] = { 0 };



bool isNumber(char*s )
{
    while (*s)
        if (!isdigit(*s++))
            return false;

    return true;
}




void loadSourceProg(char* path)
{
    FILE* fd = fopen(path, "r");
    if (!fd)
        panic1("No such file: ", path);

    char  c;
    char* p = sourceProgBuffer;

    while ((c = fgetc(fd)) != EOF)
        *p++ = c;

    *p = '\0';
    fclose(fd);
}


int main(int argc, char** argv)
{
    char* path = argc > 1 ? argv[1] : "build.s1";
    loadSourceProg(path);


    printf("%s\n", sourceProgBuffer);
}

