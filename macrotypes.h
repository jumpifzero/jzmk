typedef uint8_t byte;

typedef struct action_s {
  byte action;
  byte key;  
} action;

typedef struct macro_s { 
  byte len;
  action* actions;  
} macro;
