;; List

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
'map ( swap lcopy swap map! ) def

'fold ( d f |
  dup null? if
    drop d
  else
    dup car d f i => d cdr lp
  then ) def

;; IO

'print ( display newline ) def
'with-output-to-string ( open-output-string dup stdout! swap i dup stdout! port->string ) def
'stack-string-join ( f | ( f (display) -stack-each ) with-output-to-string ) def

;; Dict

'dict-each ( dict f | ( dict dict-keys ) f stack-each ) def

;; Vocab

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

;; Context

'global '() val def
'global! ( => global ) def context global!
'>global ( global cons global! ) def
'also ( global context! ) def
'local: ( '%local vocab* >context ) def
'local? ( vocab-name '%local eq? ) def
