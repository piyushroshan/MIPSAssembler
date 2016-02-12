/* prog.c 

   usage: prog <infile>

   This program will read in a TMIPS assembly file and
   process instructions from it. It makes a first pass
   reading in all the instructions and symbols into a
   linked list and puts all labels into a hash table.
   On a second pass it evaluates all the symbols and
   assembles the intructions, throwing errors as they
   arise. After this it will then either print a detailed
   error file or produce an object file in hexadecimal format.


*/

/* prog.h 
   this file contains the prototypes for the
   utility functions as well as many of the constants used
   throughout the program.
*/

/************* Includes **************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ctype.h>

/*************** constants ******************/

/*lengths of different arrays */
#define OPCODE_LEN 8
#define LABEL_LEN 32
#define INST_LEN 33
#define REG_LEN 6
#define IMMEDIATE_LEN 17
#define LINE_LEN 81
#define FILE_LEN 255

#define ERR_OPCODE 0       /* illegal opcode detected */
#define ERR_UNDEFSYMBOL 1  /* undefined symbol used   */
#define ERR_MULTSYMBOL 2   /* mutiply defined symbold */
/************** Constants *************/
#define HASH_SIZE 13

/* types of instructions */
#define RTYPE 0
#define ITYPE 1
#define JTYPE 2

#define DEBUG 1
#define REGBITS 6 /* register string length */


/* utility functions */


/* checks if a line is blank */
int isBlank(const char *line);

/* check for comment in line */
int commentExists(const char *line);

/* is line a comment ? */
int isComment(const char *line);

/* strips inline comment out of instruction */
void stripComment(char *line);

/* trim whitespace from string */
char *trimWhiteSpace(char *str);

/* return binary string for reg number */
char* regToBin(char *reg);

/* return a binary string for immediate number */
char* immToBin(char *imm);

/* return a binary string for immediate number */
char* subImmToBin(char *imm, int end, int start);

/* converts an integer in string format to a 32 bit binary string */
char* numTo32Bin(char *num);

/* returns a pointer to a new strign subset */
char* substring(const char* str, size_t begin, size_t len);

/* converts a 32 bit binary string to hex */
char* binToHex32(char *bin);

/* takes in an integer address and converts it to a 16 bit hex string */
char* addrToHex(int address, char *ret);

/****************** Constants **************/

/****************** Data Structures ********************/

/* this node will hold an error found
   while reading asm file */
typedef struct errnode_s
{
    /* variables */
    int errtype;             /* type of error */
    int lineno;              /* line number error was encountered on */

    char symbol[LABEL_LEN];  /* name of symbol */
    char opcode[OPCODE_LEN]; /* name of opcode */


    struct errnode_s *next; /* pointer to next node in list */

} errnode;

typedef struct errlist_s
{
    errnode *head;  /* pointer to the head of the list */
    errnode *cur;   /* node pointer used for traversal */

    int count;       /* number of instructions */

} errlist;


/*************** Functions **************************************/

/* add a node to list */
void add_err(errlist *list, errnode *node);

/* delete the list */
void delete_errlist(errlist *list);



/*************** Data structures *********************/

/* this node will hold a data entry
   from the data section of the asm file */
typedef struct datanode_s
{
    /* variables */
    int address;                  /* numeric address of instruction as integer */
    int lineno;                   /* line number the instruction was read from */
    char label[LABEL_LEN];        /* name of label if one exists */

    char binval[INST_LEN];      /* binary represenation of compiled instruction */
    char hex_val[INST_LEN];      /* hex representation of compiled instruction */

    struct datanode_s *next;

} datanode;

typedef struct datalist_s
{
    datanode *head;  /* pointer to the head of the list */
    datanode *cur;   /* node pointer used for traversal */

    int count;       /* number of instructions */

} datalist;


/*************** Functions **************************************/

/* add a node to list */
void add_datanode(datalist *list, datanode *node);

/* delete the list */
void delete_datalist(datalist* list);




/************* Includes ****************/

/**************Data structures ****************/

/* this node will hold an instruction read
from the assembly file to later be converted
into a hex code */
typedef struct instnode_s
{
    /* variables all instructions will utilize */
    int  address;                 /* numeric address of instruction as integer */
    int lineno;                   /* line number the instruction was read from */
    char label[LABEL_LEN];        /* name of label if one exists */
    int  inst_type;               /* type of instruction: R, I, J */
    char opcode_name[OPCODE_LEN]; /* name of the opcode */
    char opcode_bin[OPCODE_LEN];  /* binary string for opcode */
    char bin_inst[INST_LEN];      /* binary represenation of compiled instruction */
    char hex_inst[INST_LEN];      /* hex representation of compiled instruction */

    /* type dependent variables */
    char rs1[REG_LEN];       /* Register source 1 */
    char rs2[REG_LEN];       /* Register source 2 */
    char rt[REG_LEN];        /* Register target   */
    char sa[REG_LEN];        /* shift amount      */
    char imm[IMMEDIATE_LEN]; /* immediate address */
    char symbol[LABEL_LEN];  /* symbol */

    struct instnode_s *next; /* pointer to next node in list */

} instnode;

typedef struct instlist_s
{
    instnode *head;  /* pointer to the head of the list */
    instnode *cur;   /* node pointer used for traversal */

    int count;       /* number of instructions */

} instlist;


/*************** Functions **************************************/

/* add a node to list */
void add_node(instlist *list, instnode *node);

/* delete the list */
void delete_list(instlist* list);



typedef struct ListNode
{
  struct ListNode* next;   /* pointer to next node */
  char value[LINE_LEN];     /* value of entry */
  int address;             /* integer address to location */
} lnode;


typedef struct TableNode
{
    struct TableNode* next; //pointer to next node in table
    lnode*     head;        //pointer to first entry in entry list
    int key;                //string value held by node
    int entries;            //number of entries at this key
} tnode;
/*====================================================================*/

tnode* addhashnode(tnode* head, int key, const char* val, int address);
void deletetable(tnode* head);

/* returns true if hash key and value are found, false otherwise */
int checkHash(tnode* head, int key,const char* val, int *address);

/* generate hash and return key*/
int hashgen(char *s,  int T);



/*************** functions *****************/

/* takes in line, returns 0 or 1 if there
   is a comment in the line or not */
int commentExists(const char *line)
{
    /* return value */
    int ret = 0;

    /* check for # in line */
    if (strchr(line, '#'))
    {
        ret = 1;
    }

    return ret;
}

/* checks if a line is blank */
int isBlank(const char *line)
{
    int i = 0; /* iterator */

    while (i < strlen(line))
    {
        /* check for symbol other than whitespace */
        if (line[i] != ' ' && line[i] != '\n' && line[i] != '\t')
        {
            /* found a non whitespace symbol, return o */
            return 0;
        }
        /* increment iterator */
        i++;
    }
    /* found nothing but whitespace, return true */
    return 1;
}

