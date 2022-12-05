.IFJcode22
DEFVAR GF@$f
DEFVAR GF@$i
DEFVAR GF@$j
DEFVAR GF@tempVar&1
DEFVAR GF@tempVar&5
MOVE GF@$f int@0
MOVE GF@$i int@0
LABEL forStart&0
# Type of variable $i is int.
LT GF@tempVar&1 GF@$i int@3
JUMPIFEQ forEnd&0 GF@tempVar&1 bool@false
# Type of variable $f is int.
ADD GF@$f GF@$f int@1
MOVE GF@$j int@0
LABEL forStart&2
# Type of variable $j is int.
LT GF@tempVar&1 GF@$j int@3
JUMPIFEQ forEnd&2 GF@tempVar&1 bool@false
# Type of variable $f is int.
ADD GF@$f GF@$f int@1
# Break statement
JUMP forEnd&0
# Type of variable $j is int|undefined.
TYPE GF@tempVar&1 GF@$j
JUMPIFNEQ variable_defined&3 GF@tempVar&1 string@
DPRINT string@Variable\032$j\032is\032not\032defined.
EXIT int@5
LABEL variable_defined&3
ADD GF@tempVar&1 GF@$j int@1
MOVE GF@$j GF@tempVar&1
JUMP forStart&2
LABEL forEnd&2
# Type of variable $i is int|undefined.
TYPE GF@tempVar&5 GF@$i
JUMPIFNEQ variable_defined&4 GF@tempVar&5 string@
DPRINT string@Variable\032$i\032is\032not\032defined.
EXIT int@5
LABEL variable_defined&4
ADD GF@tempVar&5 GF@$i int@1
MOVE GF@$i GF@tempVar&5
JUMP forStart&0
LABEL forEnd&0
# Type of variable $f is int.
JUMPIFNEQ ifElse&6 GF@$f int@2
WRITE string@GOOD\010
JUMP ifEnd&6
LABEL ifElse&6
WRITE string@BAD\010
LABEL ifEnd&6
EXIT int@0
