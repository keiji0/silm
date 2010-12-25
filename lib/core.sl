;; Test

'test-mode #f val def
'test-on ( #t => test-mode ) def
'test-off ( #f => test-mode ) def
'test: ( read test-mode if eval i else drop then ) def

;; List ;;

'each ( f |
  dup null? if
    drop
  else
    dup car f i cdr lp
  then ) def

'map* ( f |
  dup null? if
    drop
  else
    dup car f i over car! cdr dup drop lp
  then ) def 

'map! ( over swap map* ) def
'map ( [ list fun > list2 ] swap lcopy swap map! ) def

'fold ( d f |
  dup null? if
    drop d
  else
    dup car d f i => d cdr lp
  then ) def

'list-times ( l n f |
  l null? not if
    l car n f i
    l cdr => l
    n 1 + => n lp
  then ) def

'car-reverse! ( dup dup car reverse! swap car! ) def

'split-list ( l p | [ > l2 ]
  l '() '() cons ( a s |
   a p i if
      '() s car-reverse! cons
    else
      a s car cons s car! s
    then ) fold
  dup car null? if
    drop '()
  else
    car-reverse! reverse!
  then ) def

;; Loop ;;

'times ( n f | n 0 > if n f i n 1 - => n lp then ) def

;; IO ;;

'print ( display newline ) def
'with-output-to-string ( open-output-string dup stdout! swap i dup stdout! port->string ) def
'stack-string-join ( f | ( f (display) -stack-each ) with-output-to-string ) def

;; Dict ;;

'dict-each ( dict f | ( dict dict-keys ) f stack-each ) def

;; Vocab ;;

'vocab-list 10 make-dict def
'vocab-name ( '%name swap dict-get ) def
'vocab-name! ( '%name swap dict-put! ) def
'vocab-def ( 2dup vocab-name! swap vocab-list dict-put! ) def
'vocab-find ( vocab-list dict-get ) def
'vocab* ( name |
  name vocab-find dup undefined? if
    drop 10 make-dict dup name swap vocab-def
  then ) def
'vocab: ( read vocab* >context ) def
'core context car vocab-name!
'.context ( context ( vocab-name write newline ) each ) def

;; Context ;;

'global '() val def
'global! ( => global ) def context global!
'>global ( global cons global! ) def
'also ( global context! ) def
'local: ( '%local vocab* >context ) def
'local? ( vocab-name '%local eq? ) def

;; Record ;;

'record-name ( 0 swap vector-ref ) def
'record-name! ( 0 swap vector-set! ) def
'record-slot ( 1 swap vector-ref ) def
'record-slot! ( 1 swap vector-set! ) def
'record-super ( 2 swap vector-ref ) def
'record-super! ( 2 swap vector-set! ) def
'record-size ( record-slot length ) def
'make ( record-size make-vector ) def

'record-getter-name-gen ( [ symbol > symbol1 ]
  symbol->string "@" string-append string->symbol ) def

'record-setter-name-gen ( [ symbol > symbol1 ]
  symbol->string "!" string-append string->symbol ) def

'record-slot-def ( [ symbol id > ]
  2dup
  swap record-getter-name-gen swap
    ( swap vector-ref ) data+block def
  swap record-setter-name-gen swap
    ( swap vector-set! ) data+block def ) def

'record-slot-build ( [ record > ]
  record-slot 0 ( record-slot-def ) list-times ) def

'record* ( name slot | [ > record ]
  3 make-vector
    name over record-name!
    slot over record-slot!
    dup record-slot-build ) def

'record: ( [ "name" "(slot ...)" ]
  read dup read record* def ) def