/* takes in a line, returns 1 if
 the first non-whitespace character is a comment */
int isComment(const char *line)
{

    int i = 0;    /* iterator */

    /* loop through line, if we encounter something
       other than a tab or whitespace before a #, the line
       is not a comment, otherwise it is */
    while(i < strlen(line))
    {
        if (line[i] != '#')
        {
            if (line[i] != '\t' && line[i] != ' ')
            {
                /* first char was not a #, return false */
                return 0;
            }
        }
        else
        {
            /* first char other than whitespace was a #, return true */
            return 1;
        }
        i++;
    }
    return 0;
}

/*takes in a line, will strip out the comment behind it */
void stripComment(char *line)
{
    int i;   /* iterator */

    /* set iterator to last character  in line */
    i = strlen(line)-1;

    while (i >= 0)
    {
        if (line[i] == '#')
        {
            line[i] = '\0';
        }
        i--;
    }

}

/* takes in a char pointer, will trim the whitespace
  characters from the front and end of the string ,
  returning the new trimmed string */
char *trimWhiteSpace(char *str)
{
    char *end;

    /* trim leading space */
    while(isspace(*str))
    {
        str++;
    }
    if(*str == 0)
    {
        /* string was all spaces */
        return str;
    }

    /* trim trailing space */
    end = str + strlen(str) - 1;
    while(end > str && isspace(*end))
    {
        end--;
    }
    /* write new null terminator */
    *(end+1) = 0;

    /* return trimmed string */
    return str;
}

/* takes in a string holding a register in format $NUM
   and returns a string of 5 bit binary */
char* regToBin(char *reg)
{
    int dec = 0;  /* decimal number */
    char bin[REGBITS] = "00000";
    char integer[OPCODE_LEN];
    int i, j=0;

    /* strip otu the $ symbol */
    for(i=0; i<=strlen(reg); i++)
    {
        if (reg[i] != '$')
        {
            integer[j] = reg[i];
            j++;
        }
    }
    if(reg[0] == '$' && integer[0]=='t'){
        dec = atoi(integer+1);
        dec = dec + 8;
    }
    else if(reg[0] == '$' && integer[0]=='s'){
        dec = atoi(integer+1);
        dec = dec + 16;
    }else if(reg[0] == '$' && integer[0]=='0'){
        dec = 0;
    }
    if(DEBUG) printf("... Reg %s: %d\n",reg,dec);
    /* computer binary and fill the string */
    i = strlen(bin)-1;
    while (dec > 0)
    {
        bin[i] = (char)(((int)'0')+(dec % 2));
        dec /= 2;
        i--;
    }

    strcpy(reg, bin);

    /* return binary string */
    return reg;
}

/* return a binary string for immediate number */
char* immToBin(char *imm)
{
    int dec = 0;  /* decimal number */
    int negflag = 0;
    char bin[IMMEDIATE_LEN] = "0000000000000000";
    int i;
    int carry = 0;

    dec = atoi(imm);

    /* check if negative and set flag if so */
    if (dec < 0)
    {
        dec = -dec;
        negflag = 1;
    }

    /* computer binary and fill the string */
    i = strlen(bin)-1;
    while (dec > 0)
    {
        bin[i] = (char)(((int)'0')+(dec % 2));
        dec /= 2;
        i--;
    }

    /* convert to two's complement if negative */
    if (negflag)
    {
        /* negate the bits */
        for (i = 0; i<strlen(bin); i++)
        {
            if (bin[i] == '1')
            {
                bin[i] = '0';
            }
            else if (bin[i] == '0')
            {
                bin[i] = '1';
            }
        }
        /* add 1 */
        i = strlen(bin)-1;
        carry = 1;
        while (i > 0)
        {
            if (carry == 1)
            {
                if (bin[i] == '0')
                {
                    bin[i] = '1';
                    carry = 0;
                }
                else
                {
                    bin[i] = '0';
                    carry = 1;
                }
            }
            i--;
        }
    }
    strcpy(imm, bin);

    /* return binary string */
    return imm;
}

/* return a binary string for immediate number */
char* subImmToBin(char *imm, int end, int start)
{
    int dec = 0;  /* decimal number */
    int negflag = 0;
    char bin[INST_LEN] = "00000000000000000000000000000000";
    int i;
    int carry = 0;
    dec = atoi(imm);

    /* check if negative and set flag if so */
    if (dec < 0)
    {
        dec = -dec;
        negflag = 1;
    }

    /* computer binary and fill the string */
    i = strlen(bin)-1;
    while (dec > 0)
    {
        bin[i] = (char)(((int)'0')+(dec % 2));
        dec /= 2;
        i--;
    }

    /* convert to two's complement if negative */
    if (negflag)
    {
        /* negate the bits */
        for (i = 0; i<strlen(bin); i++)
        {
            if (bin[i] == '1')
            {
                bin[i] = '0';
            }
            else if (bin[i] == '0')
            {
                bin[i] = '1';
            }
        }
        /* add 1 */
        i = strlen(bin)-1;
        carry = 1;
        while (i > 0)
        {
            if (carry == 1)
            {
                if (bin[i] == '0')
                {
                    bin[i] = '1';
                    carry = 0;
                }
                else
                {
                    bin[i] = '0';
                    carry = 1;
                }
            }
            i--;
        }
    }
    strcpy(imm, "00000000000000000000000000000000");
    char nbin[IMMEDIATE_LEN] = "0000000000000000";
    for (i=start;i<=end;i++){
        int j=1;
        nbin[IMMEDIATE_LEN-j]=bin[i];
        j++;
    }
    strcpy(imm, nbin);
    /* return binary string */
    return imm;
}

/* takes in a string representing an integer and
   will convert it to a 32 bit binary string */
char* numTo32Bin(char *num)
{
    int dec = 0;  /* decimal number */
    int negflag = 0;
    char bin[INST_LEN] = "00000000000000000000000000000000";
    int i;
    int carry = 0;

    dec = atoi(num);

    /* set flag if negative */
    if (dec < 0)
    {
        dec = -dec;
        negflag = 1;
    }

    /* computer binary and fill the string */
    i = strlen(bin)-1;
    while (dec > 0)
    {
        bin[i] = (char)(((int)'0')+(dec % 2));
        dec /= 2;
        i--;
    }

    /* convert to two's complement if negative */
    if (negflag)
    {
        /* negate the bits */
        for (i = 0; i<strlen(bin); i++)
        {
            if (bin[i] == '1')
            {
                bin[i] = '0';
            }
            else if (bin[i] == '0')
            {
                bin[i] = '1';
            }
        }
        /* add 1 */
        i = strlen(bin)-1;
        carry = 1;
        while (i > 0)
        {
            if (carry == 1)
            {
                if (bin[i] == '0')
                {
                    bin[i] = '1';
                    carry = 0;
                }
                else
                {
                    bin[i] = '0';
                    carry = 1;
                }
            }
            i--;
        }
    }
    strcpy(num, bin);

    /* return 32 bit binary string */
    return num;
}

