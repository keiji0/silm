"lib/cilm.sl" load

package: aaa

cilm-define-dict package-name 'aaa eq? test
'aaa 8 cilm-define
'aaa cilm-lookup 8 = test
'aab cilm-lookup not test

package: bbb

cilm-define-dict package-name 'bbb eq? test
'aaa cilm-lookup 8 = test
cilm-order length 2 = test
cilm-order car  package-name 'aaa eq? test



'(a b) split-in&out .stack 0 bye

'(a b > 3) split-in&out .stack 0 bye

'(a b c ) ( ', eq? ) split-list cdr ( #f ) split-list .stack 0 bye

'( const int *n, int x ) ( '> eq? ) split-list


dup car ( ', eq? ) split-list
swap cdr  ( ', eq? ) split-list

.stack 0 bye

fun: hoge (int n, const char *name > int) (
  (printf "%d\n" n)
  (return 8)
)



fun: hoge (int: n int: x > int: y) (
  (print "%d %d\n" n x)
  (return (+ n x))
)

0 bye

;; .stack 0 bye

;; struct: Point (
;; )

;; package: graphic

;; private:
  
;; public:

;; struct: Point (
;;   (int x)
;;   (int y)
;; )

;; fun: main (int n | int y > int) (
;;   (printf "%d\n" n)
;;   (return (+ n n))
;; )

;; fun: foo ( int n | int y > int ) (
;;   (printf "%d\n" n)
;; ) fun

;; fun: main ( int n | int y > int ) (
;;   (foo 8)
;;   (return (+ n n))
;; )

;; package: abc

;; (graphic.foo "hoge")
;; (return (graphic.main))
