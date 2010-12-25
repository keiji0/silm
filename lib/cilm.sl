;; vocab: cilm

;;   record: Struct ( name fields )
;;   record: Fun ( type name in out body )
;;   record: Macro ( macro-name macro-block )

;;   'struct* ( name fields |
;;     Struct make
;;       name over name!
;;       fields over fields! ) def

;;   'fun* ( type name in out body |
;;     Fun make
;;       type over type!
;;       name over name!
;;       in over in!
;;       out over out!
;;       body over body! ) def

;;   'macro* ( name block |
;;     Macro make
;;       name over macro-name!
;;       block over macro-block! ) def

;;   ;; Package ;;

;;   record: Package (
;;     package-name
;;     package-fun
;;     package-struct
;;     package-macro
;;   )

;;   'package-dict 20 make-dict def
;;   'find-package ( [ symbol > package ] package-dict dict-get ) def
;;   'make-package ( name | [ > package ]
;;      name find-package dup undefined? if
;;        drop Package make dup name swap package-name!
;;        dup 20 make-dict swap package-fun!
;;        dup 20 make-dict swap package-struct!
;;        dup 20 make-dict swap package-macro!
;;        dup name package-dict dict-put!
;;      then ) def

;;   'cilm-order '() val def
;;   '>cilm-order ( cilm-order cons => cilm-order ) def
;;   'cilm-global-packages '() val def
;;   '>cilm-global-packages ( cilm-global-packages cons => cilm-global-packages ) def
;;   'cilm-also ( cilm-global-packages => cilm-order ) def
;;   'cilm-define-dict 'main make-package dup >cilm-global-packages val def
;;   'cilm-define-flag 'private val def
;;   'cilm-define* ( selector | [ symbol obj > ]
;;     swap cilm-define-dict selector i dict-put! ) def

;;   'cilm-lookup* ( symbol | [ > obj ]
;;     dup null? if
;;       drop #f
;;     else
;;       dup car symbol swap dict-get dup
;;         undefined? if drop cdr lp then
;;         nip
;;     then ) def
;;   'cilm-lookup ( [ symbol > obj ] cilm-define-dict cilm-order cons swap cilm-lookup* ) def

;;   'private: ( 'private => cilm-define-flag ) def
;;   'public: ( 'public => cilm-define-flag ) def

;;   'package: (
;;     cilm-define-dict >cilm-order
;;     read make-package => cilm-define-dict ) def

;;   'split-in&out ( [ '( int > out ) ]
;;     ( '> eq? ) split-list
;;     dup car
;;     swap cdr dup null? if else car then ) def

;;   'fun: ( [ "name" "(int ... [ > out ...])" "(body ...)" ]
;;     cilm-define-flag
;;     read dup [ name ]
;;     read split-in&out [ in & out ]
;;     read [ body ]
;;     fun* ( package-fun@ ) cilm-define* ) def

;;   'struct: ( [ "name" "((type name) ...)" ]
;;     read dup
;;     read
;;     struct* ( package-struct@ ) cilm-define* ) def

;;   'macro: ( [ "name" "block" ]
;;     read dup read eval
;;     macro* ( package-macro@ ) cilm-define* ) def

;; [fun: hoge (a b c) ((printf "sssss\n"))
;; struct: Point ((int x))
;; macro: hoge (8 8 +)]

;;    ;; Primitive function

;;    'cond* ( body |
;;      dup null? if
;;        reverse!
;;      else
;;        car dup car 
;;      then ) def

';; (((symbol?) 8)) cond*

;; .stack 0 bye

;;    'cond ( [ ]
;;      car (  ) map
;;    ) macro def

;; (cond (a b c))
;; .stack 0 bye

;;    'iscfun? ( obj | obj pair? if obj car symbol? else #f then ) def

;;    'display-lit (
;;    ) def

;;    'display-list-comma (
;;    ) def

;;    'display-fun (
;;      dup car display
;;      "(" display
;;      cdr (display "," display) each
;;      ")" display
;;      ) def

;;    'body>c ( [ body > cbody ]
;;      dup null? if
;;        drop
;;      else
;;        dup car iscfun?
;;        if car display-fun then
;;      then
;;      ) def

';; ;.stack
;; 0 bye

'(1 2 3) (print) each

0 bye