/* i had to write this function because ITSUNIX doesn't support
   it. takes in a string and a limiting size, returns the strlen
   of the string if its less than or equal to the limit, otherwise
   it returns the limit */
size_t strnlen(const char *s, size_t n)
{
    if (strlen(s) <= n)
    {
        return strlen(s);
    }
    else
    {
        return n;
    }
}

/* another function that ITSUNIX doesnt support. this function
   allocates a new string from a subset of an existing one,
   is used to facilitate the strsub below */
char* strndup (char const *s, size_t n)
{
    size_t len = strnlen (s, n);
    char *new = malloc (len + 1);

    if (new == NULL)
        return NULL;

    new[len] = '\0';
    return memcpy (new, s, len);
}

/* this function allows for taking a subset of a string,
   which is useful for the binary to hex conversions. it takes
   in a string, a start position and length to copy, and
   will copy a new string in that range and return the char
   pointer to it */
char* substring(const char* str, size_t begin, size_t len)
{
    /* check for problems with string size and position variables */
    if (str == 0 || strlen(str) == 0 || strlen(str) < begin || strlen(str) < (begin+len))
        return 0;

    /* return pointer to new string */
    return strndup(str + begin, len);
}

/* this function will take in a binary number and convert it to
   hexidecimal string of 32 bits */
char* binToHex32(char *bin)
{
    int i=0; /* iterator */
    char hex[INST_LEN];  /* hex string holder */
    char *sub;           /* string ptr for use with strsub */

    /* clear out hex string */
    strcpy(hex, "");

    /* loop through whole binary string, 32 bits */
    while (i < LABEL_LEN)
    {
        /* grab 4 digits of the binary */
        sub = substring(bin, i, 4);

        /* compare to dif binary strings and concatenate the correct hex
          digit to the hex string */
        if (strcmp(sub, "0000")==0)
        {
            strcat(hex, "0");
        }
        else if (strcmp(sub, "0001")==0)
        {
            strcat(hex, "1");
        }
        else if (strcmp(sub, "0010")==0)
        {
            strcat(hex, "2");
        }
        else if (strcmp(sub, "0011")==0)
        {
            strcat(hex, "3");
        }
        else if (strcmp(sub, "0100")==0)
        {
            strcat(hex, "4");
        }
        else if (strcmp(sub, "0101")==0)
        {
            strcat(hex, "5");
        }
        else if (strcmp(sub, "0110")==0)
        {
            strcat(hex, "6");
        }
        else if (strcmp(sub, "0111")==0)
        {
            strcat(hex, "7");
        }
        else if (strcmp(sub, "1000")==0)
        {
            strcat(hex, "8");
        }
        else if (strcmp(sub, "1001")==0)
        {
            strcat(hex, "9");
        }
        else if (strcmp(sub, "1010")==0)
        {
            strcat(hex, "A");
        }
        else if (strcmp(sub, "1011")==0)
        {
            strcat(hex, "B");
        }
        else if (strcmp(sub, "1100")==0)
        {
            strcat(hex, "C");
        }
        else if (strcmp(sub, "1101")==0)
        {
            strcat(hex, "D");
        }
        else if (strcmp(sub, "1110")==0)
        {
            strcat(hex, "E");
        }
        else if (strcmp(sub, "1111")==0)
        {
            strcat(hex, "F");
        }

        /* free memory used by strsub */
        free(sub);

        /* increment iterator to next block of binary digits */
        i += 4;
    }

    /* copy hex string to return ptr */
    strcpy(bin, hex);

    /* return new string */
    return bin;

}

/* this function is used to take an integer address
   and convert it into a 16 bit hex string */
char* addrToHex(int address, char *ret)
{
    char bin[IMMEDIATE_LEN] = "0000000000000000";
    int i;
    char hex[IMMEDIATE_LEN];
    char *sub;
    int dec = address;

    /* computer binary and fill the string */
    i = strlen(bin)-1;
    while (dec > 0)
    {
        bin[i] = (char)(((int)'0')+(dec % 2));
        dec /= 2;
        i--;
    }

    strcpy(hex, "");
    i = 0;

    /* loop through blocks of 4 binary digits and
       concatenate the correct hex symbol to the hex string */
    while (i < IMMEDIATE_LEN-1)
    {
        /* grab next block */
        sub = substring(bin, i, 4);
        if (strcmp(sub, "0000")==0)
        {
            strcat(hex, "0");
        }
        else if (strcmp(sub, "0001")==0)
        {
            strcat(hex, "1");
        }
        else if (strcmp(sub, "0010")==0)
        {
            strcat(hex, "2");
        }
        else if (strcmp(sub, "0011")==0)
        {
            strcat(hex, "3");
        }
        else if (strcmp(sub, "0100")==0)
        {
            strcat(hex, "4");
        }
        else if (strcmp(sub, "0101")==0)
        {
            strcat(hex, "5");
        }
        else if (strcmp(sub, "0110")==0)
        {
            strcat(hex, "6");
        }
        else if (strcmp(sub, "0111")==0)
        {
            strcat(hex, "7");
        }
        else if (strcmp(sub, "1000")==0)
        {
            strcat(hex, "8");
        }
        else if (strcmp(sub, "1001")==0)
        {
            strcat(hex, "9");
        }
        else if (strcmp(sub, "1010")==0)
        {
            strcat(hex, "A");
        }
        else if (strcmp(sub, "1011")==0)
        {
            strcat(hex, "B");
        }
        else if (strcmp(sub, "1100")==0)
        {
            strcat(hex, "C");
        }
        else if (strcmp(sub, "1101")==0)
        {
            strcat(hex, "D");
        }
        else if (strcmp(sub, "1110")==0)
        {
            strcat(hex, "E");
        }
        else if (strcmp(sub, "1111")==0)
        {
            strcat(hex, "F");
        }

        /* free memory used by strsub */
        free(sub);

        /* jump to next block of 4 binary digits */
        i += 4;
    }

    /* copy hex string to return ptr */
    strcpy(ret, hex);

    /* return new string */
    return ret;
}
/***** argument constants *****/
#define ARGS_NEEDED 2
#define ARG1 1
#define ARG2 2
#define DEBUG 1

