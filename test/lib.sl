"lib/core.sl" load

;; List

'(2 3 4) (2 *) map '(4 6 8) equal? test
'(a b c d) '() (cons) fold '(d c b a) equal? test

;; IO

;; Dict

;; Vocab

vocab-list dict? test
context car vocab-name 'core eq? test
'foo vocab* dup dict? test
dup vocab-name 'foo eq? test
'foo vocab* eq? test

;; Context

'ffffff lookup not test
'map lookup drop block? test

local:
  'foo 8 def

vocab: hoge
  'foo lookup test 8 = test
  'bar 20 def

also

'hoge vocab-find vocab-name 'hoge eq? test
'foo lookup not test
'bar lookup not test
'hoge vocab-find >context
'bar lookup test 20 = test
'foo lookup not test
