"lib/core.sl" load

;; List ;;

'( 2 3 4 ) ( 2 * ) map '( 4 6 8 ) equal? test
'( a b c d ) '() ( cons ) fold '( d c b a ) equal? test
'(a b 2 c d) (number?) split-list '((a b) (c d)) equal? test

;; IO ;;

;; Dict ;;

;; Vocab ;;

vocab-list dict? test
context car vocab-name 'core eq? test
'foo vocab* dup dict? test
dup vocab-name 'foo eq? test
'foo vocab* eq? test

;; Context ;;

'ffffff lookup not test
'map lookup drop block? test