/* main method */
int main(int argc, char **argv)
{
    /************* Variables **********************/
    char file[FILE_LEN];     /* string for file name */
    char errfile[FILE_LEN];  /* string for error file name */
    FILE* fp = NULL;         /* file pointer for asm file */
    FILE* errfp = NULL;      /* file pointer to error file */
    char line[LINE_LEN];     /* line to be read in from asm file */

    char label[LABEL_LEN];       /* used to hold label */
    char* temp;                  /* used for splitting strings */
    char opname[OPCODE_LEN];     /* holds opcode name */
    char instargs[LINE_LEN];     /* holds instruction arguments */
    char directive[LABEL_LEN];   /* holds data directive */
    char immarg[IMMEDIATE_LEN];  /* holds immediate argument */
    char regarg[OPCODE_LEN];     /* holds register argument */
    /* used for splitting instruction args into sep args */
    char arg1[LINE_LEN];
    char arg2[LINE_LEN];
    char arg3[LINE_LEN];


    /* file read flags */
    int found_text = 0;     /* has .text been encountered */
    int found_data = 0;     /* has .data been found yet */
    int was_error = 0;      /* was the opcode an error? */

    int counter = 0;        /* line counter */
    int address = 0;        /* address counter */
    int addr = 0;           /* address holder */
    int i;                  /* iterator */


    /* list stuff */
    instlist *instructions;  /* instruction list */
    instnode *tempinst;      /* holder pointer   */

    errlist *errors;         /* error list */
    errnode *temperr;        /* temporary error node pointer */

    datalist *data;          /* data list */
    datanode *tempdata;      /* temporary data node */

    int tSize  = HASH_SIZE; /* size of table, read from first line of input */
    int genkey = 0;         /* key generated by hash function  */

    tnode* hash_head = NULL;    /* head node of the hash table */


    /************* BEGIN main executables *********/

    /* check if we have correct arguments */
    if (argc != ARGS_NEEDED)
    {
        fprintf(stderr, "Invalid arguments provided.\n");
        exit(1);
    }

    /* copy argument to file name */
    strcpy(file, argv[ARG1]);


    /****** begin to process asm file ******/

    /* attempt to open asm file */
    if ((fp = fopen(file, "r")) == NULL)
    {
        fprintf(stderr, "Error opening asm file: %s\n", file);
        exit(1);
    }


    /* OK we will attempt to do this, allocate list stuff
       then start reading file */

    /* allocate instructions list */
    instructions = malloc(sizeof(instlist));

    /* initialize instructions variables */
    instructions->head  = NULL;
    instructions->cur   = NULL;
    instructions->count = 0;

    /* allocate errors list */
    errors = malloc(sizeof(errlist));

    /* initialize instructions variables */
    errors->head  = NULL;
    errors->cur   = NULL;
    errors->count = 0;

    /* allocate data list */
    data = malloc(sizeof(datalist));

    /* initialize instructions variables */
    data->head  = NULL;
    data->cur   = NULL;
    data->count = 0;


    /* Loop through query file, executing commands */
    while (fgets(line, LINE_LEN, fp))
    {
        /* increment line counter */
        counter++;

        /* check to see if we're in text section yet */
        if (found_text==0)
        {
            if (strstr(line,".text"))
            {
                found_text = 1;
            }
            else
            {
                /* nothing, disregard line until .text has been found */
                continue;
            }
        }
        else if (found_data == 0)
        {
            /* check for data section */
            if (strstr(line,".data"))
            {
                /* we've hit the data section, skip out of this branch */
                found_data = 1;
                continue;
            }

            /* text has been found, but not yet to data, attempt to read instruction */

            /* check for blank line */
            if (isBlank(line))
            {
                /* do nothing, line is blank */
                continue;
            }

            /* check for comment */
            if (commentExists(line))
            {
                if (isComment(line))
                {
                    /* do nothing, line is a comment */
                    continue;
                }
                else
                {
                    /* ok, there is an inline comment after the instruction, strip it out */
                    stripComment(line);
                }
            }

            /* trim any whitespace characters off the end of the line
               if they exist */
            strcpy(line, trimWhiteSpace(line));

            /* OK we now have our raw instruction text */

            /* clear out our strings */
            strcpy(label,"");
            strcpy(opname,"");
            strcpy(instargs,"");


            /* check to see if we have a label, if so, split up the instruction */
            if (strchr(line,':'))
            {
                /* line has a label, need to insert into symbols table */
                temp = strtok(line," \t");
                strcpy(label, temp);

                strcpy(line, "");
                strcat(line, strtok(NULL, " \t"));
                temp = strtok(NULL, " \t");
                if (temp != NULL)
                {
                    strcat(line, " ");
                    strcat(line, temp);
                }

                /* strip colon */
                if (label[strlen(label)-1] == ':')
                {
                    label[strlen(label)-1] = '\0';
                }

                /* label is now set, and the remaining text in temp is the instruction */

                /* add label to symbols table */
                genkey = hashgen(label, tSize);

                /* check if symbol already exists, if so, generate error */
                if (checkHash(hash_head, genkey, label, &addr))
                {
                    /* allocate error node and fill details */
                    temperr = malloc(sizeof(errnode));
                    temperr->errtype = ERR_MULTSYMBOL;
                    temperr->lineno = counter;
                    strcpy(temperr->symbol, label);
                    add_err(errors, temperr);
                }
                else
                {
                    /* symbol isnt defined yet, add to hash table */
                    hash_head = addhashnode(hash_head, genkey, label, instructions->count);
                }
            }
            if(DEBUG) printf("Line %s\n",line);
            /* split line */
            temp = strtok(line, " \t");
            /* copy out opname */
            strcpy(opname, temp);
            temp = strtok(NULL, " \t");
            if (temp != NULL)
            {
                strcpy(instargs, temp);
                if(DEBUG) printf("... args %s\n",temp);
            }

            /* clear out args */
            strcpy(arg1, "");
            strcpy(arg2, "");
            strcpy(arg3, "");

            /* attempt to split args */
            temp = strtok(instargs, ",");
            if (temp != NULL)
            {
                if(DEBUG) printf("... arg1 %s\n",temp);
                strcpy(arg1, temp);
            }
            temp = strtok(NULL, ",");
            if (temp != NULL)
            {
                if(DEBUG) printf("... arg2 %s\n",temp);
                strcpy(arg2, temp);
            }
            temp = strtok(NULL, ",");
            if (temp != NULL)
            {
                if(DEBUG) printf("... arg3 %s\n",temp);
                strcpy(arg3, temp);
            }


            /* allocate new instruction node and copy over the
                variables we know will be used for all instructions */
            tempinst = malloc(sizeof(instnode));
            tempinst->address = address;
            tempinst->lineno = counter;
            strcpy(tempinst->opcode_name, opname);
            strcpy(tempinst->label, label);

            /* zero out the dif fields of the instruction */
            strcpy(tempinst->rs1, "00000");
            strcpy(tempinst->rs2, "00000");
            strcpy(tempinst->rt,  "00000");
            strcpy(tempinst->sa,  "00000");
            strcpy(tempinst->imm, "0000000000000000");
            strcpy(tempinst->bin_inst,"");

            /* set error flag to 0 */
            was_error = 0;

            /* ok, label was handled if there was one, ready to insert instruction.
               we must go through all the dif opcode types, setting its variables
               appropriately  */
            if (strcmp(opname,"add")==0)
            {
                tempinst->inst_type = RTYPE;
                strcpy(tempinst->opcode_bin, "100000" );
                strcpy(tempinst->rt, regToBin(arg1));
                strcpy(tempinst->rs1, regToBin(arg2));
                strcpy(tempinst->rs2, regToBin(arg3));
            }
            else if (strcmp(opname,"addi")==0)
            {
                tempinst->inst_type = ITYPE;
                strcpy(tempinst->opcode_bin, "001000" );
                strcpy(tempinst->rt, regToBin(arg1));
                strcpy(tempinst->rs1, regToBin(arg2));
                strcpy(tempinst->imm, immToBin(arg3));
                printf("Addi %s %s %s %s",tempinst->opcode_bin, tempinst->rt, tempinst->rs1, tempinst->imm);

            }
            else if (strcmp(opname,"nor")==0)
            {
                tempinst->inst_type = RTYPE;
                strcpy(tempinst->opcode_bin, "100111" );
                strcpy(tempinst->rt, regToBin(arg1));
                strcpy(tempinst->rs1, regToBin(arg2));
                strcpy(tempinst->rs2, regToBin(arg3));
            }
            else if (strcmp(opname,"ori")==0)
            {
                tempinst->inst_type = RTYPE;
                strcpy(tempinst->opcode_bin, "001101" );
                strcpy(tempinst->rt, regToBin(arg1));
                strcpy(tempinst->rs1, regToBin(arg2));
                strcpy(tempinst->imm, immToBin(arg3));
            }
            else if (strcmp(opname,"sll")==0)
            {
                tempinst->inst_type = RTYPE;
                strcpy(tempinst->opcode_bin, "000000" );
                strcpy(tempinst->rt, regToBin(arg1));
                strcpy(tempinst->rs1, regToBin(arg2));
                strcpy(tempinst->sa, regToBin(arg3));
            }
            else if (strcmp(opname,"lui")==0)
            {
                tempinst->inst_type = ITYPE;
                strcpy(tempinst->opcode_bin, "001111" );
                strcpy(tempinst->rt, regToBin(arg1));
                strcpy(tempinst->imm, immToBin(arg2));
            }
            else if (strcmp(opname,"sw")==0)
            {
                tempinst->inst_type = ITYPE;
                strcpy(tempinst->opcode_bin, "101011" );
                strcpy(tempinst->rt, regToBin(arg1));

                /* need to do some parsing for the base + register stuff */
                temp = strtok(arg2,"(");
                strcpy(immarg, temp);
                temp = strtok(NULL, "()");
                strcpy(regarg,temp);

                strcpy(tempinst->imm, immToBin(immarg));
                strcpy(tempinst->rs1, regToBin(regarg));
            }
            else if (strcmp(opname,"lw")==0)
            {
                tempinst->inst_type = ITYPE;
                strcpy(tempinst->opcode_bin, "100011" );
                strcpy(tempinst->rt, regToBin(arg1));

                /* need to do some parsing for the base + register stuff */
                temp = strtok(arg2,"(");
                strcpy(immarg, temp);
                temp = strtok(NULL, "()");
                strcpy(regarg,temp);

                strcpy(tempinst->imm, immToBin(immarg));
                strcpy(tempinst->rs1, regToBin(regarg));
            }
            else if (strcmp(opname,"bne")==0)
            {
                tempinst->inst_type = ITYPE;
                strcpy(tempinst->opcode_bin, "000110" );
                strcpy(tempinst->rt, regToBin(arg1));
                strcpy(tempinst->rs1, regToBin(arg2));
                strcpy(tempinst->symbol, arg3);
            }
            else if (strcmp(opname,"j")==0)
            {
                tempinst->inst_type = JTYPE;
                strcpy(tempinst->opcode_bin, "000010" );
                strcpy(tempinst->symbol, arg1);
            }
            
            else if (strcmp(opname,"la")==0)
            {
                //lui
                char linen[LINE_LEN];
                fgets(linen, LINE_LEN, fp);
                /* used for splitting instruction args into sep args */
                char argi[LINE_LEN];
                /* clear out argi */
                strcpy(argi, "");
                if(DEBUG) printf("Line la %s\n",linen);
                /* split line */
                temp = strtok(linen, " \t");
                temp = strtok(NULL, " \t");
                temp = strtok(NULL, " \t");
                if (temp != NULL)
                {
                    strcpy(argi, temp);
                    if(DEBUG) printf("... argi %s\n",temp);
                }
                tempinst->inst_type = ITYPE;
                strcpy(tempinst->opcode_bin, "001111" );
                strcpy(tempinst->rt, regToBin(arg1));
                strcpy(tempinst->imm, subImmToBin(argi,31,16));

                add_node(instructions, tempinst);
                address++;

                tempinst = malloc(sizeof(instnode));
                tempinst->address = address;
                tempinst->lineno = counter;
                strcpy(tempinst->opcode_name, opname);
                strcpy(tempinst->label, label);
                
                strcpy(tempinst->rs1, "00000");
                strcpy(tempinst->rs2, "00000");
                strcpy(tempinst->rt,  "00000");
                strcpy(tempinst->sa,  "00000");
                strcpy(tempinst->imm, "0000000000000000");
                strcpy(tempinst->bin_inst,"");
                //ori
                tempinst->inst_type = RTYPE;
                strcpy(tempinst->opcode_bin, "001101" );
                strcpy(tempinst->rt, regToBin(arg1));
                strcpy(tempinst->rs1, regToBin(arg1));
                strcpy(tempinst->imm, subImmToBin(argi,15,0));

            }
            else
            {
                /* bad opcode given, throw error */
                temperr = malloc(sizeof(errnode));
                temperr->errtype = ERR_OPCODE;
                temperr->lineno = counter;
                strcpy(temperr->opcode, opname);

                add_err(errors,temperr);

                /* set was_error flag to true */
                was_error = 1;

            }
            if (was_error == 0)
            {
                /* there was no error so add the node to the instruction list */
                add_node(instructions, tempinst);

                /* increment address */
                address++;
            }
        }
        else
        {
            /* we're now in the data section, look for a directive */

            /* check for blank line */
            if (isBlank(line))
            {
                /* do nothing, line is blank */
                continue;
            }

            /* check for comment */
            if (commentExists(line))
            {
                if (isComment(line))
                {
                    /* do nothing, line is a comment */
                    continue;
                }
                else
                {
                    /* ok, there is an inline comment after the instruction, strip it out */
                    stripComment(line);
                }
            }

            /* clear out string variables */
            strcpy(label, "");
            strcpy(instargs, "");
            strcpy(arg1, "");
            strcpy(arg2, "");
            strcpy(directive, "");


            /* trim any whitespace characters off the end of the line
               if they exist */
            strcpy(line, trimWhiteSpace(line));

            /* OK so at this point we should have a label, directive, arguments line */
            temp = strtok(line," ");
            strcpy(label, temp);

            /* split into directive and args */
            temp = strtok(NULL," ");
            if (temp != NULL)
            strcpy(directive, temp);

            temp = strtok(NULL, "");
            if (temp != NULL)
            {
                strcpy(instargs,temp);
            }
            /* strip off the colon */
            if (label[strlen(label)-1] == ':')
            {
                label[strlen(label)-1] = '\0';
            }

            /* add label to symbols table */
            genkey = hashgen(label, tSize);
            /* check if symbol already exists, if so, generate error */
            if (checkHash(hash_head, genkey, label, &addr))
            {
                /* allocate error node and fill details */
                temperr = malloc(sizeof(errnode));
                temperr->errtype = ERR_MULTSYMBOL;
                temperr->lineno = counter;
                strcpy(temperr->symbol, label);
                add_err(errors, temperr);
            }
            else
            {
                /* symbol isnt defined yet, add to hash table */
                hash_head = addhashnode(hash_head, genkey, label, address);
            }

            /* check for .word directive */
            if (strcmp(directive, ".word")==0)
            {
                /* split args at the colon */
                temp = strtok(instargs, ":");
                strcpy(arg1, temp);
                temp = strtok(NULL, ":");
                strcpy(arg2, temp);

                /* loop through and add data field for X amount of entries */
                for (i=0; i<atoi(arg2); i++)
                {
                    /* copy over arg for formatting */
                    strcpy(arg3, arg1);

                    /* allocate new data node and fill details */
                    tempdata = malloc(sizeof(datanode));
                    tempdata->address = address;
                    tempdata->lineno  = counter;
                    strcpy(tempdata->label, label);
                    strcpy(tempdata->binval, numTo32Bin(arg3));
                    strcpy(arg3,tempdata->binval);
                    strcpy(tempdata->hex_val, binToHex32(arg3));

                    /* add new data node to data list */
                    add_datanode(data, tempdata);

                    /* increment address counter */
                    address++;
                }
            }
            /* check for .resw directive */
            else if (strcmp(directive, ".resw")==0)
            {
                /* copy 0 to arg because resw statements zero out the data field */
                strcpy(arg1,"0");

                /* loop through and create field for X amount */
                for (i=0; i<atoi(instargs); i++)
                {
                    /* copy arg for formatting */
                    strcpy(arg3, arg1);

                    /* allocate new data node and fill details */
                    tempdata = malloc(sizeof(datanode));
                    tempdata->address = address;
                    tempdata->lineno  = counter;
                    strcpy(tempdata->label, label);
                    strcpy(tempdata->binval, numTo32Bin(arg3));
                    strcpy(arg3,tempdata->binval);
                    strcpy(tempdata->hex_val, binToHex32(arg3));

                    /* add new data node to data list */
                    add_datanode(data, tempdata);

                    /* increment address counter */
                    address++;
                }
            } /* end if .resw */
        } /* end: else data section */
    } /* end while fgets */

    /* close file */
    fclose(fp);

    /* alright file has been processed at this point.
       instructions and data directives are in their respective lists.
       we must now go through both and assemble them into binary and
       then into hex. symbols must be evaluated through the hash table
       and errors generated if they are invalid */

    /* set traversal node to head of list */
    instructions->cur = instructions->head;

    /* loop through and assemble instructions */
    while (instructions->cur != NULL)
    {
        /* check for RTYPE instruction and format acoordingly */
        if (instructions->cur->inst_type == RTYPE)
        {
            /* assemble binary instruction */
            sprintf(instructions->cur->bin_inst, "%s%s%s%s%s000000", instructions->cur->opcode_bin, instructions->cur->rs1,
                    instructions->cur->rs2, instructions->cur->rt, instructions->cur->sa);
            /* copy to temp var */
            strcpy(line,instructions->cur->bin_inst);

            /* convert to hex and copy to hex field in node */
            strcpy(instructions->cur->hex_inst, binToHex32(line));
        }
        /* check for ITYPE instruction and format acoordingly */
        else if (instructions->cur->inst_type == ITYPE)
        {
            /* assemble binary instruction */
            sprintf(instructions->cur->bin_inst, "%s%s%s%s", instructions->cur->opcode_bin, instructions->cur->rs1,
                    instructions->cur->rt, instructions->cur->imm);
            /* copy to temp var */
            strcpy(line,instructions->cur->bin_inst);

            /* convert to hex and copy to hex field in node */
            strcpy(instructions->cur->hex_inst, binToHex32(line));
        }
        /* check for JTYPE instruction and format acoordingly */
        else if (instructions->cur->inst_type == JTYPE)
        {
            /* check and see if symbol is defined in the hash table */
            if (checkHash(hash_head, hashgen(instructions->cur->symbol, tSize), instructions->cur->symbol, &addr))
            {
                /* symbol exists, so assemble instruction */
                sprintf(line,"%d",addr);
                sprintf(instructions->cur->bin_inst, "%s%s%s%s", instructions->cur->opcode_bin, instructions->cur->rs1,
                        instructions->cur->rt, immToBin(line));
                strcpy(line,instructions->cur->bin_inst);

                /* convert to hex and copy to hex field in node */
                strcpy(instructions->cur->hex_inst, binToHex32(line));
            }
            else
            {
                /* need to generate an error, symbol is invalid */

                /* allocate error node and fill details */
                temperr = malloc(sizeof(errnode));
                temperr->errtype = ERR_UNDEFSYMBOL;
                temperr->lineno = instructions->cur->lineno;
                strcpy(temperr->symbol, instructions->cur->symbol);

                /* add error node to error list */
                add_err(errors, temperr);
            }
        }
        /* traverse to next node in instructions list */
        instructions->cur = instructions->cur->next;
    } /* end while */

    /* instructions are now assembled in hex, ready to be printed */

    /* if errors exist, write to error file */

    if (errors->count > 0 )
    {
        /* attempt to open asm file */
        if ((fp = fopen(file, "r")) == NULL)
        {
            fprintf(stderr, "Error opening asm file: %s\n", file);
            exit(1);
        }

        /* format error file name */
        sprintf(errfile, "%s.err", strtok(file, "."));

        /* attempt to open error file */
        if ((errfp = fopen(errfile, "w")) == NULL)
        {
            fprintf(stderr, "Error opening error file: %s\n", errfile);
            exit(1);
        }

        /* loop through asm file and write all lines to error file with
           prefixed line numbers */
        /*reset counter to 0 */
        counter = 0;

        while (fgets(line, LINE_LEN, fp))
        {
            /* increment line counter */
            counter++;
            fprintf(errfp,"%2d   %s", counter, line);
        }
        /* close asm file */
        fclose(fp);

        /* clear some lines in error file */
        fprintf(errfp, "\n");

        fprintf(errfp, "Errors detected:\n\n");

        /* loop through errors and generate error report */
        errors->cur = errors->head;
        while (errors->cur != NULL)
        {
            /* if error i opcode, show proper message */
            if (errors->cur->errtype == ERR_OPCODE)
            {
                fprintf(errfp,"  line %2d:  Illegal opcode.\n", errors->cur->lineno);

            }
            /* else show message for undefined symbol */
            else if (errors->cur->errtype == ERR_UNDEFSYMBOL)
            {
                fprintf(errfp,"  line %2d:  Undefined symbol used.\n", errors->cur->lineno);
            }

            /* traverse to next error */
            errors->cur = errors->cur->next;
        }

        /* clear a line */
        fprintf(errfp, "\n");

        /* check for multiply defined symbols */
        /* loop through errors */
        errors->cur = errors->head;
        while (errors->cur != NULL)
        {
            /* check for multiple symbol errors */
            if (errors->cur->errtype == ERR_MULTSYMBOL)
            {
                /* print title */
                fprintf(errfp,"Multiply defined symbol(s):\n\n");

                /* loop through and print all multiple symbol errors */
                errors->cur = errors->head;
                while (errors->cur != NULL)
                {
                    if (errors->cur->errtype == ERR_MULTSYMBOL)
                    {
                        fprintf(errfp, "  %s\n", errors->cur->symbol);
                    }
                    errors->cur = errors->cur->next;
                }
                break;
            }
            /* traverse to next error node */
            errors->cur = errors->cur->next;
        } /* end while */

        /* clear a line */
        fprintf(errfp, "\n");

        /* check for undefined  symbols */
        /* loop through errors */
        errors->cur = errors->head;
        while (errors->cur != NULL)
        {
            if (errors->cur->errtype == ERR_UNDEFSYMBOL)
            {
                /* print title */
                fprintf(errfp,"Undefined symbol(s):\n\n");

                /* loop through and print all undefined symbol errors */
                errors->cur = errors->head;
                while (errors->cur != NULL)
                {
                    if (errors->cur->errtype == ERR_UNDEFSYMBOL)
                    {
                        fprintf(errfp, "  %s\n", errors->cur->symbol);
                    }
                    /* traverse to next node */
                    errors->cur = errors->cur->next;
                }
                break;
            }
            /* traverse to next node */
            errors->cur = errors->cur->next;
        } /* end while */


        /* close error file */
        fclose(errfp);

    } /* end errors */
    else
    {
        /* ok we got no errors so write the obj file */

        /* format object file name */
        sprintf(file, "%s.obj", strtok(file, "."));

        /* attempt to open object file */
        if ((fp = fopen(file, "w")) == NULL)
        {
            fprintf(stderr, "Error opening obj file: %s\n", file);
            exit(1);
        }

        /* loop through instructions writing them to the obj file */
        instructions->cur = instructions->head;

        while (instructions->cur != NULL)
        {
            /* print instruction in hex: address - instruction */
            fprintf(fp,"0x0000%s:\t0x%s\n", addrToHex(instructions->cur->address, line), instructions->cur->hex_inst);

            /* traverse to next instruction node */
            instructions->cur = instructions->cur->next;
        }

        /* now write data */
        data->cur = data->head;
        while (data->cur != NULL)
        {
            /* format and print data entry */
            fprintf(fp,"0x0000%s:\t0x%s\n", addrToHex(data->cur->address, line), data->cur->hex_val);

            /* traverse to next data entry node */
            data->cur = data->cur->next;
        }
    }
    printf("========\nCheck %s for output\n=========", file);
    /* yay, we're finally done and can delete our data structures */
    delete_errlist(errors);
    delete_list(instructions);
    delete_datalist(data);
    deletetable(hash_head);

    /**************** END main executables *********************/

    /* exit program */
    return 0;
}



