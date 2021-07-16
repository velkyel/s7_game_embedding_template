(define-macro (dotimes spec . body)	;; spec = (var end . return)
  (let ((e (gensym))
	    (n (car spec)))
	`(do ((,e ,(cadr spec))
		  (,n 0 (+ ,n 1)))
	     ((>= ,n ,e) ,@(cddr spec))
	   ,@body)))


(define (frame-entry)
  ;; (set-color 0 0 0 1)
  ;; (draw-circle (make-vec2 8 4.5) 7)
  #f)

