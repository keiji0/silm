;; Boolean 

'() null? test
'() atom? test
#t #t eq? test
#f #f eq? test
#t atom? test
#f atom? test
#t not #f eq? test
#f not #t eq? test
#t not not test
#f not not #f eq? test

;; Char ;;

#\a atom? test
#\a char? test
#\A char? test
#\space char? test
#\cr char? test
#\tab char? test
'a char? not test
3 char? not test
'(a) char? not test

;; Number ;;

3 atom? test
3 number? test
0 number? test
55 number? test
-1 number? test
-888 number? test
#\a number? not test
'b number? not test
'(a) number? not test
1 2 + 3 eq? test
8 8 = test
2 8 = not test
2 8 + 10 = test
8 2 - 6 = test
8 2 / 4 = test
2 2 * 4 = test
5 5 * 25 = test
8 neg -8 = test
8 abs 8 = test
8 2 > test
8 2 < not test
8 8 > not test
8 8 < not test
8 8 <= test
8 8 >= test

;; List ;;

5 2 cons atom? not test
5 2 cons pair? test
7 8 cons car 7 = test
7 8 cons cdr 8 = test
10 10 cons dup 8 swap car! car 8 = test
10 10 cons dup 9 swap cdr! cdr 9 = test
1 2 cons 3 cons caar 1 = test
1 2 3 () cons cons cons length 3 = test
'(a b) '(a b) equal? test
'(a b) '(b a) equal? not test
'(a (a (b c))) '(a (a (b c))) equal? test
'() null? test
'(()) car null? test
'(()()) '() '() '() cons cons equal? test
0 null? not test
#\a null? not test
'(a b c) '(a b c) equal? test
'(a c d) '(a b c) equal? not test
'(a (a b)) '(a (a b)) equal? test
'(a "b") '(a "b") equal? test
'(a b c) dup eq? test
'(a b c) dup lcopy eq? not test
'(a b c d) 'b memq car 'c eq? test
'(a b c d) 'z memq #f eq? test
'a '(1 2 3) def
a reverse! '(3 2 1) equal? test

;; Stack effect ;;

stack-deep 0 = test
1 stack-deep 1 = test drop
1 2 swap 1 = test 2 = test
3 dup 3 = test 3 = test
4 5 drop 4 = test
6 7 nip 7 = test stack-deep 0 = test

;; String ;;

"aa" atom? test
"aa" string? test
8 string? not test
#\a string? not test
'() string? not test
'(a) string? not test
"aa" "aa" equal? test
"hoge" "bar" equal? not test
"aa" "bb" string-append "aabb" equal? test
"a" "b" string-append "ab" equal? test

;; Symbol ;;

'abc atom? test
'abc symbol->string "abc" equal? test
"abc" string->symbol 'abc eq? test

;; Block ;;

'aa (9 +) def
1 aa 10 = test
'aa(10 +) def
1 aa 11 = test
(3 +) 5 swap i 8 = test

;; Control ;;

t ( if 8 then ) i 8 = test
#f ( 5 swap if 8 then ) i 5 = test
t ( if 8 else 10 then ) i 8 = test
#f ( if 8 else 10 then ) i 10 = test
( #t if 6 else #t if 7 else 8 then then ) i 6 = test
( #f if 6 else #t if 7 else 8 then then ) i 7 = test
( #f if 6 else #f if 7 else 8 then then ) i 8 = test

;; Vector ;;

5 make-vector dup vector? test
vector-length 5 = test
'vec 3 make-vector def
8 0 vec vector-set!
0 vec vector-ref 8 = test
9 1 vec vector-set!
1 vec vector-ref 9 = test

;; Dict ;;

5 make-dict dict? test
5 make-dict atom? test
5 make-dict pair? not test
'dict 20 make-dict def
8 'a dict dict-put!
'a dict dict-get 8 = test
"a" dict dict-get 8 = test
'b dict dict-get undefined? test

;; Lexical ;;

1 2 ( a b | b a ) i 1 = test 2 = test
1 2 ( a | ( b | b a - ) i ) i -1 = test
2 1 ( a b | ( a b - ) i ) i 1 = test
2 ( a | 8 => a a ) i 8 = test

;; Value ;;

'a 8 val def
a 8 = test
'aa ( a ) def
aa 8 = test
aa a = test
( 5 => a ) i
a 5 = test
aa 5 = test

;; Closure ;;

'c ( n | ( n 1 + dup => n ) ) def
'c8 8 c def
c8 9 = test
c8 10 = test
c8 11 = test

;; Lib ;;

"test/lib.sl" load

0 bye