/******************* Functions ******************/

int hashgen(char *s,  int T) {

   /* The parameter s represents the symbol to be hashed and  */
   /* the parameter T represents the size of the hash table.  */
   /* The function returns the hash value for the symbol s.   */

   /* String s is assumed to be terminated with '\0'.         */
   /* It is also assumed that T is at least 2. The returned   */
   /* hash value is an integer in the range 0 to T-1.         */

   /* The function computes the hash value using arithmetic   */
   /* based on powers of the BASE value defined below.        */

   #define  BASE   127

   int h = 0;     /* Will hold the hash value at the end. */
   int temp;      /* Temporary.                           */

   /* The hash value is computed in the for loop below. */
   for (;  *s != 0;  s++) {
       temp = (BASE * h + *s);
       if (temp < 0) temp = -temp;
       h = temp % T;
   }

   /* The hash value computation is complete. So, */
   return h;

} /* End of hash_example_one */

tnode* addhashnode(tnode* head, int key, const char* val, int address)
{
  tnode* cur  = NULL; //used for traversing the table
  lnode* lcur = NULL; //used for traversing the lists

  /* Case 1 - no nodes in table yet
   * -create new table node and make that the head
   * -create new listnode and add as an entry
   */
  if (head == NULL)
  {
    //allocate new node for head
    head = malloc(sizeof(tnode));

    //copy key to new node
    head->key = key;

    //allocate new node for the first entry under this index
    head->head = malloc(sizeof(lnode));

    //copy the value to the new entry
    strcpy(head->head->value, val);

    //copy address to the new entry
    head->head->address = address;

    //set the new entry nodes' next ptr to null
    head->head->next = NULL;

    //set number of entries for this index to 1
    head->entries = 1;

    //set the next ptr for this index node to null
    head->next = NULL;

    //return new head value
    return head;
  }
  /* Case 2 - head node defined, search for matching key
   * -if matching key found, add new entry
   * -if no matching key found, create new table and then add entry
   */
  cur = head;
  while (cur != NULL)
  {
    if (cur->key == key)
    {
      //got match, loop to end of list and append new entry node
      lcur = cur->head;
      while (lcur->next != NULL)
        lcur = lcur->next;

      //allocate new entry node
      lcur->next = malloc(sizeof(lnode));

      //copy value to new node
      strcpy(lcur->next->value, val);

      //copy address to the new entry
            lcur->next->address = address;


      //set new nodes next ptr to null
      lcur->next->next = NULL;

      //increment number of entries for this index
      cur->entries++;

      //return head ptr
      return head;
    }
    //reached end of list
    //add node onto end of table and then add new entry
    if (cur->next == NULL)
    {
      //allocate new table node
      cur->next = malloc(sizeof(tnode));

      //set table nodes key
      cur->next->key = key;

      //allocate new entry node
      cur->next->head = malloc(sizeof(lnode));

      //set new entry node's next ptr to null
      cur->next->head->next = NULL;

      //copy the value to the new entry
      strcpy(cur->next->head->value, val);

      cur->next->head->address = address;

      //set new table's entry variable to 1
      cur->next->entries=1;

      //set new nodes next to null
      cur->next->next = NULL;

      //exit loop
      cur = NULL;
    }
    else
    {
      //traverse to next node
      cur=cur->next;
    }
  }
  //return ptr to head
  return head;
}

