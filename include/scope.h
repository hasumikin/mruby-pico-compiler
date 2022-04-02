#ifndef PICORBC_SCOPE_H_
#define PICORBC_SCOPE_H_

#include <stdint.h>

#define MRB_HEADER_SIZE  32
#define MRB_FOOTER_SIZE   8
#define IREP_HEADER_SIZE 14

typedef enum literal_type
{
  STRING_LITERAL  = 0,
  INT32_LITERAL   = 1,
  SSTRING_LITERAL = 2,
  INT64_LITERAL   = 3,
  FLOAT_LITERAL   = 5,
  BIGINT_LITERAL   = 7
} LiteralType;

#define IREP_TT_NFLAG 1       /* number (non string) flag */
#define IREP_TT_SFLAG 2       /* static string flag */

typedef struct literal
{
  uint32_t type;    /* LiteralType and lenght(<<2) when it's a STRING */
  const char *value;
  struct literal *next;
} Literal;

typedef struct gen_literal
{
  const char *value;
  struct gen_literal *prev;
} GenLiteral;

/*
 * A symbol can be:
 *  @ivar, $gvar, puts(fname), :symbol, CONST
 */
typedef struct symbol
{
  uint32_t len; /* used in dump.c */
  const char *value;
  struct symbol *next;
} Symbol;

typedef struct lvar
{
  uint32_t len;
  const char *name;
  struct lvar *next;
  uint8_t regnum;
} Lvar; /* will be casted as Symbol in dump.c */

typedef struct lvar_scope_reg
{
  uint8_t scope_num;
  uint8_t reg_num;
} LvarScopeReg;

#define CODE_POOL_SIZE 25
typedef struct code_pool
{
  struct code_pool *next;       //     4 bytes or 8 bytes
  uint16_t size;                //     2 bytes
  uint8_t index;                //     1 bytes
  uint8_t data[CODE_POOL_SIZE]; //    25 bytes (Note: It can be bigger)
                                // => 32 bytes total in 32 bit
                                // or 40 bytes total in 64 bit
} CodePool;

typedef struct retry_stack
{
  uint16_t pos;
  struct retry_stack *prev;
} RetryStack;

typedef struct break_stack
{
  void *point;
  struct break_stack *prev;
  uint16_t next_pos;
  uint16_t redo_pos;
} BreakStack;

/*
 * For symbols which aren't stored in ParserState's string_pool
 * like `[]=` `attr=`
 * They should be created in generator.c
 */
typedef struct assign_symbol
{
  struct assign_symbol *prev;
  const char *value;
} AssignSymbol;

typedef struct jmp_label
{
  void *address;
  uint16_t pos;
} JmpLabel;

typedef struct backpatch
{
  JmpLabel *label;
  struct backpatch *next;
} Backpatch;

typedef struct exception_handler ExcHandler;
typedef struct exception_handler
{
  uint8_t table[13];
  ExcHandler *next;
} ExcHandler;

typedef struct scope Scope;
typedef struct scope
{
  uint32_t irep_parameters; /* bbb */
  uint32_t nest_stack; /* Initial: 00000000 00000000 00000000 00000001 */
  Scope *upper;
  Scope *first_lower;
  Scope *next;
  bool lvar_top;
  uint16_t next_lower_number;
  unsigned int nlowers;
  CodePool *first_code_pool;
  CodePool *current_code_pool;
  unsigned int nlocals;
  Symbol *symbol;
  Lvar *lvar;
  Literal *literal;
  GenLiteral *gen_literal; /* Exceptional literals in generator */
  unsigned int sp;
  unsigned int max_sp;
  uint16_t ilen;
  uint16_t slen;
  uint16_t plen;
  uint16_t clen;
  uint16_t vm_code_size;
  uint8_t *vm_code;
  BreakStack *break_stack;
  RetryStack *retry_stack;
  AssignSymbol *last_assign_symbol;
  Backpatch *backpatch; /* for backpatching of JMP label */
  int nargs_before_splat;
  uint8_t gen_splat_status;
  uint8_t gen_array_status;
  uint8_t gen_array_count;
  ExcHandler *exc_handler;
} Scope;

#define GEN_SPLAT_STATUS_NONE              0b00000000
#define GEN_SPLAT_STATUS_BEFORE_SPLAT      0b00000001
#define GEN_SPLAT_STATUS_AFTER_SPLAT       0b00000010
#define GEN_ARRAY_STATUS_NONE              0b00000000
#define GEN_ARRAY_STATUS_GENERATING        0b00000001
#define GEN_ARRAY_STATUS_GENERATING_SPLIT  0b00000010

Scope *Scope_new(Scope *upper, bool lvar_top);

void Scope_free(Scope *self);

void Scope_pushBackpatch(Scope *self, JmpLabel *label);

void Scope_shiftBackpatch(Scope *self);

void Scope_pushNCode_self(Scope *self, uint8_t *value, int size);
#define Scope_pushNCode(v, s) Scope_pushNCode_self(scope, (v), (s))

void Scope_pushCode_self(Scope *self, int val);
#define Scope_pushCode(v) Scope_pushCode_self(scope, (v))

int Scope_newLit(Scope *self, const char *value, LiteralType type);

int Scope_newSym(Scope *self, const char *value);

int Scope_assignSymIndex(Scope *self, const char *method_name);

LvarScopeReg Scope_lvar_findRegnum(Scope *self, const char *name);

int Scope_newLvar(Scope *self, const char *name, int newRegnum);

void Scope_push(Scope *self);

void Scope_finish(Scope *self);

void Scope_freeCodePool(Scope *self);

void Scope_freeLvar(Lvar *);

JmpLabel *Scope_reserveJmpLabel(Scope *self);

void Scope_backpatchJmpLabel(JmpLabel *label, uint16_t position);

void Scope_pushRetryStack(Scope *self);

void Scope_popRetryStack(Scope *self);

void Scope_pushBreakStack(Scope *self);

void Scope_popBreakStack(Scope *self);

int Scope_updateVmCodeSizeThenReturnTotalSize(Scope *self);

#endif
