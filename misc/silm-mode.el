(require 'cl)

(define-minor-mode silm-mode
  "silm edit mode" nil " silm-mode"
  '(
    ("\C-c\C-c" . compile)
    ))

(add-hook 'silm-mode-on-hook 'silm-initialize-mode)

(defun silm-initialize-mode ()
  "silmのセットアップを行う"
  (when (not silm-mode) (error "Not silm-mode"))
  ;; (silm-font-lock-init)
  )


;; Command
;;(define-key silm-mode-map "'" 'silm-mode-quote-toggle)
(define-key silm-mode-map "\C-ct" 'silm-mode-toggle-invisibility)

;; (defun silm-mode-quote-toggle ()
;;   (interactive)
;;   (save-excursion
;;     (unless (looking-at "\\<")
;;       (backward-sexp))
;;     (if (eq ?' (char-after))
;;         (delete-char 1)
;;       (insert-char ?' 1))))

(defun silm-mode-toggle-invisibility ()
  (interactive)
  (setq buffer-invisibility-spec (not buffer-invisibility-spec)))


;; Syntax

(defvar silm-mode-syntax-table nil "Syntax table used while in silm mode.")

(defun silm-syntax-load ()
  (setq silm-mode-syntax-table (make-syntax-table))
  (let ((char 0))
    (while (< char ?!)
      (modify-syntax-entry char " " silm-mode-syntax-table)
      (setq char (1+ char)))
    (while (< char 256)
      (modify-syntax-entry char "w" silm-mode-syntax-table)
      (setq char (1+ char)))
    (modify-syntax-entry ?_ "-" silm-mode-syntax-table)
    ;;(modify-syntax-entry ?' "''" silm-mode-syntax-table)
    (modify-syntax-entry ?# "'#" silm-mode-syntax-table)
    (modify-syntax-entry ?\( "()" silm-mode-syntax-table)
    (modify-syntax-entry ?\) ")(" silm-mode-syntax-table)
    (modify-syntax-entry ?\[ "<]" silm-mode-syntax-table)
    (modify-syntax-entry ?\] ">[" silm-mode-syntax-table)
    (set-syntax-table silm-mode-syntax-table)
  ))

(define-abbrev-table 'silm-mode-abbrev-table ())
(defvar silm-ws-c "\t \n")
(defvar silm-ws (format "[^%s]" silm-ws-c))
(defvar silm-token (format "[^%s]" silm-ws-c))
(defvar silm-indent-level 2 "Amount by which silm subexpressions are indented.")
(defvar silm-section-regex (format "^'%s+ vocab:$" silm-token))

(defun %silm-gen-match-reg (c fase)
  `((,(format "\\(%s\\)" c) 1 ,fase)
    ;(,(format "^\\(%s\\)[\n \t]" c) 1 ,fase)
    ;(,(format "[\n \t]\\(%s\\)$" c) 1 ,fase)
    ))


;; Face 
(defgroup silm-mode-faces nil "emwikiのフェイスのグループ")

(defface silm-mode-section-face '((((class color))
                                   (:foreground "#729FCF" :underline t :height 1.5)))
  "Silm section face"
  :group 'silm-mode-faces)
(defvar silm-mode-section-face 'silm-mode-section-face)

(defface silm-mode-block-face '((((class color))
                                 (:foreground "#666666")))
  "Silm block face"
  :group 'silm-mode-faces)
(defvar silm-mode-block-face 'silm-mode-block-face)


;; Syntax

(defun silm-mode ()
  (interactive)
  (kill-all-local-variables)
  (setq local-abbrev-table silm-mode-abbrev-table)
  (silm-syntax-load)
  (when (null font-lock-defaults)
    (set (make-local-variable 'font-lock-defaults) '(nil)))
  (make-local-variable 'silm-font-lock-keywords)
  (setq silm-font-lock-keywords
        `(
          ;; block: ([)
          ;; ,@(%silm-gen-match-reg "\\(" silm-mode-block-face)
          ;; ,@(%silm-gen-match-reg "\\)" silm-mode-block-face)
          ;; symbol: (...:)
          ;;(,(format "^'\\(%s*\\)" silm-token) 1 font-lock-function-name-face t)
          (,(format "'\\(%s*\\)" silm-token) 1 font-lock-string-face t)
          (,(format "\\<\\('\\)%s" silm-token) 1 '(face font-lock-comment-face invisible symbol-quote))
          ;; syntax target: ...: (...)
          (,(format "\\<\\(%s+:\\)\\>" silm-token silm-token) 1 font-lock-keyword-face)
          ;;(,(format "\\<%s+:\\> +\\<\\(%s+\\)\\>" silm-token silm-token) 1 font-lock-warning-face t)
          ;; Builtin
          (,(format "\\<%%%s+\\>" silm-token) 0 font-lock-builtin-face)
          ;; string: ("...")
          (,"\\<\"[^\"]+\"\\>" . font-lock-string-face)
          (,(format "\\<\'%s+\'\\>" silm-token) 0 '(face font-lock-string-face invisible nil) t)
          ;; keyword: (def|immediate|...)
          (,(format "\\<%s\\>"
                    (regexp-opt '("def" "=>" "|"
                                  "if" "else" "then"
                                  "export" "private") t))
             0 font-lock-keyword-face)
          ;; section: ("...")
          (,silm-section-regex 0 silm-mode-section-face t)
          ))
  (setq font-lock-defaults
        (cons (cons 'silm-font-lock-keywords (car font-lock-defaults))
              (cdr font-lock-defaults)))
  (font-lock-mode 1)
  (font-lock-set-defaults)
  (font-lock-fontify-buffer)
  (use-local-map silm-mode-map)
  (setq mode-name "silm")
  (setq major-mode 'silm-mode)
  (run-hooks 'silm-mode-hook)
  )

(provide 'silm-mode)