/* this function takes in a head pointer, a key, a value, and a pointer
  to return the address found into if such a hash is found */
int checkHash(tnode* head, int key,const char* val, int *address)
{
    /* temp nodes for traversal */
    tnode* tcur = NULL;
    lnode* lcur = NULL;

    /* set traversal node to head of list */
    tcur = head;

    /* loop through looking for desired key */
    while (tcur != NULL)
    {
        if (tcur->key == key)
        {
            /* found key, loop through looking for matching value */
            lcur = tcur->head;
            while (lcur != NULL)
            {
                if (strcmp(lcur->value, val)==0)
                {
                    /* found matching vlaue, set the address and return true */
                    *address = lcur->address;
                    return 1;
                }
                /* traverse to next node */
                lcur = lcur->next;
            }
        }
        /* traverse to next node */
        tcur = tcur->next;
    }

    /* matching key and vlaue were not found, return false */
    return 0;
}


void deletetable(tnode* head)
{
  //node ptrs used for place holders / traversing
  tnode* tcur  = NULL;
  tnode* tcur2 = NULL;
  lnode* lcur  = NULL;
  lnode* lcur2 = NULL;

  //exit if table is empty
  if (head == NULL)
    return;

  //loop through table
  tcur = head;
  while (tcur != NULL)
  {
    //loop through entry list
    lcur = tcur->head;
    while (lcur != NULL)
    {
      //delete each entry
      lcur2 = lcur;
      lcur = lcur->next;
      free(lcur2);
    }
    //delete each index
    tcur2 = tcur;
    tcur = tcur->next;
    free(tcur2);
  }
}
/* instructions.c 

   this file contains the functions used to facilitate
   the linked list that will hold all the assembly instructions
*/


/*************** functions ***********************/

/* this function takes in a list pointer and a node pointer,
   and will then add the node to the end of the list */
void add_node(instlist *list, instnode *node)
{
    /* if head is null, make new node the head of the list */
    if (list->head == NULL)
    {
        list->head = node;
        list->count++;
    }
    else
    {
        /* set the traversal node to head */
        list->cur = list->head;

        /* loop to end of list */
        while (list->cur->next != NULL)
        {
            list->cur = list->cur->next;
        }
        /* set last node's next pointer to our new node */
        list->cur->next = node;
        list->count++;
    }
}

/* this function takes in a list pointer and will traverse through
  the list deleting nodes and then delte itself */
void delete_list(instlist* list)
{
    /*** Variables ***/
    instnode *temp = NULL; /* temp node to hold places */


    if (list->head != NULL)
    {
        list->cur = list->head;
        while (list->cur != NULL)
        {
            temp = list->cur;
            list->cur = list->cur->next;
            free(temp);
        }
    }
    free(list);
}


/***************** Functions ******************/

/* this function takes in a list pointer and a node pointer,
   and will then add the node to the end of the list */
void add_datanode(datalist *list, datanode *node)
{
    /* if head is null, make new node the head of the list */
    if (list->head == NULL)
    {
        list->head = node;
        list->count++;
    }
    else
    {
        /* set the traversal node to head */
        list->cur = list->head;

        /* loop to end of list */
        while (list->cur->next != NULL)
        {
            list->cur = list->cur->next;
        }
        /* set last node's next pointer to our new node */
        list->cur->next = node;
        list->count++;
    }
}


/* this function takes in a list pointer and will traverse through
  the list deleting nodes and then delete the list itself */
void delete_datalist(datalist* list)
{
    /*** Variables ***/
    datanode *temp = NULL; /* temp node to hold places */


    if (list->head != NULL)
    {
        list->cur = list->head;
        while (list->cur != NULL)
        {
            temp = list->cur;
            list->cur = list->cur->next;
            free(temp);
        }
    }
    free(list);
}
/* errorlist.c -   this file contains functions for use with the
   error list.
*/

/***************** Functions  ***************/

/* this function takes in a list pointer and a node pointer,
   and will then add the node to the list in order of line number */
void add_err(errlist *list, errnode *node)
{
    node->next = NULL;

    /* if head is null, make new node the head of the list */
    if (list->head == NULL)
    {
        list->head = node;
        list->count++;
    }
    else
    {

        /* is lineno before head? replace */
        if (node->lineno < list->head->lineno)
        {
            node->next = list->head;
            list->head = node;
            list->count++;
        }
        else
        {
            /* set the traversal node to head */
            list->cur = list->head;

            /* loop through list looking for insertion point */
            while (list->cur != NULL)
            {
                if (list->cur->next != NULL)
                {

                    if (node->lineno < list->cur->next->lineno)
                    {
                        /* found insertion point, add node in */
                        node->next = list->cur->next;
                        list->cur->next = node;
                        list->count++;
                        return;
                    }
                }
                else
                {
                    list->cur->next = node;
                    list->count++;
                    return;
                }
                list->cur = list->cur->next;
            }
            /* set last node's next pointer to our new node */
       //     list->cur->next = node;
          //  list->count++;
        }
    }
}

/* this function takes in a list pointer and will traverse through
  the list deleting nodes and then delete the list itself */
void delete_errlist(errlist* list)
{
    /*** Variables ***/
    errnode *temp = NULL; /* temp node to hold places */


    if (list->head != NULL)
    {
        list->cur = list->head;
        while (list->cur != NULL)
        {
            temp = list->cur;
            list->cur = list->cur->next;
            free(temp);
        }
    }
    free(list);
}